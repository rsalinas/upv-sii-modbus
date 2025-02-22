#include <QCoreApplication>
#include "meteoclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    MeteoClient client;

    if (!client.connectDevice()) {
        qFatal() << "Failed to connect to the Modbus device:" << client.errorString();
    }

    MeteoInfo info = client.getMeteoInfo();
    qInfo() << info;

    qInfo() << client.isButtonPressed();
}
