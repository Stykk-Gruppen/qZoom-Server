#ifndef UDPSOCKETHANDLER_H
#define UDPSOCKETHANDLER_H

#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QProcess>
#include <mutex>
#include <QList>
#include <QtConcurrent/QtConcurrent>
#include <QMultiMap>
#include <QDateTime>

#include "roomshandler.h"

class UdpSocketHandler : public QObject
{
    Q_OBJECT
public:
    UdpSocketHandler(RoomsHandler* _roomsHandler, int portNumber, QObject *parent = nullptr);
    void initSocket();
private:
    void readPendingDatagrams(); //Må kanskje være void for connect enn så lenge
    void sendDatagram(QByteArray arr, QHostAddress addr);
    int mPortNumber;
    RoomsHandler* mRoomsHandler;
    QUdpSocket* mUdpSocket;
};

#endif // UDPSOCKETHANDLER_H
