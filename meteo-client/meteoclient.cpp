#include "meteoclient.h"
#include <QDebug>
#include <QEventLoop>
#include <QModbusReply>
#include <QTimer>
#include <QVariant>
#include <QtSerialBus/QModbusDataUnit>
#include <QtSerialBus/QModbusTcpClient>
#include <ostream>

// Private class that hides the implementation details.
class MeteoClientPrivate
{
public:
    MeteoClientPrivate(const QString &ip, int port, int deviceAddr, QObject *parent)
        : deviceAddress(deviceAddr)
    {
        modbusDevice = new QModbusTcpClient(parent);
        qDebug() << QString("MeteoClient connecting to %1:%2, addr==%3")
                        .arg(ip)
                        .arg(port)
                        .arg(deviceAddr);
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, ip);
        modbusDevice->setTimeout(1000);
        modbusDevice->setNumberOfRetries(3);
    }
    ~MeteoClientPrivate()
    {
        if (modbusDevice) {
            modbusDevice->disconnectDevice();
            modbusDevice->deleteLater();
        }
    }
    // Synchronously reads the specified number of input registers starting at 'first'
    QVector<quint16> getInputRegisters(int count = 1, int first = 0);

    // Synchronously reads the specified number of discrete coils starting at 'first'
    QBitArray getDiscreteCoils(int count = 1, int first = 0);
    QModbusTcpClient *modbusDevice = nullptr;
    const int deviceAddress;
};

MeteoClient::MeteoClient(QObject *parent, const QString &ipAddress, int port, int deviceAddress)
    : QObject(parent)
    , pimpl(std::make_unique<MeteoClientPrivate>(ipAddress, port, deviceAddress, this))
{}

MeteoClient::~MeteoClient()
{
    disconnectDevice();
}

bool MeteoClient::connectDevice()
{
    if (!pimpl->modbusDevice)
        return false;
    bool initiated = pimpl->modbusDevice->connectDevice();
    if (!initiated) {
        qWarning() << "Error initiating connection:" << pimpl->modbusDevice->errorString();
        return false;
    }

    // Wait for the device to become fully connected.
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(2000); // 2 second timeout
    QObject::connect(pimpl->modbusDevice,
                     &QModbusTcpClient::stateChanged,
                     [&loop](QModbusDevice::State state) {
                         if (state == QModbusDevice::ConnectedState)
                             loop.quit();
                     });
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();

    if (pimpl->modbusDevice->state() != QModbusDevice::ConnectedState) {
        qWarning() << "Failed to connect:" << pimpl->modbusDevice->errorString();
        return false;
    }
    qDebug() << "Connected to"
             << pimpl->modbusDevice->connectionParameter(QModbusDevice::NetworkAddressParameter)
                    .toString()
             << ":"
             << pimpl->modbusDevice->connectionParameter(QModbusDevice::NetworkPortParameter).toInt()
             << "addr==" << pimpl->deviceAddress;
    return true;
}

void MeteoClient::disconnectDevice()
{
    if (pimpl->modbusDevice)
        pimpl->modbusDevice->disconnectDevice();
}

bool MeteoClient::isConnected() const
{
    return pimpl->modbusDevice && pimpl->modbusDevice->state() == QModbusDevice::ConnectedState;
}

QString MeteoClient::errorString() const
{
    return pimpl->modbusDevice ? pimpl->modbusDevice->errorString() : QString();
}

QVector<quint16> MeteoClientPrivate::getInputRegisters(int count, int first)
{
    //qDebug() << __FUNCTION__ << "first=" << first << " count=" << count;
    QVector<quint16> registers;
    if (!modbusDevice) {
        qDebug() << "modbusDevice is null";
        return registers;
    }
    QModbusDataUnit request(QModbusDataUnit::InputRegisters, first, count);
    QModbusReply *reply = modbusDevice->sendReadRequest(request, deviceAddress);
    if (!reply) {
        qWarning() << "Error sending request:" << modbusDevice->errorString() << "to addr"
                   << deviceAddress << "first==" << first << ", count=" << count;
        return registers;
    }
    QEventLoop loop;
    QObject::connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        registers.resize(unit.valueCount());
        for (uint i = 0; i < unit.valueCount(); i++) {
            registers[i] = unit.value(i);
        }
    } else {
        qWarning() << "Modbus response error:" << reply->errorString();
    }
    reply->deleteLater();
    //qDebug() << "Returning" << registers.size() << "values";
    return registers;
}

QBitArray MeteoClientPrivate::getDiscreteCoils(int count, int first)
{
    QBitArray coils(count);
    if (!modbusDevice)
        return coils;
    QModbusDataUnit request(QModbusDataUnit::DiscreteInputs, first, count);
    QModbusReply *reply = modbusDevice->sendReadRequest(request, deviceAddress);
    if (!reply) {
        qWarning() << "Error sending request:" << modbusDevice->errorString();
        return coils;
    }
    QEventLoop loop;
    QObject::connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (uint i = 0; i < count && i < unit.valueCount(); i++) {
            coils.setBit(i, unit.value(i) != 0);
        }
    } else {
        qWarning() << "Modbus response error:" << reply->errorString() << reply->error();
    }
    reply->deleteLater();
    return coils;
}

MeteoInfo MeteoClient::getMeteoInfo()
{
    QVector<quint16> registers = pimpl->getInputRegisters(2, 0);
    MeteoInfo info{0, 0};
    if (registers.size() >= 2) {
        info.pressure = static_cast<int>(registers[0]);
        info.temp = static_cast<int>(registers[1]);
    } else {
        qFatal() << "Insufficient registers received for MeteoInfo.";
    }
    return info;
}

bool MeteoClient::isButtonPressed()
{
    return pimpl->getDiscreteCoils()[0];
}

std::ostream &operator<<(std::ostream &os, const MeteoInfo &info)
{
    os << "Pressure: " << info.pressure << ", Temperature: " << info.temp;
    return os;
}

QDebug operator<<(QDebug dbg, const MeteoInfo &info)
{
    dbg.nospace() << "Pressure: " << info.pressure << ", Temperature: " << info.temp;
    return dbg.space();
}
