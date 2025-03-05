QT       += core gui charts serialbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

message("Current PWD is: $$PWD")

SOURCES += \
    main.cpp

# Rutas de inclusión y dependencias
INCLUDEPATH += $$PWD/3rdparty/weather-modbustcp-master-lib/include
DEPENDPATH += $$PWD/3rdparty/weather-modbustcp-master-lib/include

message("INCLUDEPATH+: $$PWD/3rdparty/weather-modbustcp-master-lib/include")

# Reglas por defecto para despliegue.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix|win32: LIBS += -L$$PWD/3rdparty/weather-modbustcp-master-lib/ -lmeteo-client

INCLUDEPATH += $$PWD/3rdparty/weather-modbustcp-master-lib
DEPENDPATH += $$PWD/3rdparty/weather-modbustcp-master-lib
