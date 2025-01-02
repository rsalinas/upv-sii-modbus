#include <QCoreApplication>

#include "modbusslave.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    ModbusSlave slave("/tmp/modbus-slave");

    return app.exec();
}
