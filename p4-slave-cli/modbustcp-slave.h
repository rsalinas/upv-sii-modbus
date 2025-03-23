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
    void setCoil(int index, bool value); // Cambiar una coil individual
    bool getCoil(int index) const;       // Obtener una coil individual

    void setDiscreteInputs(const QVector<bool> &newDiscreteInputs);
    QVector<bool> getDiscreteInputs() const;
    void setDiscreteInput(int index, bool value); // Cambiar una entrada discreta individual
    bool getDiscreteInput(int index) const;       // Obtener una entrada discreta individual

    void setInputRegisters(const QVector<quint16> &newInputRegisters);
    QVector<quint16> getInputRegisters() const;
    void setInputRegister(int index, quint16 value); // Cambiar un registro de entrada individual
    quint16 getInputRegister(int index) const;       // Obtener un registro de entrada individual

signals:
    // Señales emitidas cuando cambian los valores
    void coilChanged(int index, bool value);
    void discreteInputChanged(int index, bool value);
    void inputRegisterChanged(int index, quint16 value);

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

    // Métodos para actualizar valores y emitir señales
    void updateCoil(int index, bool value);
    void updateDiscreteInput(int index, bool value);
    void updateInputRegister(int index, quint16 value);
};

#endif // MODBUSTCPSLAVE_H
