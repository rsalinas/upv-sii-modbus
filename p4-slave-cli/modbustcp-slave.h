#ifndef MODBUSTCPSLAVE_H
#define MODBUSTCPSLAVE_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>

class ModbusTcpSlave : public QTcpServer
{
    Q_OBJECT

public:
    explicit ModbusTcpSlave(QObject *parent = nullptr);
    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    void processRequest(QTcpSocket *socket, const QByteArray &request);
    QByteArray createResponse(const QByteArray &request);

    // Datos simulados del esclavo
    QVector<bool> coils; // Bobinas (red, green, blue)
    QVector<bool> discreteInputs; // Entradas discretas (boton0, boton1)
    QVector<quint16> inputRegisters; // Registros de entrada (pressure, temperature)

    void processReadCoilsRequest(QDataStream &stream, QDataStream &responseStream);
    void processReadDiscreteInputsRequest(QDataStream &stream, QDataStream &responseStream);
};

#endif // MODBUSTCPSLAVE_H
