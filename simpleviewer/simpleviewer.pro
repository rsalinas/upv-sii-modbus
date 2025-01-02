QT = core charts serialbus

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        timeseriesviewer.cpp

HEADERS += \
    mainwindow.h \
    timeseriesviewer.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

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

FORMS += \
    mainwindow.ui
