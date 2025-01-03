#include <QCoreApplication>
#include <QDebug>
#include <QModbusDataUnit>
#include <QTimer>
#include <QVariant>
#include <QtSerialBus/QModbusTcpClient>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QModbusTcpClient client;

    client.setConnectionParameter(QModbusDevice::NetworkAddressParameter, "127.0.0.1");
    client.setConnectionParameter(QModbusDevice::NetworkPortParameter, 1502);

    if (!client.connectDevice()) {
        qWarning() << "No se pudo conectar al servidor Modbus TCP en 127.0.0.1:1502";
        return -1;
    }

    // Función para leer la temperatura
    auto readTemperature = [&](int serverId) {
        QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0, 1);
        QModbusReply *reply = client.sendReadRequest(readUnit, serverId);
        if (!reply) {
            qWarning() << "[Cliente] No se pudo enviar petición de lectura";
            return;
        }

        QObject::connect(reply, &QModbusReply::finished, [=]() {
            if (reply->error() == QModbusDevice::NoError) {
                auto result = reply->result();
                auto tempVal = result.value(0);
                qDebug() << "[Cliente] Temperatura leída:" << tempVal << "°C";
            } else {
                qWarning() << "[Cliente] Error en lectura:" << reply->errorString();
            }
            reply->deleteLater();
        });

        if (reply->isFinished()) {
            emit reply->finished();
        }
    };

    // Función para encender/apagar el LED
    auto writeLed = [&](int serverId, bool on) {
        QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, 1, 1);
        writeUnit.setValue(0, on ? 1 : 0);

        QModbusReply *reply = client.sendWriteRequest(writeUnit, serverId);
        if (!reply) {
            qWarning() << "[Cliente] No se pudo enviar petición de escritura LED";
            return;
        }

        QObject::connect(reply, &QModbusReply::finished, [=]() {
            if (reply->error() == QModbusDevice::NoError) {
                qDebug() << "[Cliente] LED escrito correctamente a" << (on ? "ON" : "OFF");
            } else {
                qWarning() << "[Cliente] Error al escribir LED:" << reply->errorString();
            }
            reply->deleteLater();
        });
        if (reply->isFinished()) {
            emit reply->finished();
        }
    };

    QTimer::singleShot(2000, [&]() {
        readTemperature(1);
        writeLed(1, true);
    });

    QTimer::singleShot(5000, [&]() {
        readTemperature(1);
        writeLed(1, false);
    });

    qDebug() << "Cliente Modbus TCP iniciado. Conectado a 127.0.0.1:1502";

    return a.exec();
}
