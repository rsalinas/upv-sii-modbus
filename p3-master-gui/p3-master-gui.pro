QT += core gui widgets charts network
CONFIG += c++17
TEMPLATE = app
TARGET = p3-master-gui

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/modbuscontroller.cpp \
    src/modbustcp-master.cpp

HEADERS += \
    src/mainwindow.h \
    src/modbuscontroller.h \
    src/modbustcp-master.h

FORMS += \
    ui/mainwindow.ui
