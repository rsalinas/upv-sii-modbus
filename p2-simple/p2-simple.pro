QT       += core gui charts serialbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

message("Current PWD is: $$PWD")

SOURCES += \
    main.cpp

# Configuración de la librería weather-modbustcp-master-lib
# Se asume que en Windows hay carpetas separadas para release y debug, mientras que en Unix está en una única carpeta

win32:CONFIG(release, debug|release) {
    LIBS += -L$$PWD/3rdparty/weather-modbustcp-master-lib/release/ -lweather-modbustcp-master-lib
} else: win32:CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/3rdparty/weather-modbustcp-master-lib/debug/ -lweather-modbustcp-master-lib
}

message("LIBDIR: $$PWD/3rdparty/release/weather-modbustcp-master-lib")

unix: LIBS += -L$$PWD/3rdparty/weather-modbustcp-master-lib/debug/ -lweather-modbustcp-master-lib

# Rutas de inclusión y dependencias
INCLUDEPATH += $$PWD/3rdparty/weather-modbustcp-master-lib/include
DEPENDPATH += $$PWD/3rdparty/weather-modbustcp-master-lib/include

message("INCLUDEPATH+: $$PWD/3rdparty/weather-modbustcp-master-lib/include")

# Reglas por defecto para despliegue.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
