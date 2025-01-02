#include "modbustcpslave.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QByteArray>
#include <QtEndian>
#include <QDebug>

ModbusTcpSlave::ModbusTcpSlave(QObject *parent)
    : QTcpServer(parent),
      m_coils(100, false),
      m_registers(100, 0)
{
}

void ModbusTcpSlave::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        socket->deleteLater();
        return;
    }
    connect(socket, &QTcpSocket::readyRead, [this, socket]() {
        processData(socket);
    });
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void ModbusTcpSlave::processData(QTcpSocket *socket)
{
    QByteArray buffer = socket->readAll();
    int offset = 0;
    while (buffer.size() - offset >= 7) {
        // Cabecera Modbus TCP: transacción (2), protocolo (2), longitud (2) y id de unidad (1)
        quint16 transactionId = (static_cast<quint8>(buffer[offset]) << 8) | static_cast<quint8>(buffer[offset+1]);
        quint16 protocolId    = (static_cast<quint8>(buffer[offset+2]) << 8) | static_cast<quint8>(buffer[offset+3]);
        quint16 length        = (static_cast<quint8>(buffer[offset+4]) << 8) | static_cast<quint8>(buffer[offset+5]);
        int totalLength = 6 + length;
        if (buffer.size() - offset < totalLength)
            break; // Falta parte del mensaje
        quint8 unitId = static_cast<quint8>(buffer[offset+6]);
        QByteArray pdu = buffer.mid(offset+7, length - 1);
        QByteArray responsePDU = processPDU(pdu, unitId);

        // Construir respuesta con cabecera Modbus TCP
        QByteArray response;
        QDataStream respStream(&response, QIODevice::WriteOnly);
        respStream.setByteOrder(QDataStream::BigEndian);
        respStream << transactionId << protocolId;
        quint16 respLength = 1 + static_cast<quint16>(responsePDU.size());
        respStream << respLength << unitId;
        response.append(responsePDU);
        socket->write(response);
        offset += totalLength;
    }
}

QByteArray ModbusTcpSlave::processPDU(const QByteArray &pdu, quint8 /*unitId*/)
{
    QByteArray response;
    if (pdu.isEmpty())
        return modbusExceptionResponse(0, 0x03); // Valor de datos ilegal

    quint8 functionCode = static_cast<quint8>(pdu[0]);
    QDataStream stream(pdu);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.skipRawData(1); // Saltar el código de función

    switch (functionCode) {
    case 0x01: { // Leer coils
        if (pdu.size() < 5)
            return modbusExceptionResponse(functionCode, 0x03);
        quint16 startAddr, quantity;
        stream >> startAddr >> quantity;
        if (startAddr + quantity > static_cast<quint16>(m_coils.size()) || quantity < 1 || quantity > 2000)
            return modbusExceptionResponse(functionCode, 0x02);
        int byteCount = (quantity + 7) / 8;
        QByteArray coilStatus(byteCount, 0);
        for (int i = 0; i < quantity; ++i) {
            if (m_coils[startAddr + i])
                coilStatus[i / 8] |= (1 << (i % 8));
        }
        QDataStream respStream(&response, QIODevice::WriteOnly);
        respStream.setByteOrder(QDataStream::BigEndian);
        respStream << functionCode << static_cast<quint8>(byteCount);
        response.append(coilStatus);
        break;
    }
    case 0x05: { // Escribir coil único
        if (pdu.size() < 5)
            return modbusExceptionResponse(functionCode, 0x03);
        quint16 addr, value;
        stream >> addr >> value;
        if (addr >= static_cast<quint16>(m_coils.size()))
            return modbusExceptionResponse(functionCode, 0x02);
        m_coils[addr] = (value == 0xFF00);
        // Respuesta: eco de la solicitud
        QDataStream respStream(&response, QIODevice::WriteOnly);
        respStream.setByteOrder(QDataStream::BigEndian);
        respStream << functionCode << addr << value;
        break;
    }
    case 0x0F: { // Escribir múltiples coils
        if (pdu.size() < 6)
            return modbusExceptionResponse(functionCode, 0x03);
        quint16 startAddr, quantity;
        quint8 byteCount;
        stream >> startAddr >> quantity >> byteCount;
        if (pdu.size() < static_cast<int>(6 + byteCount))
            return modbusExceptionResponse(functionCode, 0x03);
        if (startAddr + quantity > static_cast<quint16>(m_coils.size()) || quantity < 1 || quantity > 0x07B0)
            return modbusExceptionResponse(functionCode, 0x02);
        QByteArray coilData = pdu.mid(5, byteCount);
        for (int i = 0; i < quantity; ++i) {
            int byteIndex = i / 8;
            int bitIndex = i % 8;
            bool coilValue = (coilData.at(byteIndex) >> bitIndex) & 0x01;
            m_coils[startAddr + i] = coilValue;
        }
        // Respuesta: función, dirección inicial y cantidad de coils
        QDataStream respStream(&response, QIODevice::WriteOnly);
        respStream.setByteOrder(QDataStream::BigEndian);
        respStream << functionCode << startAddr << quantity;
        break;
    }
    case 0x03: { // Leer registros de retenció
        if (pdu.size() < 5)
            return modbusExceptionResponse(functionCode, 0x03);
        quint16 startAddr, quantity;
        stream >> startAddr >> quantity;
        if (startAddr + quantity > static_cast<quint16>(m_registers.size()) || quantity < 1 || quantity > 125)
            return modbusExceptionResponse(functionCode, 0x02);
        QDataStream respStream(&response, QIODevice::WriteOnly);
        respStream.setByteOrder(QDataStream::BigEndian);
        respStream << functionCode << static_cast<quint8>(quantity * 2);
        for (int i = 0; i < quantity; ++i)
            respStream << m_registers[startAddr + i];
        break;
    }
    case 0x06: { // Escribir registro único
        if (pdu.size() < 5)
            return modbusExceptionResponse(functionCode, 0x03);
        quint16 addr, value;
        stream >> addr >> value;
        if (addr >= static_cast<quint16>(m_registers.size()))
            return modbusExceptionResponse(functionCode, 0x02);
        m_registers[addr] = value;
        // Respuesta: eco de la solicitud
        QDataStream respStream(&response, QIODevice::WriteOnly);
        respStream.setByteOrder(QDataStream::BigEndian);
        respStream << functionCode << addr << value;
        break;
    }
    case 0x10: { // Escribir múltiples registros
        if (pdu.size() < 6)
            return modbusExceptionResponse(functionCode, 0x03);
        quint16 startAddr, quantity;
        quint8 byteCount;
        stream >> startAddr >> quantity >> byteCount;
        if (byteCount != quantity * 2 || pdu.size() < static_cast<int>(6 + byteCount))
            return modbusExceptionResponse(functionCode, 0x03);
        if (startAddr + quantity > static_cast<quint16>(m_registers.size()) || quantity < 1 || quantity > 123)
            return modbusExceptionResponse(functionCode, 0x02);
        for (int i = 0; i < quantity; ++i) {
            quint16 regVal;
            stream >> regVal;
            m_registers[startAddr + i] = regVal;
        }
        // Respuesta: función, dirección inicial y cantidad de registros
        QDataStream respStream(&response, QIODevice::WriteOnly);
        respStream.setByteOrder(QDataStream::BigEndian);
        respStream << functionCode << startAddr << quantity;
        break;
    }
    default:
        return modbusExceptionResponse(functionCode, 0x01); // Función ilegal
    }
    return response;
}

QByteArray ModbusTcpSlave::modbusExceptionResponse(quint8 functionCode, quint8 exceptionCode)
{
    QByteArray response;
    QDataStream stream(&response, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint8>(functionCode | 0x80) << exceptionCode;
    return response;
}
