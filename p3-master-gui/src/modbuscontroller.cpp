#include "modbuscontroller.h"

ModbusController::ModbusController(QObject *parent)
    : QObject(parent)
{}

bool ModbusController::connectToHost(const QString &host, quint16 port)
{
    return modbus.connectToHost(host, port);
}

bool ModbusController::readData(quint16 &pressure, quint16 &temperature)
{
    QVector<quint16> regs;
    if (modbus.readRegisters(0, 2, regs)) {
        pressure = regs[0];
        temperature = regs[1];
        return true;
    }
    return false;
}

bool ModbusController::setLEDs(bool r, bool g, bool b)
{
    return modbus.writeCoils(0, QVector<bool>() << r << g << b);
}
