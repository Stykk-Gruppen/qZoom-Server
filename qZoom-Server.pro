QT -= gui
QT += network sql

CONFIG += c++17 console
CONFIG -= app_bundle
QTPLUGIN += qsqlmysql
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS += -L/usr/lib -I/usr/include

SOURCES += \
        core/database.cpp \
        core/participant.cpp \
        handlers/roomshandler.cpp \
        handlers/sqltcpserverhandler.cpp \
        handlers/tcpserverhandler.cpp \
        handlers/udpsockethandler.cpp \
        main.cpp

configuration.path = /usr/local/qZoom-Server/config/
configuration.files = docs/*

unix:configuration.extra = rm -f /usr/local/qZoom-Server/config/qZoom-Server.conf && echo "hostname = " >> /usr/local/qZoom-Server/config/qZoom-Server.conf; \
echo "databasename = " >> /usr/local/qZoom-Server/config/qZoom-Server.conf; \
echo "username = " >> /usr/local/qZoom-Server/config/qZoom-Server.conf; \
echo "password = " >> /usr/local/qZoom-Server/config/qZoom-Server.conf

# Default rules for deployment.
target.path = /usr/local/bin/
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target configuration

HEADERS += \
    core/database.h \
    core/participant.h \
    handlers/roomshandler.h \
    handlers/sqltcpserverhandler.h \
    handlers/tcpserverhandler.h \
    handlers/udpsockethandler.h
