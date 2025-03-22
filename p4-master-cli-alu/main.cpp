#include <QCoreApplication>
#include <QTimer>

// Se incluye la cabecera de la clase ModbusTcpClient.
#include "modbustcp-master.h"

// Función principal del programa.
int main(int argc, char *argv[])
{
    // Se crea la aplicación de consola.
    QCoreApplication app(argc, argv);

    // Variable para determinar si se ejecutará una única prueba (modo oneShot).
    bool oneShot = false;
    if (argc > 1 && QString(argv[1]) == "-1") {
        oneShot = true;
    }

    // Se crea una instancia del cliente Modbus TCP.
    ModbusTcpClient modbus;

    // Se intenta conectar con el servidor Modbus en la dirección 127.0.0.1 y puerto 1502.
    if (!modbus.connectToHost("127.0.0.1", 1502)) {
        qCritical() << "Fallo al conectar con el servidor Modbus.";
        return 1;
    }

    // Variable estática para alternar el valor de escritura en las bobinas.
    static bool toggle = false;

    // Se crea un temporizador para ejecutar pruebas de forma periódica.
    QTimer timer;

    // Se define una función lambda que contiene el conjunto de pruebas (testSuite).
    auto testSuite = [&]() {
        // PRUEBA 1: Lectura de entradas discretas.
        {
            QVector<bool> values;
            // Se leen 2 entradas discretas a partir de la dirección 0.
            if (!modbus.readDiscreteInputs(0, 2, values)) {
                qFatal() << "Error al leer entradas discretas";
            }
            qDebug() << "Entradas discretas: " << values;
        }

        // PRUEBA 2: Lectura de registros (por ejemplo, presión y temperatura).
        {
            QVector<quint16> registers;
            // Se leen 2 registros a partir de la dirección 0.
            if (modbus.readRegisters(0, 2, registers)) {
                qDebug() << "Presión:" << registers.at(0)
                         << " mbar, Temperatura:" << registers.at(1) << " ºC";
            } else {
                qDebug() << "Error al leer registros.";
            }
        }

        // PRUEBA 3: Escritura en bobinas (coils).
        {
            // Se escribe en tres bobinas usando el valor de 'toggle'.
            if (!modbus.writeCoils(0, QVector<bool>() << toggle << toggle << toggle)) {
                qDebug() << "Error al escribir en las bobinas";
            }
            // Se alterna el valor para la siguiente escritura.
            toggle = !toggle;
        }

        // Si se ejecuta en modo oneShot, se finaliza la aplicación tras la prueba.
        if (oneShot) {
            QCoreApplication::quit();
        }
    };

    // Configuración para ejecutar testSuite:
    if (oneShot) {
        // Ejecuta testSuite una única vez inmediatamente.
        QTimer::singleShot(0, testSuite);
    } else {
        // Conecta el temporizador para ejecutar testSuite cada 1000 ms (1 segundo).
        QObject::connect(&timer, &QTimer::timeout, testSuite);
        timer.start(1000);
    }

    // Se inicia el loop principal de la aplicación.
    return app.exec();
}
