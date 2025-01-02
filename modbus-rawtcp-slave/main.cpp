#include <QCoreApplication>
#include <QDebug>
#include "modbustcpslave.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    ModbusTcpSlave server;
    quint16 port = 502; // Puerto estándar Modbus TCP
    if (!server.listen(QHostAddress::Any, port)) {
        qCritical() << "Error al iniciar el servidor:" << server.errorString();
        return 1;
    }
    qDebug() << "Servidor Modbus TCP escuchando en el puerto" << port;
    return app.exec();
}

