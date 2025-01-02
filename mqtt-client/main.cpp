#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttTopicFilter>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("MQTTClient");

    QString hostname = "82.165.78.222";
    quint16 port = 1883;
    QString username = "upv-sii";
    QString password = "ETSIADI";

    QStringList args = app.arguments();
    if (args.size() > 1)
        hostname = args.value(1);
    if (args.size() > 2)
        username = args.value(2);
    if (args.size() > 3)
        password = args.value(3);

    QString topicName = QString("upv/sii/%1").arg(hostname);

    QMqttClient mqttClient;
    mqttClient.setHostname(hostname);
    mqttClient.setPort(port);
    mqttClient.setUsername(username);
    mqttClient.setPassword(password);

    QTimer timeoutTimer;
    timeoutTimer.setInterval(5000);
    timeoutTimer.setSingleShot(true);

    QObject::connect(&mqttClient, &QMqttClient::connected, [&]() {
        qDebug() << "Connected to broker";
        auto subscription = mqttClient.subscribe(QMqttTopicFilter(topicName));
        if (!subscription) {
            qDebug() << "Failed to subscribe to topic";
            return;
        }
        qDebug() << "Subscribed to topic";
        mqttClient.publish(QMqttTopicName(topicName), "Hello, MQTT from Qt!");
        timeoutTimer.start();
    });

    QObject::connect(&mqttClient,
                     &QMqttClient::messageReceived,
                     [&](const QByteArray &message, const QMqttTopicName &topic) {
                         qDebug() << "Message received from topic" << topic.name() << ":"
                                  << message;
                         timeoutTimer.stop();
                     });

    QObject::connect(&mqttClient, &QMqttClient::disconnected, []() {
        qDebug() << "Disconnected from broker";
    });

    QObject::connect(&timeoutTimer, &QTimer::timeout, [&]() {
        qDebug() << "Error: Message not received within the timeout period";
        QCoreApplication::exit(1);
    });

    mqttClient.connectToHost();

    return app.exec();
}
