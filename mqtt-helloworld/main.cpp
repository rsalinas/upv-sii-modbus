#include <QCoreApplication>
#include <QtMqtt/QMqttClient>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QMqttClient client;
    client.setHostname("siimqtt.mooo.com");
    client.setPort(1883);
    client.setUsername("groupXX"); // FIXME: Completar nombre del grupo
    client.setPassword("");        // FIXME: Poner la contraseña enviada por Poliformat

    QObject::connect(&client, &QMqttClient::connected, [&]() {
        qInfo() << "Conectado al broker MQTT\n";
        client.subscribe(QMqttTopicFilter(client.username() + "/mytopic"));

        QString mensaje = "Hello world desde Qt";
        client.publish(QMqttTopicName(client.username() + "/mytopic"), mensaje.toUtf8());
        qInfo() << "Mensaje publicado: " << mensaje;
    });

    QObject::connect(&client,
                     &QMqttClient::messageReceived,
                     [&](const QByteArray &message, const QMqttTopicName &topic) {
                         qInfo() << "Mensaje recibido en " << topic.name() << ": " << message;
                     });

    // Esto inicia la conexión, pero vuelve inmediatamente.
    // El código que se suscribe y envía mensajes va en el manejador de "QMqttClient::connected".
    client.connectToHost();
    return a.exec();
}
