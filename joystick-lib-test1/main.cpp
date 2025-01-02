#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "joystickmodbustcpmaster.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    auto joystick = new JoystickModbusTcpMaster(&app);

    // Ensure the Modbus device is initialized
    if (!joystick->modbusDevice()) {
        qDebug() << "Modbus device is not initialized.";
        return -1;
    }

    // Connect to the stateChanged signal
    QObject::connect(joystick->modbusDevice(),
                     &QModbusTcpClient::stateChanged,
                     [&](QModbusDevice::State state) {
                         if (state == QModbusDevice::ConnectedState) {
                             qDebug() << "Conexión establecida con el servidor Modbus.";

                             // Perform asynchronous joystick read
                             QModbusReply *reply = joystick->readJoystick();
                             if (!reply) {
                                 qDebug() << "No se pudo iniciar la lectura del joystick.";
                                 app.quit();
                                 return;
                             }

                             QObject::connect(reply, &QModbusReply::finished, [&]() {
                                 if (reply->error() == QModbusDevice::NoError) {
                                     QModbusDataUnit unidad = reply->result();
                                     qDebug() << "Posición del joystick:" << unidad.values();
                                 } else {
                                     qDebug()
                                         << "Error al leer el joystick:" << reply->errorString();
                                 }
                                 reply->deleteLater();
                                 app.quit(); // Exit the application after processing the read
                             });
                         } else if (state == QModbusDevice::UnconnectedState) {
                             qDebug() << "Desconectado.";
                         }
                     });

    // Start the connection
    qDebug() << "Intentando conectar...";
    bool inicioConexion = joystick->connectToServer("localhost", 1502);
    if (!inicioConexion) {
        qDebug() << "Error al iniciar la conexión.";
        return -1;
    }

    qDebug() << "antes de app.exec()";
    return app.exec();
}
