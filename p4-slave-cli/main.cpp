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

    // Conectar las señales de cambio a slots de ejemplo
    QObject::connect(&slave, &ModbusTcpSlave::coilChanged, [](int index, bool value) {
        qDebug() << "Coil" << index << "cambió a" << value;
    });

    QObject::connect(&slave, &ModbusTcpSlave::discreteInputChanged, [](int index, bool value) {
        qDebug() << "Discrete Input" << index << "cambió a" << value;
    });

    QObject::connect(&slave, &ModbusTcpSlave::inputRegisterChanged, [](int index, quint16 value) {
        qDebug() << "Input Register" << index << "cambió a" << value;
    });

    // Modificar valores individuales
    slave.setCoil(0, true);           // Cambiar la coil 0 a true
    slave.setDiscreteInput(1, false); // Cambiar la entrada discreta 1 a false
    slave.setInputRegister(0, 2048);  // Cambiar el registro de entrada 0 a 2048

    // Iniciar el servidor Modbus TCP
    if (!slave.listen(QHostAddress::Any, 1502)) {
        qCritical() << "No se pudo iniciar el servidor Modbus TCP.";
        return 1;
    }

    qDebug() << "Servidor Modbus TCP iniciado en el puerto 1502.";

    return app.exec();
}
