#ifndef TCPSERVERHANDLER_H
#define TCPSERVERHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtConcurrent/QtConcurrent>

#include "roomshandler.h"
#include "tcpsockethandler.h"

class TcpServerHandler : public QObject, public RoomsHandler
{
    Q_OBJECT
public:
    explicit TcpServerHandler(QObject *parent = nullptr);
    void initTcpServer();
    void acceptTcpConnection();
    void readTcpPacket();

private:
    int sendTcpPacket(QByteArray arr);
    static void sendHeader(QHostAddress receiverAddress, QByteArray data, uint16_t port);

    QHostAddress mSenderAddress;
    QTcpSocket *mTcpServerConnection = nullptr;
    QTcpServer* mTcpServer;
    uint16_t mPort;
};

#endif // TCPSERVERHANDLER_H
