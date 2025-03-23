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

    // Métodos para modificar y obtener los datos simulados desde fuera
    void setCoils(const QVector<bool> &newCoils);
    QVector<bool> getCoils() const;

    void setDiscreteInputs(const QVector<bool> &newDiscreteInputs);
    QVector<bool> getDiscreteInputs() const;

    void setInputRegisters(const QVector<quint16> &newInputRegisters);
    QVector<quint16> getInputRegisters() const;

signals:
    // Señal emitida cuando una coil cambia de valor
    void coilChanged(int index, bool value);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    void processRequest(QTcpSocket *socket, const QByteArray &request);
    QByteArray createResponse(const QByteArray &request);

    // Datos simulados del esclavo
    QVector<bool> coils;             // Bobinas (red, green, blue)
    QVector<bool> discreteInputs;    // Entradas discretas (boton0, boton1)
    QVector<quint16> inputRegisters; // Registros de entrada (pressure, temperature)

    void processReadCoilsRequest(QDataStream &stream, QDataStream &responseStream);
    void processReadDiscreteInputsRequest(QDataStream &stream, QDataStream &responseStream);
    void processReadInputRegistersRequest(QDataStream &stream, QDataStream &responseStream);
    void processWriteMultipleCoilsRequest(QDataStream &stream, QDataStream &responseStream);

    // Método para actualizar una coil y emitir la señal si es necesario
    void updateCoil(int index, bool value);
};

#endif // MODBUSTCPSLAVE_H
