#include <QCoreApplication>
#include <QDebug>
#include <QtNetwork/QHostAddress>

#include <QRandomGenerator>
#include <QTimer>
#include <QtSerialBus/QModbusTcpServer>

// MAIN del Esclavo (servidor) Modbus TCP
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Creamos el servidor Modbus TCP
    QModbusTcpServer server;

    // Asignamos parámetros de conexión:
    server.setConnectionParameter(QModbusDevice::NetworkAddressParameter, "0.0.0.0");
    server.setConnectionParameter(QModbusDevice::NetworkPortParameter, 1502);

    // Iniciamos el servidor
    if (!server.connectDevice()) {
        qWarning() << "No se pudo iniciar el servidor Modbus TCP";
        return -1;
    }

    // Por compatibilidad con Modbus, podríamos asignar un ID
    // (en Modbus TCP no suele ser crucial, pero se puede usar).
    server.setServerAddress(1);

    // --------------------------------------------
    // Definir el mapa de registros (por ejemplo, Holding Registers de 0 a 9)
    QModbusDataUnitMap regMap;
    QModbusDataUnit hrUnit(QModbusDataUnit::HoldingRegisters, 0, 10);
    regMap.insert(QModbusDataUnit::HoldingRegisters, hrUnit);

    if (!server.setMap(regMap)) {
        qWarning() << "Error al configurar mapa de registros en el servidor TCP";
        return -1;
    }

    // Inicializamos valores (temperatura en dirección 0, LED en dirección 1)
    server.setData(QModbusDataUnit::HoldingRegisters, 0, 25); // Temperatura inicial
    server.setData(QModbusDataUnit::HoldingRegisters, 1, 0);  // LED apagado

    // Cuando el maestro escriba un registro, comprobamos si escribió el LED.
    QObject::connect(&server,
                     &QModbusServer::dataWritten,
                     [&](QModbusDataUnit::RegisterType table, int address, int size) {
                         if (table == QModbusDataUnit::HoldingRegisters && address == 1
                             && size == 1) {
                             quint16 ledVal = 0;
                             if (server.data(QModbusDataUnit::HoldingRegisters, 1, &ledVal)) {
                                 qDebug()
                                     << "[Servidor] El LED ha sido escrito con valor:" << ledVal;
                             }
                         }
                     });

    // Simulamos una temperatura aleatoria cada 3s y la actualizamos en HR[0]
    QTimer tempTimer;
    QObject::connect(&tempTimer, &QTimer::timeout, [&]() {
        int newTemp = 20 + QRandomGenerator::global()->bounded(15); // 20..34
        server.setData(QModbusDataUnit::HoldingRegisters, 0, newTemp);
        qDebug() << "[Servidor] Temperatura simulada =" << newTemp << "°C";
    });
    tempTimer.start(3000);

    qDebug() << "Servidor Modbus TCP iniciado en puerto 1502 (slave address=1)";

    return a.exec();
}
