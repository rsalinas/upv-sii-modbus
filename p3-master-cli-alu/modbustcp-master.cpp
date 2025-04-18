#include "modbustcp-master.h"

// Constructor de la clase ModbusTcpClient.
ModbusTcpClient::ModbusTcpClient(QObject *parent)
    : QObject(parent)
    , transactionId(1) // Se inicializa el ID de transacción en 1.
{
    // Se crea el socket TCP, asignándolo como hijo para gestión automática de memoria.
    socket = new QTcpSocket(this);
}

// Destructor de la clase ModbusTcpClient.
ModbusTcpClient::~ModbusTcpClient()
{
    // Se desconecta el socket del host al destruir la instancia.
    socket->disconnectFromHost();
}

// Método para conectar con un host remoto usando la dirección, puerto y un tiempo de espera.
bool ModbusTcpClient::connectToHost(const QString &host, quint16 port, int timeout)
{
    // Se intenta establecer la conexión con el host especificado.
    socket->connectToHost(host, port);
    // Se espera a que la conexión se establezca o se agote el tiempo.
    if (!socket->waitForConnected(timeout)) {
        qCritical() << "Error de conexión:" << socket->errorString();
        return false;
    }
    return true;
}

// Método que envía una solicitud Modbus (PDU) y espera la respuesta correspondiente.
bool ModbusTcpClient::sendRequest(const QByteArray &pdu, QByteArray &responsePdu, int timeout)
{
    QByteArray request;
    // Se crea un stream para construir el mensaje completo.
    QDataStream stream(&request, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    // Se escribe el identificador de la transacción.
    stream << transactionId;
    // Se escribe el identificador del protocolo (0x0000 para Modbus TCP).
    stream << quint16(0x0000);
    // Se calcula la longitud del mensaje: 1 byte para la unidad de identificación más el tamaño del PDU.
    quint16 length = 1 + pdu.size();
    stream << length;
    // Se escribe el identificador de la unidad (normalmente 0x01).
    stream << quint8(0x01);
    // Se añade el PDU al mensaje.
    request.append(pdu);

    qDebug() << "Datos de solicitud: " << request.toHex();
    // Se envía la solicitud a través del socket.
    socket->write(request);
    // Se espera a que se hayan escrito todos los datos.
    if (!socket->waitForBytesWritten(timeout)) {
        qCritical() << "Error al escribir en el socket:" << socket->errorString();
        return false;
    }

    QByteArray header;
    // Se lee el encabezado de la respuesta (debe tener 7 bytes).
    while (header.size() < 7) {
        if (!socket->waitForReadyRead(timeout)) {
            qCritical() << "Tiempo de espera agotado al esperar el encabezado. timeout=="
                        << timeout;
            return false;
        }
        header.append(socket->read(7 - header.size()));
    }

    // Se procesa el encabezado recibido.
    QDataStream headerStream(header);
    headerStream.setByteOrder(QDataStream::BigEndian);
    quint16 respTransactionId, protocolId, respLength;
    quint8 unitId;
    headerStream >> respTransactionId >> protocolId >> respLength >> unitId;

    // Se verifica que el ID de transacción de la respuesta coincida con el enviado.
    if (respTransactionId != transactionId) {
        qCritical() << "Desajuste en el ID de transacción " << respTransactionId
                    << "!=" << transactionId;
        return false;
    }
    ++transactionId; // Se incrementa el ID de transacción para la próxima solicitud.

    // Se calcula la cantidad de bytes restantes que deben leerse (excluyendo la unidad de identificación).
    int remaining = respLength - 1;
    QByteArray responseBuffer;
    // Se lee el resto de la respuesta.
    while (responseBuffer.size() < remaining) {
        if (socket->bytesAvailable() == 0 && !socket->waitForReadyRead(timeout)) {
            qCritical() << "Tiempo de espera agotado al esperar la respuesta completa. timeout=="
                        << timeout;
            return false;
        }
        responseBuffer.append(socket->read(remaining - responseBuffer.size()));
    }
    // Se almacena la parte de la respuesta que corresponde al PDU.
    responsePdu = responseBuffer;
    qDebug() << "Datos de respuesta: " << responsePdu.toHex();
    return true;
}

// Función auxiliar para desempaquetar los bits recibidos en el formato Modbus.
// Lee 'bitCount' bits desde el QDataStream y los almacena en un QVector<bool>.
static QVector<bool> unpackModbusBits(QDataStream &stream, quint16 bitCount)
{
    QVector<bool> bits;
    bits.reserve(bitCount); // Reserva espacio para los bits.

    quint8 byte;
    // Se recorre cada bit solicitado.
    for (quint16 i = 0; i < bitCount; ++i) {
        // Cada 8 bits se lee un nuevo byte del stream.
        if (i % 8 == 0) {
            stream >> byte;
        }
        // Se añade el valor del bit (comprobando si está activo o no).
        bits.append(byte & (1 << (i % 8)));
    }
    return bits;
}

// Método para leer entradas discretas a partir de una dirección y cantidad.
bool ModbusTcpClient::readDiscreteInputs(quint16 address, quint16 count, QVector<bool> &ret)
{
    // Código de función Modbus para leer entradas discretas.
    const quint8 FUNCTION_CODE = 0x02;

    // Se prepara el PDU (Unidad de Datos del Protocolo) para la solicitud.
    QByteArray pdu;
    QDataStream pduStream(&pdu, QIODevice::WriteOnly);
    pduStream.setByteOrder(QDataStream::BigEndian); // Se usa el orden de bytes Big Endian.
    pduStream << FUNCTION_CODE << address << count;

    QByteArray responsePDU;
    // Se envía la solicitud y se espera la respuesta.
    if (!sendRequest(pdu, responsePDU))
        return false;

    // Se procesa la respuesta recibida.
    QDataStream respStream(responsePDU);
    respStream.setByteOrder(QDataStream::BigEndian);
    quint8 functionCode, byteCount;
    respStream >> functionCode >> byteCount;
    // Se verifica que la respuesta tenga el código de función y la cantidad de bytes esperados.
    if (functionCode != FUNCTION_CODE || byteCount != (count + 7) / 8) {
        qCritical() << "Respuesta inesperada para readCoil: Recibido: f==" << functionCode
                    << " count==" << byteCount;
        return false;
    }

    // Se desempaquetan los bits y se guardan en 'ret'.
    ret = unpackModbusBits(respStream, count);
    return true;
}

// Método para leer registros a partir de una dirección y cantidad.
bool ModbusTcpClient::readRegisters(quint16 startAddress, quint16 count, QVector<quint16> &registers)
{
    qDebug() << __FUNCTION__ << " no implementada";
    return false;

    // Código de función Modbus para leer registros.
    const quint8 FUNCTION_CODE = 0x04;

    // Se prepara el PDU con la dirección inicial y la cantidad de registros a leer.
    QByteArray pdu;
    QDataStream pduStream(&pdu, QIODevice::WriteOnly);
    pduStream.setByteOrder(QDataStream::BigEndian);
    // TODO: Codificar petición: pduStream << ...;

    QByteArray responsePDU;
    // Se envía la solicitud y se espera la respuesta.
    if (!sendRequest(pdu, responsePDU))
        return false;

    // Se procesa la respuesta.
    QDataStream respStream(responsePDU);
    respStream.setByteOrder(QDataStream::BigEndian);

    quint8 functionCode;
    // TODO: Decodificar respuesta:
    // respStream >> functionCode >> ...;

    // Se verifica que la respuesta tenga el formato correcto.
    if (functionCode != FUNCTION_CODE /* TODO: verificar el resto de cosas verificables */) {
        qCritical() << "Respuesta inesperada para readRegisters.";
        return false;
    }

    registers.clear();
    // Se leen cada uno de los registros (cada registro consta de 2 bytes) y se añaden al vector.
    for (int i = 0; i < count; ++i) {
        quint16 reg;
        respStream >> reg;
        registers.append(reg);
    }
    return true;
}


// Método para escribir múltiples bobinas (coils) en el servidor Modbus.
bool ModbusTcpClient::writeCoils(quint16 address, const QVector<bool> &values)
{
    qDebug() << __FUNCTION__ << " no implementada";
    return false;
}
