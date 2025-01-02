#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QRandomGenerator>
#include <QTimer>
#include <QtSerialBus/QModbusRtuSerialServer>
#include <QtSerialPort/QtSerialPort>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Configuración del parser de línea de comandos
    QCommandLineParser parser;
    parser.setApplicationDescription("Esclavo Modbus RTU");
    parser.addHelpOption();

    // Opción para especificar el puerto serie
    QCommandLineOption portOption(QStringList() << "p" << "port",
                                  "Nombre del puerto serie.",
                                  "port",
                                  "/tmp/modbus-slave");
    parser.addOption(portOption);

    // Opción para especificar la dirección del esclavo
    QCommandLineOption slaveIdOption(QStringList() << "s" << "slave-id",
                                     "Dirección del esclavo Modbus (1-247).",
                                     "slave-id",
                                     "1");
    parser.addOption(slaveIdOption);

    // Procesar los argumentos
    parser.process(a);

    QString port = parser.value(portOption);
    bool ok;
    int slaveId = parser.value(slaveIdOption).toInt(&ok);
    if (!ok || slaveId < 1 || slaveId > 247) {
        qWarning() << "La dirección del esclavo debe ser un número entre 1 y 247.";
        return -1;
    }

    // Crear el servidor Modbus RTU
    QModbusRtuSerialServer server;
    server.setServerAddress(slaveId);

    // Configuración del puerto serie
    server.setConnectionParameter(QModbusDevice::SerialPortNameParameter, port);
    server.setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud9600);
    server.setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
    server.setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
    server.setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);

    // Conectar el dispositivo
    if (!server.connectDevice()) {
        qWarning() << "No se pudo iniciar el esclavo RTU en el puerto:" << port;
        return -1;
    }

    // Configurar el mapa de registros
    QModbusDataUnitMap regMap;
    QModbusDataUnit hrUnit(QModbusDataUnit::HoldingRegisters, 0, 10);
    regMap.insert(QModbusDataUnit::HoldingRegisters, hrUnit);
    if (!server.setMap(regMap)) {
        qWarning() << "Error al configurar el mapa de registros en el servidor.";
        return -1;
    }

    // Inicializar valores en los Holding Registers
    server.setData(QModbusDataUnit::HoldingRegisters, 0, 25); // Temp. inicial
    server.setData(QModbusDataUnit::HoldingRegisters, 1, 0);  // LED apagado

    // Conectar la señal de escritura
    QObject::connect(&server,
                     &QModbusServer::dataWritten,
                     [&](QModbusDataUnit::RegisterType table, int address, int size) {
                         if (table == QModbusDataUnit::HoldingRegisters && address == 1
                             && size == 1) {
                             quint16 ledVal = 0;
                             if (server.data(QModbusDataUnit::HoldingRegisters, 1, &ledVal)) {
                                 qDebug() << "[Esclavo] LED escrito con valor:" << ledVal;
                             } else {
                                 qWarning() << "[Esclavo] No se pudo leer el LED del registro 1";
                             }
                         }
                     });

    // Temporizador para simular cambios de temperatura
    QTimer tempTimer;
    QObject::connect(&tempTimer, &QTimer::timeout, [&]() {
        int newTemp = 20 + QRandomGenerator::global()->bounded(15);
        server.setData(QModbusDataUnit::HoldingRegisters, 0, newTemp);
        qDebug() << "[Esclavo] Temperatura simulada =" << newTemp << "°C";
    });
    tempTimer.start(3000); // cada 3 segundos

    qDebug() << "Esclavo Modbus RTU en marcha. Puerto:" << port << ", slaveID:" << slaveId;

    return a.exec();
}
