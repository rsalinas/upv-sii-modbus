#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QTimer>
#include <QtSerialPort/QSerialPort>

class ModbusMaster : public QObject
{
    Q_OBJECT

public:
    ModbusMaster(const QString &portName, QObject *parent = nullptr)
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
            qFatal("[Maestro] No se pudo abrir el puerto serie: %s",
                   qPrintable(serialPort->errorString()));
        }

        connect(serialPort, &QSerialPort::readyRead, this, &ModbusMaster::handleResponse);
    }

    void readTemperature(int slaveId)
    {
        readRegister(slaveId, 0x00, 1); // Dirección 0x00, 1 registro
    }

    void writeLed(int slaveId, bool on)
    {
        writeRegister(slaveId, 0x01, on ? 1 : 0); // Dirección 0x01, valor 1 (ON) o 0 (OFF)
    }

    void readCoil(int slaveId, quint16 address)
    {
        QByteArray request;
        request.append(slaveId);              // Slave ID
        request.append(0x01);                 // Function code: Read Coils
        request.append(char(address >> 8));   // Start address high byte
        request.append(char(address & 0xFF)); // Start address low byte
        request.append(char(0x00));           // Quantity of coils high byte
        request.append(char(0x01));           // Quantity of coils low byte
        appendCrc(request);
        serialPort->write(request);
        qDebug() << "[Maestro] Solicitud de lectura de coil enviada.";
    }

    void writeCoil(int slaveId, quint16 address, bool value)
    {
        QByteArray request;
        request.append(slaveId);                         // Slave ID
        request.append(0x05);                            // Function code: Write Single Coil
        request.append(char(address >> 8));              // Coil address high byte
        request.append(char(address & 0xFF));            // Coil address low byte
        request.append(value ? char(0xFF) : char(0x00)); // Value high byte
        request.append(char(0x00));                      // Value low byte
        appendCrc(request);
        serialPort->write(request);
        qDebug() << "[Maestro] Solicitud de escritura de coil enviada.";
    }

    void readRegister(int slaveId, quint16 address, quint16 quantity)
    {
        QByteArray request;
        request.append(slaveId);               // Slave ID
        request.append(0x03);                  // Function code: Read Holding Registers
        request.append(char(address >> 8));    // Start address high byte
        request.append(char(address & 0xFF));  // Start address low byte
        request.append(char(quantity >> 8));   // Quantity of registers high byte
        request.append(char(quantity & 0xFF)); // Quantity of registers low byte
        appendCrc(request);
        serialPort->write(request);
        qDebug() << "[Maestro] Solicitud de lectura de registro enviada.";
    }

    void writeRegister(int slaveId, quint16 address, quint16 value)
    {
        QByteArray request;
        request.append(slaveId);              // Slave ID
        request.append(0x06);                 // Function code: Write Single Register
        request.append(char(address >> 8));   // Register address high byte
        request.append(char(address & 0xFF)); // Register address low byte
        request.append(char(value >> 8));     // Value high byte
        request.append(char(value & 0xFF));   // Value low byte
        appendCrc(request);
        serialPort->write(request);
        qDebug() << "[Maestro] Solicitud de escritura de registro enviada.";
    }

private slots:
    void handleResponse()
    {
        QByteArray response = serialPort->readAll();
        if (response.size() < 5) {
            qWarning() << "[Maestro] Respuesta incompleta recibida.";
            return;
        }

        quint8 functionCode = response.at(1);
        if (functionCode & 0x80) {
            quint8 errorCode = response.at(2);
            qWarning() << "[Maestro] Error recibido del esclavo. Código de error:" << errorCode;
            return;
        }

        switch (functionCode) {
        case 0x01: // Read Coils
        case 0x03: // Read Holding Registers
            if (response.size() >= 7) {
                quint16 value = (quint8(response.at(3)) << 8) | quint8(response.at(4));
                qDebug() << "[Maestro] Valor leído:" << value;
            }
            break;
        case 0x05: // Write Single Coil
        case 0x06: // Write Single Register
            qDebug() << "[Maestro] Confirmación de escritura recibida.";
            break;
        default:
            qWarning() << "[Maestro] Código de función no reconocido en la respuesta.";
            break;
        }
    }

private:
    void appendCrc(QByteArray &data)
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

    QSerialPort *serialPort;
};

#endif // MODBUSMASTER_H
