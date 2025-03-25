#include "modbustcp-slave.h"

ModbusTcpSlave::ModbusTcpSlave(QObject *parent)
    : QTcpServer(parent)
{
    // La inicialización de los datos se ha movido al main
}

// Métodos para coils
void ModbusTcpSlave::setCoils(const QVector<bool> &newCoils)
{
    coils = newCoils;
}

QVector<bool> ModbusTcpSlave::getCoils() const
{
    return coils;
}

void ModbusTcpSlave::setCoil(int index, bool value)
{
    if (index >= 0 && index < coils.size()) {
        updateCoil(index, value);
    } else {
        qWarning() << "Índice de coil fuera de rango:" << index;
    }
}

bool ModbusTcpSlave::getCoil(int index) const
{
    if (index >= 0 && index < coils.size()) {
        return coils[index];
    }
    qWarning() << "Índice de coil fuera de rango:" << index;
    return false;
}

// Métodos para discreteInputs
void ModbusTcpSlave::setDiscreteInputs(const QVector<bool> &newDiscreteInputs)
{
    discreteInputs = newDiscreteInputs;
}

QVector<bool> ModbusTcpSlave::getDiscreteInputs() const
{
    return discreteInputs;
}

void ModbusTcpSlave::setDiscreteInput(int index, bool value)
{
    if (index >= 0 && index < discreteInputs.size()) {
        updateDiscreteInput(index, value);
    } else {
        qWarning() << "Índice de entrada discreta fuera de rango:" << index;
    }
}

bool ModbusTcpSlave::getDiscreteInput(int index) const
{
    if (index >= 0 && index < discreteInputs.size()) {
        return discreteInputs[index];
    }
    qWarning() << "Índice de entrada discreta fuera de rango:" << index;
    return false;
}

// Métodos para inputRegisters
void ModbusTcpSlave::setInputRegisters(const QVector<quint16> &newInputRegisters)
{
    inputRegisters = newInputRegisters;
}

QVector<quint16> ModbusTcpSlave::getInputRegisters() const
{
    return inputRegisters;
}

void ModbusTcpSlave::setInputRegister(int index, quint16 value)
{
    if (index >= 0 && index < inputRegisters.size()) {
        updateInputRegister(index, value);
    } else {
        qWarning() << "Índice de registro de entrada fuera de rango:" << index;
    }
}

quint16 ModbusTcpSlave::getInputRegister(int index) const
{
    if (index >= 0 && index < inputRegisters.size()) {
        return inputRegisters[index];
    }
    qWarning() << "Índice de registro de entrada fuera de rango:" << index;
    return 0;
}

// Métodos para actualizar valores y emitir señales
void ModbusTcpSlave::updateCoil(int index, bool value)
{
    if (coils[index] != value) {
        coils[index] = value;
        emit coilChanged(index, value);
    }
}

void ModbusTcpSlave::updateDiscreteInput(int index, bool value)
{
    if (discreteInputs[index] != value) {
        discreteInputs[index] = value;
        emit discreteInputChanged(index, value);
    }
}

void ModbusTcpSlave::updateInputRegister(int index, quint16 value)
{
    if (inputRegisters[index] != value) {
        inputRegisters[index] = value;
        emit inputRegisterChanged(index, value);
    }
}

bool ModbusTcpSlave::listen(const QHostAddress &address, quint16 port)
{
    qDebug() << "Intentando iniciar servidor Modbus TCP en" << address << "puerto" << port;
    if (!QTcpServer::listen(address, port)) {
        qCritical() << "No se pudo iniciar el servidor Modbus TCP.";
        return false;
    }
    qDebug() << "Servidor Modbus TCP iniciado correctamente.";
    return true;
}

void ModbusTcpSlave::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead, this, &ModbusTcpSlave::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ModbusTcpSlave::onDisconnected);

    qDebug() << "Nueva conexión establecida desde" << socket->peerAddress() << "puerto"
             << socket->peerPort();
}

void ModbusTcpSlave::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        qWarning() << "Error: No se pudo obtener el socket que emitió la señal.";
        return;
    }

    QByteArray request = socket->readAll();
    qDebug() << "Solicitud recibida desde" << socket->peerAddress() << "puerto"
             << socket->peerPort();
    qDebug() << "Datos de la solicitud (hex):" << request.toHex();

    processRequest(socket, request);
}

void ModbusTcpSlave::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket) {
        qDebug() << "Conexión cerrada desde" << socket->peerAddress() << "puerto"
                 << socket->peerPort();
        socket->deleteLater();
    }
}

void ModbusTcpSlave::processRequest(QTcpSocket *socket, const QByteArray &request)
{
    qDebug() << "Procesando solicitud...";
    QByteArray response = createResponse(request);
    qDebug() << "Respuesta generada (hex):" << response.toHex();

    auto nWritten = socket->write(response);
    if (nWritten == -1) {
        qCritical() << "Error al escribir la respuesta en el socket:" << socket->errorString();
    } else {
        qDebug() << "Respuesta enviada correctamente." << nWritten;
    }
    socket->flush();
}

void ModbusTcpSlave::processReadCoilsRequest(QDataStream &stream, QDataStream &responseStream)
{
    quint16 startAddress, coilCount;
    stream >> startAddress >> coilCount;

    qDebug() << "Read Coils - Start Address:" << startAddress << "Coil Count:" << coilCount;

    quint8 byteCount = (coilCount + 7) / 8;
    responseStream << byteCount; // Añade byteCount a la respuesta

    quint8 data = 0;
    for (int i = 0; i < coilCount; ++i) {
        if (coils[startAddress + i]) {
            data |= (1 << (i % 8));
        }
        if ((i + 1) % 8 == 0 || i == coilCount - 1) {
            responseStream << data;
            data = 0;
        }
    }
}

void ModbusTcpSlave::processReadDiscreteInputsRequest(QDataStream &stream,
                                                      QDataStream &responseStream)
{
    quint16 startAddress, inputCount;
    stream >> startAddress >> inputCount;

    qDebug() << "Read Discrete Inputs - Start Address:" << startAddress
             << "Input Count:" << inputCount;

    quint8 byteCount = (inputCount + 7) / 8;
    responseStream << byteCount; // Añade byteCount a la respuesta

    quint8 data = 0;
    for (int i = 0; i < inputCount; ++i) {
        if (discreteInputs[startAddress + i]) {
            data |= (1 << (i % 8));
        }
        if ((i + 1) % 8 == 0 || i == inputCount - 1) {
            qDebug() << "Writing data to stream in 0x2: " << data;
            responseStream << data;
            data = 0;
        }
    }
}



QByteArray ModbusTcpSlave::createResponse(const QByteArray &request)
{
    QDataStream stream(request);
    stream.setByteOrder(QDataStream::BigEndian);

    quint16 transactionId, protocolId, length;
    quint8 unitId, functionCode;

    stream >> transactionId >> protocolId >> length >> unitId >> functionCode;

    qDebug() << "Solicitud recibida:";
    qDebug() << "Transaction ID:" << transactionId;
    qDebug() << "Protocol ID:" << protocolId;
    qDebug() << "Length:" << length;
    qDebug() << "Unit ID:" << unitId;
    qDebug() << "Function Code:" << functionCode;

    QByteArray response;
    QDataStream responseStream(&response, QIODevice::WriteOnly);
    responseStream.setByteOrder(QDataStream::BigEndian);

    responseStream << transactionId << protocolId << quint16(0); // Length will be updated later
    responseStream << unitId << functionCode;

    switch (functionCode) {
    case 0x01: // Read Coils
        processReadCoilsRequest(stream, responseStream);
        break;

    case 0x02: // Read Discrete Inputs
        processReadDiscreteInputsRequest(stream, responseStream);
        break;
#warning TODO: Implementar estas funciones y reactivar el código
        // case 0x04: // Read Input Registers
        //     processReadInputRegistersRequest(stream, responseStream);
        //     break;
        // case 0x0F: // Write Multiple Coils
        //     processWriteMultipleCoilsRequest(stream, responseStream);
        //     break;

    default:
        qWarning() << "Función no soportada. Código de función:" << functionCode;
        responseStream << quint8(functionCode | 0x80)
                       << quint8(0x01); // Exception code 01 (Illegal Function)
        break;
    }

    // Actualizar la longitud de la respuesta
    quint16 responseLength = response.size() - 6; // Almacena el valor en una variable
    response.replace(1 + 4 /* +1 porque incluye el device address */,
                     2,
                     QByteArray::fromRawData(reinterpret_cast<const char *>(&responseLength), 2));

    qDebug() << "Respuesta final (hex):" << response.toHex();
    return response;
}
