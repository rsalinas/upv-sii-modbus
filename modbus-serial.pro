TEMPLATE = subdirs
CONFIG(StudentDebug): DEFINES += STUDENT_VERSION
SUBDIRS += \
    joystick-master-lib \
    joystick-modbustcp-slave \
    learning-graphics1 \
    mqtt-helloworld \
    p3-master-cli \
    modbus-rawserial-master \
    modbus-rawserial-slave \
    modbus-rawtcp-slave \
    modbus-serial-master \
    modbus-serial-slave \
    modbus-tcp-master \
    modbus-tcp-slave \
    mqtt-client \
    simpleviewer \
    weather-modbustcp-master \
    weather-modbustcp-master-lib \
    meteo-client \
    weather-modbustcp-slave \
    p3-master-cli-alu \
    p3-slave-cli \
    p3-slave-cli-alu \
    p3-master-gui \
    p3-slave-gui


#    p2-simple \     meteoclient-cli joystick-lib-test1 \ p2-ejemplo \
