#include <QCoreApplication>
#include <QDebug>
#include <QSerialPort>
#include <QTimer>
#include <QVariant> // Define QVariant::fromValue
#include <QtSerialBus/QModbusDataUnit>
#include <QtSerialBus/QModbusRtuSerialClient>
#include <QtSerialPort/QSerialPort> // Define QSerialPort y sus enums

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QModbusRtuSerialClient client;

    auto port = "/tmp/modbus-master";
    client.setConnectionParameter(QModbusDevice::SerialPortNameParameter, port);
    client.setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud9600);
    client.setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
    client.setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
    client.setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);

    if (!client.connectDevice()) {
        qFatal() << "No se pudo conectar el maestro RTU en " << port;
        return -1;
    }

    // -------------------------------------------------------
    // Función para leer la temperatura (HoldingRegister en dirección 0)
    auto readTemperature = [&](int slaveId) {
        // Queremos leer 1 registro desde la dirección 0 (HoldingRegisters)
        QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0, 1);
        QModbusReply *reply = client.sendReadRequest(readUnit, slaveId);
        if (!reply) {
            qWarning() << "[Maestro] No se pudo enviar petición de lectura";
            return;
        }

        if (!reply->isFinished()) {
            QObject::connect(reply, &QModbusReply::finished, [=]() {
                if (reply->error() == QModbusDevice::NoError) {
                    auto result = reply->result();
                    auto tempVal = result.value(0);
                    qDebug() << "[Maestro] Temperatura leída:" << tempVal << "°C";
                } else {
                    qWarning() << "[Maestro] Error en lectura:" << reply->errorString();
                }
                reply->deleteLater();
            });
        } else {
            reply->deleteLater();
        }
    };

    // -------------------------------------------------------
    // Función para encender/apagar el LED (HoldingRegister dirección 1)
    auto writeLed = [&](int slaveId, bool on) {
        // Dirección 1, tamaño 1, valor: 1 = encendido, 0 = apagado
        QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, 1, 1);
        writeUnit.setValue(0, on ? 1 : 0);

        QModbusReply *reply = client.sendWriteRequest(writeUnit, slaveId);
        if (!reply) {
            qWarning() << "[Maestro] No se pudo enviar petición de escritura LED";
            return;
        }

        // Cuando termine la operación...
        QObject::connect(reply, &QModbusReply::finished, [=]() {
            if (reply->error() == QModbusDevice::NoError) {
                qDebug() << "[Maestro] LED escrito correctamente:" << (on ? "ON" : "OFF");
            } else {
                qWarning() << "[Maestro] Error al escribir LED:" << reply->errorString();
            }
            reply->deleteLater();
        });
    };

    // -------------------------------------------------------
    // Ejemplo: leer temperatura y encender LED, luego de 5s leer de nuevo y apagar LED
    QTimer::singleShot(2000, [&]() {
        readTemperature(1); // Leer temp del esclavo ID=1
        writeLed(1, true);  // Encender LED
    });

    QTimer::singleShot(5000, [&]() {
        readTemperature(1); // Volver a leer temp
        writeLed(1, false); // Apagar LED
    });

    qDebug() << "Maestro Modbus RTU conectado. Puerto: " << port;

    return a.exec();
}
