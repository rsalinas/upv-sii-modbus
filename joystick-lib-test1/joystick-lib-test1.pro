QT = core
QT += serialbus

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../joystick-master-lib/release/ -ljoystick-master-lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../joystick-master-lib/debug/ -ljoystick-master-lib
else:unix: LIBS += -L$$OUT_PWD/../joystick-master-lib/ -ljoystick-master-lib

INCLUDEPATH += $$PWD/../joystick-master-lib
DEPENDPATH += $$PWD/../joystick-master-lib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../joystick-master-lib/release/libjoystick-master-lib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../joystick-master-lib/debug/libjoystick-master-lib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../joystick-master-lib/release/joystick-master-lib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../joystick-master-lib/debug/joystick-master-lib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../joystick-master-lib/libjoystick-master-lib.a
