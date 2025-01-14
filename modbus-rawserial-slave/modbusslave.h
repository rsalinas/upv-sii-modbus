#ifndef MODBUSSLAVE_H
#define MODBUSSLAVE_H

#include <QDebug>
#include <QObject>
#include <QRandomGenerator>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class ModbusSlave : public QObject
{
    Q_OBJECT

public:
    ModbusSlave();

    ModbusSlave(const QString &portName, QObject *parent = nullptr)
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
            qCritical() << "Failed to open serial port:" << serialPort->errorString();
            return;
        }

        connect(serialPort, &QSerialPort::readyRead, this, &ModbusSlave::handleRequest);
        qDebug() << "Modbus slave listening on" << portName;
    }

private slots:
    void handleRequest()
    {
        QByteArray request = serialPort->readAll();

        if (request.isEmpty()) {
            return;
        }

        quint8 functionCode = request.at(1); // Modbus function code is at byte 1
        QByteArray response;

        switch (functionCode) {
        case 0x03: // Read Holding Registers
            response = handleReadRegister(request);
            break;
        case 0x06: // Write Single Register
            response = handleWriteRegister(request);
            break;
        case 0x01: // Read Coils
            response = handleReadCoil(request);
            break;
        case 0x05: // Write Single Coil
            response = handleWriteCoil(request);
            break;
        default:
            response = createExceptionResponse(functionCode, 0x01); // Illegal function
            break;
        }

        serialPort->write(response);
    }

    QByteArray handleReadRegister(const QByteArray &request)
    {
        if (request.size() < 6) {
            return createExceptionResponse(0x03, 0x02); // Invalid data address
        }

        quint16 startAddress = (quint8(request.at(2)) << 8) | quint8(request.at(3));
        quint16 quantity = (quint8(request.at(4)) << 8) | quint8(request.at(5));

        QByteArray response;
        response.append(request.at(0)); // Slave ID
        response.append(0x03);          // Function code
        response.append(quantity * 2);  // Byte count

        for (int i = 0; i < quantity; ++i) {
            quint16 value = QRandomGenerator::global()->bounded(0xFFFF);
            response.append(value >> 8);
            response.append(value & 0xFF);
        }

        return response;
    }

    QByteArray handleWriteRegister(const QByteArray &request)
    {
        if (request.size() < 6) {
            return createExceptionResponse(0x06, 0x02); // Invalid data address
        }

        quint16 address = (quint8(request.at(2)) << 8) | quint8(request.at(3));
        quint16 value = (quint8(request.at(4)) << 8) | quint8(request.at(5));

        qDebug() << "Write Register - Address:" << address << "Value:" << value;

        return request; // Echo back the request as the response
    }

    QByteArray handleReadCoil(const QByteArray &request)
    {
        if (request.size() < 6) {
            return createExceptionResponse(0x01, 0x02); // Invalid data address
        }

        quint16 quantity = (quint8(request.at(4)) << 8) | quint8(request.at(5));

        QByteArray response;
        response.append(request.at(0));      // Slave ID
        response.append(0x01);               // Function code
        response.append((quantity + 7) / 8); // Byte count

        for (int i = 0; i < quantity; ++i) {
            quint8 bit = QRandomGenerator::global()->bounded(2); // Random 0 or 1
            if (i % 8 == 0) {
                response.append((char) 0x00); // Initialize byte
            }
            response[response.size() - 1] |= (bit << (i % 8));
        }

        return response;
    }

    QByteArray handleWriteCoil(const QByteArray &request)
    {
        if (request.size() < 6) {
            return createExceptionResponse(0x05, 0x02); // Invalid data address
        }

        quint16 address = (quint8(request.at(2)) << 8) | quint8(request.at(3));
        quint16 value = (quint8(request.at(4)) << 8) | quint8(request.at(5));

        qDebug() << "Write Coil - Address:" << address << "Value:" << (value ? "ON" : "OFF");

        return request; // Echo back the request as the response
    }

    QByteArray createExceptionResponse(quint8 functionCode, quint8 exceptionCode)
    {
        QByteArray response;
        response.append(0x01);                // Slave ID
        response.append(functionCode | 0x80); // Exception function code
        response.append(exceptionCode);       // Exception code
        return response;
    }

private:
    QSerialPort *serialPort;
};

#endif // MODBUSSLAVE_H
