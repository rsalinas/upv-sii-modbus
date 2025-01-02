#ifndef MODBUSTCPSLAVE_H
#define MODBUSTCPSLAVE_H

#include <QTcpServer>
#include <QVector>

class ModbusTcpSlave : public QTcpServer {
    Q_OBJECT
public:
    explicit ModbusTcpSlave(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QVector<bool> m_coils;
    QVector<quint16> m_registers;

    void processData(QTcpSocket *socket);
    QByteArray processPDU(const QByteArray &pdu, quint8 unitId);
    QByteArray modbusExceptionResponse(quint8 functionCode, quint8 exceptionCode);
};

#endif // MODBUSTCPSLAVE_H
#ifndef MODBUSTCPSLAVE_H
#define MODBUSTCPSLAVE_H

#include <QTcpServer>
#include <QVector>

class ModbusTcpSlave : public QTcpServer {
    Q_OBJECT
public:
    explicit ModbusTcpSlave(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QVector<bool> m_coils;
    QVector<quint16> m_registers;

    void processData(QTcpSocket *socket);
    QByteArray processPDU(const QByteArray &pdu, quint8 unitId);
    QByteArray modbusExceptionResponse(quint8 functionCode, quint8 exceptionCode);
};

#endif // MODBUSTCPSLAVE_H

