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
    void readPendingDatagrams(); //Må kanskje være void for connect enn så lenge
private:
    void sendDatagram(QByteArray arr, QHostAddress addr);
    //void sendParticipantRemovalNotice(QString roomId, QString streamId);
    void sendTcpPacket(QTcpSocket *socket, QByteArray arr);
    RoomsHandler* mRoomsHandler;
    QHostAddress mSenderAddress;
    QUdpSocket* mUdpSocket;
    int mPortNumber;
};

#endif // UDPSOCKETHANDLER_H
