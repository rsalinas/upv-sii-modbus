#include <QCommandLineParser>
#include <QCoreApplication>
#include "ModbusSlave.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Esclavo Modbus RTU");
    parser.addHelpOption();

    QCommandLineOption portOption({"p", "port"},
                                  "Nombre del puerto serie.",
                                  "port",
                                  "/tmp/modbus-slave");
    parser.addOption(portOption);

    QCommandLineOption slaveIdOption({"s", "slave-id"},
                                     "Dirección del esclavo Modbus (1-247).",
                                     "slave-id",
                                     "1");
    parser.addOption(slaveIdOption);

    parser.process(a);

    QString port = parser.value(portOption);
    bool ok;
    int slaveId = parser.value(slaveIdOption).toInt(&ok);
    if (!ok || slaveId < 1 || slaveId > 247) {
        qWarning() << "La dirección del esclavo debe ser un número entre 1 y 247.";
        return -1;
    }

    ModbusSlave slave;
    qDebug() << "opening, port=" << port;
    if (!slave.initialize(port, slaveId)) {
        qWarning() << "Error initializing.";
        return -1;
    }

    return a.exec();
}
