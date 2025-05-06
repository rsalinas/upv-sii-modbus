QT += core gui charts serialbus
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

CONFIG += c++17

#TARGET = weather-modbustcp-master
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# INCLUDEPATH += ../weather-modbustcp-master-lib
# LIBS += -L../build-weather-modbustcp-master-lib-*/ -lweather-modbustcp-master-lib

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../weather-modbustcp-master-lib/release/ -lweather-modbustcp-master-lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../weather-modbustcp-master-lib/debug/ -lweather-modbustcp-master-lib
else:unix: LIBS += -L$$OUT_PWD/../weather-modbustcp-master-lib/ -lweather-modbustcp-master-lib

INCLUDEPATH += $$PWD/../weather-modbustcp-master-lib
DEPENDPATH += $$PWD/../weather-modbustcp-master-lib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../weather-modbustcp-master-lib/release/libweather-modbustcp-master-lib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../weather-modbustcp-master-lib/debug/libweather-modbustcp-master-lib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../weather-modbustcp-master-lib/release/weather-modbustcp-master-lib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../weather-modbustcp-master-lib/debug/weather-modbustcp-master-lib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../weather-modbustcp-master-lib/libweather-modbustcp-master-lib.a
