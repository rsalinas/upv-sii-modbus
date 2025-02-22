#ifndef METEOCLIENT_H
#define METEOCLIENT_H

#include <QBitArray>
#include <QObject>
#include <QVector>
#include <memory>

// Structure containing meteorological information.
struct MeteoInfo {
    int pressure;
    int temp;
};

class MeteoClientPrivate; // Forward declaration for the Pimpl

class MeteoClient : public QObject
{
    Q_OBJECT
public:
    explicit MeteoClient(QObject *parent = nullptr,
                         const QString &ipAddress = "127.0.0.1",
                         int port = 1502,
                         int deviceAddress = 1);
    ~MeteoClient();

    // Connects to the weather station.
    bool connectDevice();

    // Disconnects from the Modbus device.
    void disconnectDevice();

    // Returns true if the device is connected.
    bool isConnected() const;

    // Returns the current error string.
    QString errorString() const;

    // Reads the first two input registers and returns a MeteoInfo (pressure and temperature).
    MeteoInfo getMeteoInfo();

    // Reads the button.
    bool isButtonPressed();

private:
    std::unique_ptr<MeteoClientPrivate> pimpl;
};

// For easy pretty printing
std::ostream &operator<<(std::ostream &os, const MeteoInfo &info);
QDebug operator<<(QDebug dbg, const MeteoInfo &info);

#endif // METEOCLIENT_H
