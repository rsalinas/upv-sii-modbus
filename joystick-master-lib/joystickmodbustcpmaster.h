#ifndef JOYSTICKMODBUSTCPMASTER_H
#define JOYSTICKMODBUSTCPMASTER_H

#include <QObject>
#include <QVector>
#include <QtSerialBus/QModbusReply>
#include <QtSerialBus/QModbusTcpClient>

class JoystickModbusTcpMaster : public QObject
{
    Q_OBJECT
public:
    explicit JoystickModbusTcpMaster(QObject *parent = nullptr);
    ~JoystickModbusTcpMaster();

    /// Conecta con el servidor Modbus TCP en host:port.
    bool connectToServer(const QString &host, int port);

    /// Desconecta del servidor.
    void disconnectFromServer();

    /// Escribe los estados de 3 LEDs binarios (coils). Se espera un QVector<bool> de tamaño >= 3.
    QModbusReply *writeBinaryLeds(const QVector<bool> &ledStates);

    /// Lee el estado de 2 botones (discrete inputs).
    QModbusReply *readButtons();

    /// Lee la posición del joystick (input registers, 2 registros).
    QModbusReply *readJoystick();

    /// Escribe 3 valores para los LEDs PWM (holding registers).
    QModbusReply *writePwmLeds(const QVector<quint16> &pwmValues);

    /// Acceso directo al dispositivo Modbus (opcional).
    QModbusTcpClient *modbusDevice() const;

private:
    QModbusTcpClient *m_modbusDevice;
};

#endif // JOYSTICKMODBUSTCPMASTER_H
