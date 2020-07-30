#ifndef TCPSERVERHANDLER_H
#define TCPSERVERHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtConcurrent/QtConcurrent>

#include "roomshandler.h"
#include "tcpsockethandler.h"

class TcpServerHandler : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerHandler(RoomsHandler* _roomsHandler, QObject *parent = nullptr);
    void initTcpServer();
    void acceptTcpConnection();
    void readTcpPacket();

private:
    QByteArray returnCodesArray;
    //Should match enum in Client::TcpSocketHandler
    enum mTcpReturnValues { STREAM_ID_NOT_FOUND, ROOM_ID_NOT_FOUND, SESSION_STARTED };
    static int sendTcpPacket(QTcpSocket*, QByteArray arr);
    static void sendHeader(QTcpSocket* receiverSocket, QByteArray data);
    RoomsHandler* mRoomsHandler;
    QHostAddress mSenderAddress;
    QTcpSocket *mTcpServerConnection = nullptr;
    QTcpServer* mTcpServer;
    uint16_t mPort;
};

#endif // TCPSERVERHANDLER_H
