#include <QCoreApplication>
#include <QTimer>

#include "modbustcp-master.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    bool oneShot = false;
    if (argc > 1 && QString(argv[1]) == "-1") {
        oneShot = true;
    }

    ModbusTcpClient modbus;
    if (!modbus.connectToHost("127.0.0.1", 1502)) {
        qCritical() << "Failed to connect to Modbus server.";
        return 1;
    }

    static bool toggle = false;

    QTimer timer;
    auto go = [&]() {
        if (true) {
            QVector<bool> values;
            if (!modbus.readDiscreteInputs(0, 2, values)) {
                qFatal() << "Error reading discrete inputs";
            }
            qDebug() << "Discrete inputs: " << values;
        }
        if (true) {
            bool coil0, coil1;
            if (modbus.readCoil(0, coil0) && modbus.readCoil(1, coil1)) {
                qDebug() << "Coils: Coil 0:" << coil0 << ", Coil 1:" << coil1;
            } else {
                qDebug() << "Error reading coils.";
            }
        }

        if (true) {
            QVector<quint16> registers;
            if (modbus.readRegisters(0, 2, registers)) {
                qDebug() << "Register 0:" << registers.at(0) << ", Register 1:" << registers.at(1);
            } else {
                qDebug() << "Error reading registers.";
            }
        }

        if (true) {
            for (int i = 0; i < 3; ++i) {
                if (!modbus.writeCoil(i, toggle)) {
                    qDebug() << "Error writing to coil" << i;
                }
                bool currentValue;
                if (!modbus.readCoil(i, currentValue)) {
                    qDebug() << "Error reading from coil" << i;
                }
                if (toggle != currentValue) {
                    qFatal() << "Wrong value after setting coil";
                } else {
                    qDebug() << "coil check OK";
                }
            }
            toggle = !toggle;
        }

        if (oneShot) {
            QCoreApplication::quit();
        }
    };

    if (oneShot) {
        QTimer::singleShot(0, go);
    } else {
        QObject::connect(&timer, &QTimer::timeout, go);
        timer.start(1000);
    }

    return app.exec();
}
