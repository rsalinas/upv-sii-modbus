QT = core network

CONFIG += c++17 cmdline

CONFIG(StudentDebug): DEFINES += STUDENT_VERSION
SOURCES += \
        main.cpp modbustcp-slave.cpp

HEADERS += modbustcp-slave.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
