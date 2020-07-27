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
    QCommandLineOption removeOldParticipantsOption("p", QCoreApplication::translate("main", "Sets the interval of removal of inactive participants to <seconds>. The default value is 10 minutes."),
                                                   QCoreApplication::translate("main", "seconds"));
    parser.addOption(removeOldParticipantsOption);
    parser.process(a);

    qDebug() << "qZoom-Server running Qt Version: " << QT_VERSION_STR;

    RoomsHandler* roomsHandler = new RoomsHandler();
    UdpSocketHandler* udpSocket = new UdpSocketHandler(roomsHandler);
    TcpServerHandler* tcpServer = new TcpServerHandler(roomsHandler);
    //tcpServer->initTcpServer();
    int oldParticipantsRemovalIntervalValue = parser.isSet(removeOldParticipantsOption) ? parser.value(removeOldParticipantsOption).toInt() : 600;
    roomsHandler->startRemovalTimer(oldParticipantsRemovalIntervalValue);

   // roomsHandler->printMap();

    return a.exec();
}
