QT += core gui widgets network
CONFIG += c++17
TEMPLATE = app
TARGET = p3-slave-gui

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp ./src/modbustcp-slave.cpp

HEADERS += \
    src/mainwindow.h ./src/modbustcp-slave.h

FORMS += \
    ui/mainwindow.ui
