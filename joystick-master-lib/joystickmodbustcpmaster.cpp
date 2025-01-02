#include "joystickmodbustcpmaster.h"
#include <QDebug>
#include <QModbusDataUnit>
#include <QVariant>

JoystickModbusTcpMaster::JoystickModbusTcpMaster(QObject *parent)
    : QObject(parent)
{
    m_modbusDevice = new QModbusTcpClient(this);
}

JoystickModbusTcpMaster::~JoystickModbusTcpMaster()
{
    if (m_modbusDevice)
        m_modbusDevice->disconnectDevice();
}

bool JoystickModbusTcpMaster::connectToServer(const QString &host, int port)
{
    if (!m_modbusDevice)
        return false;

    m_modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, host);
    m_modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    m_modbusDevice->setTimeout(1000);
    m_modbusDevice->setNumberOfRetries(3);
    return m_modbusDevice->connectDevice();
}

void JoystickModbusTcpMaster::disconnectFromServer()
{
    if (m_modbusDevice)
        m_modbusDevice->disconnectDevice();
}

QModbusTcpClient *JoystickModbusTcpMaster::modbusDevice() const
{
    return m_modbusDevice;
}

QModbusReply *JoystickModbusTcpMaster::writeBinaryLeds(const QVector<bool> &ledStates)
{
    if (!m_modbusDevice)
        return nullptr;

    if (ledStates.size() < 3) {
        qWarning() << "writeBinaryLeds: Se esperan al menos 3 estados.";
        return nullptr;
    }

    // Definimos un QModbusDataUnit para coils (iniciando en la dirección 0, 3 registros)
    QModbusDataUnit writeUnit(QModbusDataUnit::Coils, 0, 3);
    for (int i = 0; i < 3; ++i)
        writeUnit.setValue(i, ledStates.at(i) ? 1 : 0);

    // Se asume que el slave tiene dirección 1.
    QModbusReply *reply = m_modbusDevice->sendWriteRequest(writeUnit, 1);
    if (!reply)
        qWarning() << "Error al enviar solicitud de escritura de LEDs binarios:"
                   << m_modbusDevice->errorString();
    else if (reply->isFinished())
        reply->deleteLater();

    return reply;
}

QModbusReply *JoystickModbusTcpMaster::readButtons()
{
    if (!m_modbusDevice)
        return nullptr;

    // Lectura de 2 entradas discretas (iniciando en la dirección 0)
    QModbusDataUnit readUnit(QModbusDataUnit::DiscreteInputs, 0, 2);
    QModbusReply *reply = m_modbusDevice->sendReadRequest(readUnit, 1);
    if (!reply)
        qWarning() << "Error al enviar solicitud de lectura de botones:"
                   << m_modbusDevice->errorString();

    return reply;
}

QModbusReply *JoystickModbusTcpMaster::readJoystick()
{
    qDebug() << __FUNCTION__;
    if (!m_modbusDevice)
        return nullptr;

    // Lectura de 2 registros de entrada (iniciando en la dirección 0)
    QModbusDataUnit readUnit(QModbusDataUnit::InputRegisters, 0, 2);
    qDebug() << __FUNCTION__ << "About to sendReadRequest";
    QModbusReply *reply = m_modbusDevice->sendReadRequest(readUnit, 0);
    if (!reply)
        qWarning() << "Error al enviar solicitud de lectura del joystick:"
                   << m_modbusDevice->errorString();
    qDebug() << "about to reply";
    return reply;
}

QModbusReply *JoystickModbusTcpMaster::writePwmLeds(const QVector<quint16> &pwmValues)
{
    if (!m_modbusDevice)
        return nullptr;

    if (pwmValues.size() < 3) {
        qWarning() << "writePwmLeds: Se esperan al menos 3 valores.";
        return nullptr;
    }

    // Escritura en 3 registros de retención (iniciando en la dirección 0)
    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, 0, 3);
    for (int i = 0; i < 3; ++i)
        writeUnit.setValue(i, pwmValues.at(i));

    QModbusReply *reply = m_modbusDevice->sendWriteRequest(writeUnit, 1);
    if (!reply)
        qWarning() << "Error al enviar solicitud de escritura de LEDs PWM:"
                   << m_modbusDevice->errorString();
    else if (reply->isFinished())
        reply->deleteLater();

    return reply;
}
