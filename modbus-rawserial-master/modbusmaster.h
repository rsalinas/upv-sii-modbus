// modbusmaster.h
#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QObject>
#include <QQueue>
#include <QSerialPort>
#include <QTimer>
#include <QVector>
#include <functional>

class ModbusMaster : public QObject
{
    Q_OBJECT

public:
    explicit ModbusMaster(const QString &portName, QObject *parent = nullptr);
    ~ModbusMaster();

    void readRegister(int slaveId,
                      quint16 address,
                      quint16 quantity,
                      std::function<void(const QVector<quint16> &values)> onSuccess,
                      std::function<void(const QString &error)> onError);

    void writeRegister(int slaveId,
                       quint16 address,
                       quint16 value,
                       std::function<void()> onSuccess,
                       std::function<void(const QString &error)> onError);

    void readCoil(int slaveId,
                  quint16 address,
                  int quantity,
                  std::function<void(QVector<bool> coilValue)> onSuccess,
                  std::function<void(const QString &error)> onError);

    void writeCoil(int slaveId,
                   quint16 address,
                   bool value,
                   std::function<void()> onSuccess,
                   std::function<void(const QString &error)> onError);

signals:
    void responseReceived(const QByteArray &response);
    void errorOccurred(const QString &message);

private slots:
    void handleResponse();
    void processNextRequest();

private:
    struct ModbusRequest
    {
        QByteArray data;
        std::function<void(const QByteArray &response)> callbackSuccess;
        std::function<void(const QString &error)> callbackError;
        int retriesRemaining;
    };

    void appendCrc(QByteArray &data);
    bool verifyCrc(const QByteArray &data);

    QSerialPort *serialPort;
    QQueue<ModbusRequest> requestQueue;
    bool isProcessing = false;
    QTimer timeoutTimer;

    void startProcessing(const ModbusRequest &request);
    const int maxRetries = 3; // Número máximo de reintentos por solicitud

    QVector<quint16> parseRegisters(const QByteArray &response);
};

#endif // MODBUSMASTER_H
