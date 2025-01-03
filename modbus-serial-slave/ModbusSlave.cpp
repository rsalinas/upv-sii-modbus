#include "ModbusSlave.h"
#include <QRandomGenerator>

ModbusSlave::ModbusSlave(QObject *parent)
    : QObject(parent)
{
    connect(&server,
            &QModbusServer::dataWritten,
            this,
            [&](QModbusDataUnit::RegisterType table, int address, int size) {
                if (table == QModbusDataUnit::HoldingRegisters && address == 1 && size == 1) {
                    quint16 ledVal = 0;
                    if (server.data(QModbusDataUnit::HoldingRegisters, 1, &ledVal)) {
                        qDebug() << "[Esclavo] LED escrito con valor:" << ledVal;
                    } else {
                        emit errorOccurred("[Esclavo] No se pudo leer el LED del registro 1");
                    }
                }
            });

    connect(&tempTimer, &QTimer::timeout, this, [&]() {
        int newTemp = 20 + QRandomGenerator::global()->bounded(15); // 20..34
        server.setData(QModbusDataUnit::HoldingRegisters, 0, newTemp);
        qDebug() << "[Esclavo] Temperatura simulada =" << newTemp << "°C";
        emit temperatureUpdated(newTemp);
    });
}

ModbusSlave::~ModbusSlave()
{
    server.disconnectDevice();
}

bool ModbusSlave::initialize(const QString &port, int slaveId)
{
    server.setServerAddress(slaveId);

    server.setConnectionParameter(QModbusDevice::SerialPortNameParameter, port);
    server.setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud9600);
    server.setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
    server.setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
    server.setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);

    if (!server.connectDevice()) {
        emit errorOccurred("No se pudo iniciar el esclavo RTU en el puerto: " + port);
        return false;
    }

    setupRegisters();

    tempTimer.start(3000); // Simulación cada 3 segundos

    qDebug() << "Esclavo Modbus RTU inicializado. Puerto:" << port << ", slaveID:" << slaveId;
    return true;
}

void ModbusSlave::setupRegisters()
{
    QModbusDataUnitMap regMap;
    QModbusDataUnit hrUnit(QModbusDataUnit::HoldingRegisters, 0, 10);
    regMap.insert(QModbusDataUnit::HoldingRegisters, hrUnit);

    if (!server.setMap(regMap)) {
        emit errorOccurred("Error al configurar el mapa de registros en el servidor.");
        return;
    }

    server.setData(QModbusDataUnit::HoldingRegisters, 0, 25); // Temp. inicial
    server.setData(QModbusDataUnit::HoldingRegisters, 1, 0);  // LED apagado
}
