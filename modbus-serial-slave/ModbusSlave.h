#ifndef MODBUSSLAVE_H
#define MODBUSSLAVE_H

#include <QDebug>
#include <QModbusRtuSerialServer>
#include <QObject>
#include <QSerialPort>
#include <QTimer>

class ModbusSlave : public QObject
{
    Q_OBJECT

public:
    explicit ModbusSlave(QObject *parent = nullptr);
    ~ModbusSlave();

    bool initialize(const QString &port, int slaveId);

private:
    QModbusRtuSerialServer server;
    QTimer tempTimer;

    void setupRegisters();

signals:
    void errorOccurred(const QString &message);
    void temperatureUpdated(int newTemp);
};

#endif // MODBUSSLAVE_H#pragma once
