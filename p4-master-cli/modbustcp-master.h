#ifndef MODBUS_H
#define MODBUS_H

#include <QDataStream>
#include <QDebug>
#include <QObject>
#include <QTcpSocket>
#include <QVector>

class ModbusTcpClient : public QObject
{
    Q_OBJECT
public:
    explicit ModbusTcpClient(QObject *parent = nullptr);
    ~ModbusTcpClient();

    bool connectToHost(const QString &host, quint16 port, int timeout = 3000);
    void disconnectFromHost();

    bool readDiscreteInputs(quint16 address, quint16 count, QVector<bool> &ret);
    bool readCoil(quint16 address, bool &value);
    bool writeCoil(quint16 address, bool value);
    bool readRegisters(quint16 startAddress, quint16 count, QVector<quint16> &registers);

private:
    QTcpSocket *socket;
    quint16 transactionId;

    bool sendRequest(const QByteArray &pdu, QByteArray &responsePDU, int timeout = 1000);
};

#endif // MODBUS_H
