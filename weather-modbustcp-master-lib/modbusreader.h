#ifndef MODBUSREADER_H
#define MODBUSREADER_H

#include <QBitArray>
#include <QObject>
#include <QVector>

class ModbusReaderPrivate; //forward declaration for pimpl

class ModbusReader : public QObject
{
    Q_OBJECT
public:
    explicit ModbusReader(const QString &ipAddress = "localhost",
                          int port = 1502,
                          int deviceAddress = 1,
                          QObject *parent = nullptr);
    ~ModbusReader();

    QVector<quint16> getInputRegisters(int count = 1, int first = 0);
    QBitArray getDiscreteCoils(int count = 1, int first = 0);

    void readInputRegistersAsync(int count = 1, int first = 0);
    void readDiscreteCoilsAsync(int count = 1, int first = 0);

    bool connectDevice();
    void disconnectDevice();
    bool isConnected() const;
    QString errorString() const;

signals:
    void inputRegistersReady(const QVector<quint16> &registers);
    void discreteCoilsReady(const QBitArray &coils);
    void modbusError(const QString &errorString);

private slots:
    void handleInputRegistersReply();
    void handleDiscreteCoilsReply();

private:
    std::unique_ptr<ModbusReaderPrivate> pimpl;
};

#endif // MODBUSREADER_H
