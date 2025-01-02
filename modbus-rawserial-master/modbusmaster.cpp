// modbusmaster.cpp
#include "modbusmaster.h"
#include <QDebug>

ModbusMaster::ModbusMaster(const QString &portName, QObject *parent)
    : QObject(parent)
    , serialPort(new QSerialPort(this))
{
    serialPort->setPortName(portName);
    serialPort->setBaudRate(QSerialPort::Baud9600);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort->open(QIODevice::ReadWrite)) {
        emit errorOccurred(QString("No se pudo abrir el puerto: %1").arg(serialPort->errorString()));
        return;
    }

    connect(serialPort, &QSerialPort::readyRead, this, &ModbusMaster::handleResponse);
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        if (!requestQueue.isEmpty()) {
            auto &currentRequest = requestQueue.head();
            if (currentRequest.retriesRemaining > 0) {
                qDebug() << "[Maestro] Timeout, reintentando solicitud. Retries restantes:"
                         << currentRequest.retriesRemaining;
                --currentRequest.retriesRemaining;
                startProcessing(currentRequest);
            } else {
                currentRequest.callbackError("timeout");
                //emit errorOccurred("Timeout agotado. Descarta solicitud.");
                requestQueue.dequeue();
                isProcessing = false;
                processNextRequest();
            }
        }
    });
}

ModbusMaster::~ModbusMaster()
{
    serialPort->close();
}

void ModbusMaster::readRegister(int slaveId,
                                quint16 address,
                                quint16 quantity,
                                std::function<void(const QVector<quint16> &values)> onSuccess,
                                std::function<void(const QString &error)> onError)
{
    QByteArray request;
    request.append(slaveId);
    request.append(0x03);
    request.append(char(address >> 8));
    request.append(char(address & 0xFF));
    request.append(char(quantity >> 8));
    request.append(char(quantity & 0xFF));
    appendCrc(request);

    requestQueue.enqueue({request,
                          [onSuccess, onError, this](const QByteArray &response) {
                              if (response.size() < 5) {
                                  onError("Respuesta incompleta del esclavo.");
                                  return;
                              }
                              QVector<quint16> values = parseRegisters(response);
                              onSuccess(values);
                          },
                          onError,
                          maxRetries});
    processNextRequest();
}

void ModbusMaster::writeRegister(int slaveId,
                                 quint16 address,
                                 quint16 value,
                                 std::function<void()> onSuccess,
                                 std::function<void(const QString &error)> onError)
{
    QByteArray request;
    request.append(slaveId);
    request.append(0x06);
    request.append(char(address >> 8));
    request.append(char(address & 0xFF));
    request.append(char(value >> 8));
    request.append(char(value & 0xFF));
    appendCrc(request);

    requestQueue.enqueue({request,
                          [onSuccess, onError](const QByteArray & /*response*/) { onSuccess(); },
                          onError,
                          maxRetries});
    processNextRequest();
}

#include <QByteArray>
#include <QDebug>
#include <QString>

QString prettyHexDump(const QByteArray &data, int bytesPerLine = 16)
{
    QString output;
    for (int i = 0; i < data.size(); i += bytesPerLine) {
        // Address
        output += QString("%1  ").arg(i, 8, 16, QChar('0')).toUpper();

        // Hexadecimal values
        QString hexPart;
        QString asciiPart;
        for (int j = 0; j < bytesPerLine; ++j) {
            if (i + j < data.size()) {
                quint8 byte = static_cast<quint8>(data[i + j]);
                hexPart += QString("%1 ").arg(byte, 2, 16, QChar('0')).toUpper();
                asciiPart += (byte >= 32 && byte <= 126) ? QChar(byte) : '.'; // Printable ASCII
            } else {
                hexPart += "   "; // Padding for alignment
                asciiPart += ' ';
            }
        }

        // Combine hex and ASCII parts
        output += hexPart + " " + asciiPart + "\n";
    }
    return output;
}

void ModbusMaster::readCoil(int slaveId,
                            quint16 address,
                            int quantity,
                            std::function<void(QVector<bool> coilValue)> onSuccess,
                            std::function<void(const QString &error)> onError)
{
    qDebug() << "readCoil: " << quantity;
    QByteArray request;
    request.append(slaveId);
    request.append(0x01);
    request.append(char(address >> 8));
    request.append(char(address & 0xFF));
    request.append(char(quantity >> 8));   // High byte of quantity
    request.append(char(quantity & 0xFF)); // Low byte of quantity

    appendCrc(request);

    requestQueue.enqueue({request,
                          [onSuccess, onError, quantity](const QByteArray &response) {
                              if (response.size() < 5) {
                                  onError("Respuesta incompleta del esclavo.");
                                  return;
                              }
                              //qDebug() << prettyHexDump(response);
                              QVector<bool> coilValues;
                              for (int i = 0; i < quint8(response.at(2)); ++i) {
                                  quint8 byte = quint8(response.at(3 + i));
                                  for (int bit = 0; bit < 8; ++bit) {
                                      if ((i * 8 + bit) >= quantity) {
                                          break;
                                      }
                                      coilValues.append(byte & (1 << bit));
                                  }
                              }
                              onSuccess(coilValues);
                          },
                          onError,
                          maxRetries});
    processNextRequest();
}

void ModbusMaster::writeCoil(int slaveId,
                             quint16 address,
                             bool value,
                             std::function<void()> onSuccess,
                             std::function<void(const QString &error)> onError)
{
    QByteArray request;
    request.append(slaveId);
    request.append(0x05);
    request.append(char(address >> 8));
    request.append(char(address & 0xFF));
    request.append(value ? char(0xFF) : char(0x00));
    request.append(char(0x00));
    appendCrc(request);

    requestQueue.enqueue({request,
                          [onSuccess, onError](const QByteArray & /*response*/) { onSuccess(); },
                          onError,
                          maxRetries});
    processNextRequest();
}

void ModbusMaster::handleResponse()
{
    QByteArray response = serialPort->readAll();
    timeoutTimer.stop();
    isProcessing = false;

    if (!requestQueue.isEmpty()) {
        auto currentRequest = requestQueue.dequeue();

        if (!verifyCrc(response)) {
            currentRequest.callbackError("CRC inválido o respuesta incompleta.");
        } else {
            currentRequest.callbackSuccess(response);
        }
    }

    processNextRequest();
}

void ModbusMaster::processNextRequest()
{
    if (isProcessing || requestQueue.isEmpty()) {
        return;
    }

    auto currentRequest = requestQueue.head();
    startProcessing(currentRequest);
}

void ModbusMaster::startProcessing(const ModbusRequest &request)
{
    isProcessing = true;
    auto data = request.data;
#if 0 // create crc errors
    static int n = 0;
    if (++n % 7 == 0) {
        //data.replace(data.size() - 2, 2, QByteArray(2, 0));
        //data.chop(2);
        data.remove(0, 1); // Elimina el primer byte
    }
#endif

    serialPort->write(data);
    timeoutTimer.start(500);
}

void ModbusMaster::appendCrc(QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (char byte : data) {
        crc ^= quint8(byte);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    data.append(crc & 0xFF);
    data.append((crc >> 8) & 0xFF);
}

bool ModbusMaster::verifyCrc(const QByteArray &data)
{
    if (data.size() < 2) {
        return false;
    }

#if 0 // create receive crc errors
    static int n = 0;
    if (++n % 7 == 0) {
        return false;
    }
#endif

    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size() - 2; ++i) {
        crc ^= quint8(data.at(i));
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    quint16 receivedCrc = quint16(quint8(data.at(data.size() - 1)) << 8)
                          | quint8(data.at(data.size() - 2));
    return crc == receivedCrc;
}

QVector<quint16> ModbusMaster::parseRegisters(const QByteArray &response)
{
    QVector<quint16> values;
    int byteCount = response.at(2);
    for (int i = 0; i < byteCount; i += sizeof(quint16)) {
        quint16 value = (quint8(response.at(3 + i)) << 8) | quint8(response.at(3 + i + 1));
        values.append(value);
    }
    return values;
}
