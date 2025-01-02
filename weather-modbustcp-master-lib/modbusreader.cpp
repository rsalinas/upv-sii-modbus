#include "modbusreader.h"
#include <QDebug>
#include <QEventLoop>
#include <QModbusReply>
#include <QVariant>
#include <QtSerialBus/QModbusDataUnit>
#include <QtSerialBus/QModbusTcpClient>

// Clase privada que oculta la implementación
class ModbusReaderPrivate
{
public:
    ModbusReaderPrivate(const QString &ip, int port, int deviceAddr, QObject *parent)
        : deviceAddress(deviceAddr)
    {
        modbusDevice = new QModbusTcpClient(parent);
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, ip);
        modbusDevice->setTimeout(1000);
        modbusDevice->setNumberOfRetries(3);
        qDebug() << QString("ModbusReader creado para %1:%2 addr==%3")
                        .arg(ip)
                        .arg(port)
                        .arg(deviceAddress);
    }
    ~ModbusReaderPrivate()
    {
        if (modbusDevice) {
            modbusDevice->disconnectDevice();
            modbusDevice->deleteLater();
        }
    }
    QModbusTcpClient *modbusDevice = nullptr;
    const int deviceAddress;
};

ModbusReader::ModbusReader(const QString &ipAddress, int port, int deviceAddress, QObject *parent)
    : QObject(parent)
    , pimpl(std::make_unique<ModbusReaderPrivate>(ipAddress, port, deviceAddress, this))
{}

ModbusReader::~ModbusReader() = default;

bool ModbusReader::connectDevice()
{
    if (!pimpl->modbusDevice)
        return false;
    bool connected = pimpl->modbusDevice->connectDevice();
    if (!connected) {
        qWarning() << "Error al conectar:" << pimpl->modbusDevice->errorString();
    } else {
        qDebug() << "Conectado a"
                 << pimpl->modbusDevice->connectionParameter(QModbusDevice::NetworkAddressParameter)
                        .toString()
                 << ":"
                 << pimpl->modbusDevice->connectionParameter(QModbusDevice::NetworkPortParameter)
                        .toInt()
                 << "addr==" << pimpl->deviceAddress;
    }
    return connected;
}

void ModbusReader::disconnectDevice()
{
    if (pimpl->modbusDevice)
        pimpl->modbusDevice->disconnectDevice();
}

bool ModbusReader::isConnected() const
{
    return pimpl->modbusDevice && pimpl->modbusDevice->state() == QModbusDevice::ConnectedState;
}

QString ModbusReader::errorString() const
{
    return pimpl->modbusDevice ? pimpl->modbusDevice->errorString() : QString();
}

QVector<quint16> ModbusReader::getInputRegisters(int count, int first)
{
    qDebug() << __FUNCTION__ << "first=" << first << " count=" << count;
    QVector<quint16> registers;
    if (!pimpl->modbusDevice) {
        qDebug() << "modbusDevice es nulo";
        return registers;
    }
    QModbusDataUnit request(QModbusDataUnit::InputRegisters, first, count);
    QModbusReply *reply = pimpl->modbusDevice->sendReadRequest(request, pimpl->deviceAddress);
    if (!reply) {
        qWarning() << "Error al enviar la petición:" << pimpl->modbusDevice->errorString();
        return registers;
    }
    QEventLoop loop;
    connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
    qDebug() << "Entrando en loop.exec()";
    loop.exec();
    qDebug() << "Fin del loop";
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        registers.resize(unit.valueCount());
        for (uint i = 0; i < unit.valueCount(); i++) {
            registers[i] = unit.value(i);
        }
    } else {
        qWarning() << "Error en la respuesta Modbus:" << reply->errorString();
    }
    reply->deleteLater();
    qDebug() << "Devolviendo" << registers.size() << "valores";
    return registers;
}

QBitArray ModbusReader::getDiscreteCoils(int count, int first)
{
    QBitArray coils(count);
    if (!pimpl->modbusDevice)
        return coils;
    QModbusDataUnit request(QModbusDataUnit::Coils, first, count);
    QModbusReply *reply = pimpl->modbusDevice->sendReadRequest(request, pimpl->deviceAddress);
    if (!reply) {
        qWarning() << "Error al enviar la petición:" << pimpl->modbusDevice->errorString();
        return coils;
    }
    QEventLoop loop;
    connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (uint i = 0; i < unit.valueCount(); i++) {
            coils.setBit(i, unit.value(i) != 0);
        }
    } else {
        qWarning() << "Error en la respuesta Modbus:" << reply->errorString();
    }
    reply->deleteLater();
    return coils;
}

void ModbusReader::readInputRegistersAsync(int count, int first)
{
    if (!pimpl->modbusDevice) {
        emit modbusError("Dispositivo Modbus no disponible.");
        return;
    }
    QModbusDataUnit request(QModbusDataUnit::InputRegisters, first, count);
    QModbusReply *reply = pimpl->modbusDevice->sendReadRequest(request, pimpl->deviceAddress);
    if (!reply) {
        emit modbusError("Error al enviar la petición: " + pimpl->modbusDevice->errorString());
        return;
    }
    connect(reply, &QModbusReply::finished, this, &ModbusReader::handleInputRegistersReply);
}

void ModbusReader::readDiscreteCoilsAsync(int count, int first)
{
    if (!pimpl->modbusDevice) {
        emit modbusError("Dispositivo Modbus no disponible.");
        return;
    }
    QModbusDataUnit request(QModbusDataUnit::DiscreteInputs, first, count);
    QModbusReply *reply = pimpl->modbusDevice->sendReadRequest(request, pimpl->deviceAddress);
    if (!reply) {
        emit modbusError("Error al enviar la petición: " + pimpl->modbusDevice->errorString());
        return;
    }
    connect(reply, &QModbusReply::finished, this, &ModbusReader::handleDiscreteCoilsReply);
}

void ModbusReader::handleInputRegistersReply()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;
    QVector<quint16> registers;
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        registers.resize(unit.valueCount());
        for (uint i = 0; i < unit.valueCount(); i++) {
            registers[i] = unit.value(i);
        }
        emit inputRegistersReady(registers);
    } else {
        emit modbusError("Error en la respuesta Modbus: " + reply->errorString());
    }
    reply->deleteLater();
}

void ModbusReader::handleDiscreteCoilsReply()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;
    QBitArray coils;
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        coils.resize(unit.valueCount());
        for (uint i = 0; i < unit.valueCount(); i++) {
            coils.setBit(i, unit.value(i) != 0);
        }
        emit discreteCoilsReady(coils);
    } else {
        emit modbusError("Error en la respuesta Modbus: " + reply->errorString());
    }
    reply->deleteLater();
}
