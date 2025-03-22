#ifndef MODBUS_H
#define MODBUS_H

// Se incluye la librería QTcpSocket de Qt para gestionar conexiones TCP.
#include <QTcpSocket>

// Declaración de la clase ModbusTcpClient que se encarga de la comunicación con un servidor Modbus a través de TCP.
class ModbusTcpClient : public QObject
{
    Q_OBJECT // Macro de Qt para habilitar el sistema de señales y slots.

public:
    // Constructor de la clase.
    explicit ModbusTcpClient(QObject *parent = nullptr);

    // Destructor de la clase. Desconecta del servidor.
    ~ModbusTcpClient();

    // Método para conectar con el host remoto especificado, en el puerto dado.
    // 'timeout' define el tiempo máximo de espera (por defecto 1000 milisegundos).
    bool connectToHost(const QString &host, quint16 port, int timeout = 1000);

    // Método para leer entradas discretas (inputs) a partir de una dirección inicial y una cantidad.
    // Los valores leídos se almacenan en el vector 'inputs'.
    bool readDiscreteInputs(quint16 startAddress, quint16 count, QVector<bool> &inputs);

    // Método para leer registros a partir de una dirección inicial y una cantidad.
    // Los registros leídos se guardan en el vector 'registers'.
    bool readRegisters(quint16 startAddress, quint16 count, QVector<quint16> &registers);

    // Método para leer bobinas a partir de una dirección inicial y una cantidad.
    // Los registros leídos se guardan en el vector 'values'.
    bool readCoils(quint16 startAddress, quint16 count, QVector<bool> &values);

    // Método para escribir valores en las bobinas (coils) a partir de una dirección inicial.
    // Los valores a escribir se pasan en el vector 'values'.
    bool writeCoils(quint16 startAddress, const QVector<bool> &values);

private:
    // Puntero al socket TCP que se utiliza para la comunicación.
    QTcpSocket *socket;

    // Identificador de transacción que se utiliza para relacionar solicitudes y respuestas.
    quint16 transactionId;

    // Método privado que envía una solicitud (PDU) y espera la respuesta correspondiente.
    // 'timeout' especifica el tiempo máximo de espera para la respuesta.
    bool sendRequest(const QByteArray &pdu, QByteArray &responsePDU, int timeout = 1000);
};

#endif // MODBUS_H
