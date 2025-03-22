#include "modbustcp-master.h"

ModbusTcpClient::ModbusTcpClient(QObject *parent)
    : QObject(parent)
    , transactionId(1)
{
    socket = new QTcpSocket(this);
}

ModbusTcpClient::~ModbusTcpClient()
{
    disconnectFromHost();
}

bool ModbusTcpClient::connectToHost(const QString &host, quint16 port, int timeout)
{
    socket->connectToHost(host, port);
    if (!socket->waitForConnected(timeout)) {
        qCritical() << "Connection error:" << socket->errorString();
        return false;
    }
    return true;
}

void ModbusTcpClient::disconnectFromHost()
{
    if (socket->isOpen()) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(1000);
    }
}

static QVector<bool> unpackModbusBits(QDataStream &stream, quint16 bitCount)
{
    QVector<bool> bits;
    bits.reserve(bitCount);

    quint8 byte;
    for (quint16 i = 0; i < bitCount; ++i) {
        if (i % 8 == 0) {
            stream >> byte;
        }
        bits.append(byte & (1 << (i % 8)));
    }
    return bits;
}

bool ModbusTcpClient::readDiscreteInputs(quint16 address, quint16 count, QVector<bool> &ret)
{
    const quint8 FUNCTION_CODE = 0x02;

    QByteArray pdu;
    QDataStream pduStream(&pdu, QIODevice::WriteOnly);
    pduStream.setByteOrder(QDataStream::BigEndian);
    pduStream << FUNCTION_CODE << address << count;

    QByteArray responsePDU;
    if (!sendRequest(pdu, responsePDU))
        return false;

    QDataStream respStream(responsePDU);
    respStream.setByteOrder(QDataStream::BigEndian);
    quint8 functionCode, byteCount;
    respStream >> functionCode >> byteCount;
    if (functionCode != FUNCTION_CODE || byteCount != (count + 7) / 8) {
        qCritical() << "Unexpected response for readCoil: Got: f==" << functionCode
                    << " count==" << byteCount;
        return false;
    }

    ret = unpackModbusBits(respStream, count);
    return true;
}

bool ModbusTcpClient::readCoil(quint16 address, bool &value)
{
    const quint8 FUNCTION_CODE = 0x01;

    QByteArray pdu;
    QDataStream pduStream(&pdu, QIODevice::WriteOnly);
    pduStream.setByteOrder(QDataStream::BigEndian);
    pduStream << FUNCTION_CODE << address << quint16(1);

    QByteArray responsePDU;
    if (!sendRequest(pdu, responsePDU))
        return false;

    QDataStream respStream(responsePDU);
    respStream.setByteOrder(QDataStream::BigEndian);
    quint8 functionCode, byteCount;
    respStream >> functionCode >> byteCount;
    if (functionCode != FUNCTION_CODE || byteCount < 1) {
        qCritical() << "Unexpected response for readCoil.";
        return false;
    }

    quint8 coilStatus;
    respStream >> coilStatus;
    value = (coilStatus & 0x01) != 0;

    return true;
}

bool ModbusTcpClient::writeCoil(quint16 address, bool value)
{
    const quint8 FUNCTION_CODE = 0x05;

    QByteArray pdu;
    QDataStream pduStream(&pdu, QIODevice::WriteOnly);
    pduStream.setByteOrder(QDataStream::BigEndian);
    pduStream << FUNCTION_CODE << address << (value ? quint16(0xFF00) : quint16(0x0000));

    QByteArray responsePDU;
    if (!sendRequest(pdu, responsePDU))
        return false;

    if (responsePDU != pdu) {
        qCritical() << "Write coil response mismatch.";
        return false;
    }
    return true;
}

bool ModbusTcpClient::readRegisters(quint16 startAddress, quint16 count, QVector<quint16> &registers)
{
    const quint8 FUNCTION_CODE = 0x04;

    QByteArray pdu;
    QDataStream pduStream(&pdu, QIODevice::WriteOnly);
    pduStream.setByteOrder(QDataStream::BigEndian);
    pduStream << FUNCTION_CODE << startAddress << count;

    QByteArray responsePDU;
    if (!sendRequest(pdu, responsePDU))
        return false;

    QDataStream respStream(responsePDU);
    respStream.setByteOrder(QDataStream::BigEndian);
    quint8 functionCode, byteCount;
    respStream >> functionCode >> byteCount;
    if (functionCode != FUNCTION_CODE || byteCount != count * 2) {
        qCritical() << "Unexpected response for readRegisters.";
        return false;
    }

    registers.clear();
    for (int i = 0; i < count; ++i) {
        quint16 reg;
        respStream >> reg;
        registers.append(reg);
    }
    return true;
}


bool ModbusTcpClient::sendRequest(const QByteArray &pdu, QByteArray &responsePDU, int timeout)
{
    QByteArray request;
    QDataStream stream(&request, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << transactionId;
    stream << quint16(0x0000);
    quint16 length = 1 + pdu.size();
    stream << length;
    stream << quint8(0x01);
    request.append(pdu);

    qDebug() << "Request data: " << request.toHex();
    socket->write(request);
    if (!socket->waitForBytesWritten(timeout)) {
        qCritical() << "Error writing to socket:" << socket->errorString();
        return false;
    }

    QByteArray header;
    while (header.size() < 7) {
        if (!socket->waitForReadyRead(timeout)) {
            qCritical() << "Timeout waiting for header. timeout==" << timeout;
            return false;
        }
        header.append(socket->read(7 - header.size()));
    }

    QDataStream headerStream(header);
    headerStream.setByteOrder(QDataStream::BigEndian);
    quint16 respTransactionId, protocolId, respLength;
    quint8 unitId;
    headerStream >> respTransactionId >> protocolId >> respLength >> unitId;

    if (respTransactionId != transactionId) {
        qCritical() << "Transaction ID mismatch " << respTransactionId << "!=" << transactionId;
        return false;
    }
    ++transactionId;

    int remaining = respLength - 1;
    QByteArray pduResponse;
    while (pduResponse.size() < remaining) {
        if (socket->bytesAvailable() == 0 && !socket->waitForReadyRead(timeout)) {
            qCritical() << "Timeout waiting for full response. t==" << timeout;
            return false;
        }
        pduResponse.append(socket->read(remaining - pduResponse.size()));
    }
    responsePDU = pduResponse;
    qDebug() << "Response data: " << responsePDU.toHex();
    return true;
}
