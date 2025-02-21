#include <QCommandLineParser>
#include <QCoreApplication>
#include <QSerialPortInfo>
#include "ModbusSlave.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef __linux__
    system("set -x ; ! killall socat -v; socat -d -d "
           "pty,raw,echo=0,link=/tmp/modbus-slave "
           "pty,raw,echo=0,wait-slave,link=/tmp/modbus-master < /dev/null &");
#endif

    qDebug() << "Puertos serie disponibles:";
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &port : ports) {
        if (port.systemLocation().startsWith("/dev/pts/")) {
            qDebug() << "Nombre del puerto:" << port.portName();
            qDebug() << "Descripción:" << port.description();
            qDebug() << "Fabricante:" << port.manufacturer();
            qDebug() << "Número de serie:" << port.serialNumber();
            qDebug() << "Sistema operativo local:" << port.systemLocation();
            qDebug() << "---------------------------";
        } else {
            //qDebug() << "Ignored: " << port.systemLocation();
        }
    }

    QCommandLineParser parser;
    parser.setApplicationDescription("Modbus-TCP Weather station");
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
