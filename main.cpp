#include <QCoreApplication>
#include "udpsockethandler.h"
#include "tcpserverhandler.h"
#include "roomshandler.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("qZoom-Server");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Hello! This is the current description");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption udpPortNumber("udp", QCoreApplication::translate("main", "Sets the port number for the UDP socket"),
                                                   QCoreApplication::translate("main", "port number"));
    QCommandLineOption tcpPortNumber("tcp", QCoreApplication::translate("main", "Sets the port number for the TCP socket"),
                                                   QCoreApplication::translate("main", "port number"));
    /*QCommandLineOption removeOldParticipantsOption("p", QCoreApplication::translate("main", "Sets the interval of removal of inactive participants to <seconds>. The default value is 10 minutes."),
                                                   QCoreApplication::translate("main", "seconds"));*/
    //parser.addOption(removeOldParticipantsOption);
    parser.addOption(udpPortNumber);
    parser.addOption(tcpPortNumber);
    parser.process(a);

    qDebug() << "qZoom-Server running Qt Version: " << QT_VERSION_STR;
    int portNumberTCP = parser.isSet(udpPortNumber) ? parser.value(udpPortNumber).toInt() : 1338;
    if(portNumberTCP<1024 && portNumberTCP>49151)
    {
        printf("TCP port number was not valid, needs to be larger than 1024 and smaller than 49151");
        exit(-1);
    }
    int portNumberUDP = parser.isSet(udpPortNumber) ? parser.value(udpPortNumber).toInt() : 1337;
    if(portNumberUDP<1024 && portNumberUDP>49151)
    {
        printf("UDP port number was not valid, needs to be larger than 1024 and smaller than 49151");
        exit(-1);
    }

    RoomsHandler* roomsHandler = new RoomsHandler();
    UdpSocketHandler* udpSocket = new UdpSocketHandler(roomsHandler,portNumberUDP);
    TcpServerHandler* tcpServer = new TcpServerHandler(roomsHandler,portNumberTCP);
    //tcpServer->initTcpServer();
    //int oldParticipantsRemovalIntervalValue = parser.isSet(removeOldParticipantsOption) ? parser.value(removeOldParticipantsOption).toInt() : 600;
    //roomsHandler->startRemovalTimer(oldParticipantsRemovalIntervalValue);

   // roomsHandler->printMap();

    return a.exec();
}
