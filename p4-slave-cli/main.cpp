#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QVector>

#include "modbustcp-slave.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Crear una instancia del servidor Modbus TCP
    ModbusTcpSlave slave;
    if (!slave.listen(QHostAddress::Any, 1502)) {
        qCritical() << "No se pudo iniciar el servidor Modbus TCP.";
        return 1;
    }

    qDebug() << "Servidor Modbus TCP iniciado en el puerto 1502.";

    return app.exec();
}
