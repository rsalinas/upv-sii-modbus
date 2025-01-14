#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "modbusmaster.h"
#include <unistd.h>

char op = 0;
bool led = false;
quint16 threshold = 50;

auto errorHandler = []() { QCoreApplication::exit(1); };

void runop(ModbusMaster &master);
void dale(ModbusMaster &master)
{
    QTimer::singleShot(10, [&]() { runop(master); });
}

void runop(ModbusMaster &master)
{
    //qDebug() << "Running op" << (int) op;
    if (op == 40)
        errorHandler();
    switch (op++ % 4) {
    case 0:
        master.readRegister(
            1 /* slave */,
            0 /* start addr */,
            4 /*count*/,
            [&master](const QVector<quint16> &values) {
                qDebug() << "Temperatura leída con éxito:" << values << "°C";
                dale(master);
            },
            [](const QString &error) {
                qDebug() << "Error al leer la temperatura:" << error;
                errorHandler();
            });
        break;
    case 1:
        master.writeCoil(
            1,
            1,
            led,
            [&]() {
                qDebug() << "LED escrito con éxito.";
                dale(master);
            },
            [](const QString &error) {
                qDebug() << "Error al escribir el LED:" << error;
                errorHandler();
            });
        led = !led;
        break;
    case 2:
        master.readCoil(
            1,
            0,
            4,
            [&](QVector<bool> values) {
                qDebug() << "Coils leídos. Valor:" << values;
                dale(master);
            },
            [](const QString &error) {
                qDebug() << "Error al leer el coil:" << error;
                errorHandler();
            });
        break;
    case 3:
        master.writeRegister(
            1,
            2,
            threshold,
            [&]() {
                qDebug() << "Umbral escrito con éxito.";
                dale(master);
            },
            [](const QString &error) {
                qDebug() << "Error al escribir el umbral:" << error;
                errorHandler();
            });
        threshold = (threshold + 10) % 100;
        break;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    ModbusMaster master("/tmp/modbus-master");
    dale(master);
    return a.exec();
}
