QT = core serialbus

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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../meteo-client/release/ -lmeteo-client
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../meteo-client/debug/ -lmeteo-client
else:unix: LIBS += -L$$OUT_PWD/../meteo-client/ -lmeteo-client

INCLUDEPATH += $$PWD/../meteo-client
DEPENDPATH += $$PWD/../meteo-client

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../meteo-client/release/libmeteo-client.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../meteo-client/debug/libmeteo-client.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../meteo-client/release/meteo-client.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../meteo-client/debug/meteo-client.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../meteo-client/libmeteo-client.a
