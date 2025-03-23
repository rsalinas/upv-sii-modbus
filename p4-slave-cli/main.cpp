#include <QCoreApplication>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>

#include "modbustcp-slave.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Crear una instancia del servidor Modbus TCP
    ModbusTcpSlave slave;

    // Inicializar los datos simulados desde el main
    slave.setCoils(QVector<bool>(3, false));
    slave.setDiscreteInputs(QVector<bool>(2, true));
    slave.setInputRegisters(QVector<quint16>() << 1024 << 25);

    qDebug() << "Datos simulados inicializados:";
    qDebug() << "Coils:" << slave.getCoils();
    qDebug() << "Discrete Inputs:" << slave.getDiscreteInputs();
    qDebug() << "Input Registers:" << slave.getInputRegisters();

    // Conectar la señal de cambio de coil a un slot de ejemplo
    QObject::connect(&slave, &ModbusTcpSlave::coilChanged, [](int index, bool value) {
        qDebug() << "Coil" << index << "cambió a" << value;
    });

    // Iniciar el servidor Modbus TCP
    if (!slave.listen(QHostAddress::Any, 1502)) {
        qCritical() << "No se pudo iniciar el servidor Modbus TCP.";
        return 1;
    }

    qDebug() << "Servidor Modbus TCP iniciado en el puerto 1502.";

    return app.exec();
}
