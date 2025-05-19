#ifndef MODBUSCONTROLLER_H
#define MODBUSCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QString>
#include "modbustcp-master.h"

class ModbusController : public QObject
{
    Q_OBJECT
public:
    explicit ModbusController(QObject *parent = nullptr);
    bool connectToHost(const QString &host, quint16 port);
    bool readData(quint16 &pressure, quint16 &temperature);
    bool setLEDs(bool r, bool g, bool b);
    bool readButtons(bool &b1, bool &b2);

private:
    ModbusTcpClient modbus;
};

#endif // MODBUSCONTROLLER_H
