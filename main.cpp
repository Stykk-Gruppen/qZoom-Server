#include <QCoreApplication>
#include "handlers/udpsockethandler.h"
#include "handlers/tcpserverhandler.h"
#include "handlers/sqltcpserverhandler.h"
#include "handlers/roomshandler.h"
#include <iostream>

/*! \mainpage qZoom-Server Documentation
 *
 * \section intro_sec Introduction
 *
 * This document describes all the C++ classes used in our qZoom Server
 *
 */

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("qZoom-Server");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Hello! This is the current description");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption udpPortNumber(QStringList() << "u" << "udp", QCoreApplication::translate("main", "Sets the port number for the UDP socket"),
                                                   QCoreApplication::translate("main", "port number"));
    QCommandLineOption tcpPortNumber(QStringList() << "t" << "tcp", QCoreApplication::translate("main", "Sets the port number for the TCP socket"),
                                                   QCoreApplication::translate("main", "port number"));
    QCommandLineOption sqlPortNumber(QStringList() << "s" << "sql", QCoreApplication::translate("main", "Sets the port number for the SQL-TCP socket"),
                                                   QCoreApplication::translate("main", "port number"));

    parser.addOption(udpPortNumber);
    parser.addOption(tcpPortNumber);
    parser.addOption(sqlPortNumber);
    parser.process(a);

    qDebug() << "qZoom-Server running Qt Version: " << QT_VERSION_STR;
    int portNumberTCP = parser.isSet(tcpPortNumber) ? parser.value(tcpPortNumber).toInt() : 1338;
    if(portNumberTCP < 1024 || portNumberTCP > 49151)
    {
        printf("TCP port number was not valid, needs to be larger than 1024 and smaller than 49151");
        exit(-1);
    }

    int portNumberUDP = parser.isSet(udpPortNumber) ? parser.value(udpPortNumber).toInt() : 1337;
    if(portNumberUDP < 1024 || portNumberUDP > 49151)
    {
        printf("UDP port number was not valid, needs to be larger than 1024 and smaller than 49151");
        exit(-1);
    }

    int portNumberSQL = parser.isSet(sqlPortNumber) ? parser.value(sqlPortNumber).toInt() : 1339;
    if(portNumberUDP < 1024 || portNumberUDP > 49151)
    {
        printf("SQL-TCP port number was not valid, needs to be larger than 1024 and smaller than 49151");
        exit(-1);
    }

    Database* db = new Database();
    RoomsHandler* roomsHandler = new RoomsHandler(db);
    new UdpSocketHandler(roomsHandler, portNumberUDP);
    new TcpServerHandler(roomsHandler, portNumberTCP);
    new SqlTcpServerHandler(portNumberSQL, db);

    return a.exec();
}
