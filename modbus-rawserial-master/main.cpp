#include <QCoreApplication>

#include "modbusmaster.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ModbusMaster master("/tmp/modbus-master");
    char op = 0;
    bool led = false;
    quint16 threshold = 50;
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &master, [&]() {
        switch (op++) {
        case 0:
            master.readTemperature(1); // Leer temperatura del esclavo con ID=1

            break;
        case 1:
            master.writeLed(1, led); // Encender/Apagar LED del esclavo con ID=1
            led = !led;

            break;
        case 2:
            // Leer el valor binario del registro 2 (último valor escrito)
            qDebug() << "readcoil";
            master.readCoil(1, 2);
            break;
        case 3:
            master.writeRegister(1, 2, threshold); // Escribir umbral en el registro binario 2
            threshold = (threshold + 10) % 100;    // Actualizar el umbral cíclicamente
            op = 0;
            break;
        }
    });
    timer.start(1);

    return a.exec();
}
