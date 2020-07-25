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
    UdpSocketHandler(RoomsHandler* _roomsHandler, QObject *parent = nullptr);
    void initSocket();
    void readPendingDatagrams(); //Må kanskje være void for connect enn så lenge
private:
    int sendDatagram(QByteArray);
    RoomsHandler* mRoomsHandler;
    QHostAddress mSenderAddress;
    QUdpSocket* mUdpSocket;
    uint16_t mPort;
};

#endif // UDPSOCKETHANDLER_H
