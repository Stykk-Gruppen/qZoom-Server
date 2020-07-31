#ifndef TCPSERVERHANDLER_H
#define TCPSERVERHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtConcurrent/QtConcurrent>

#include "roomshandler.h"

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
    enum mTcpHeaderValues { VIDEO_HEADER, DEAD_PARTICIPANT, NEW_DISPLAY_NAME };
    static int sendTcpPacket(QTcpSocket*, QByteArray arr);
    static void sendHeader(QTcpSocket* receiverSocket, QByteArray data, int headerValue);
    void sendParticipantRemovalNotice(QString roomId, QString streamId);
    void SendAndRecieveFromEveryParticipantInRoom(QByteArray header, QTcpSocket *readSocket);
    void sendUpdatedDisplayNameToEveryParticipantInRoom();
    RoomsHandler* mRoomsHandler;

    QTcpServer* mTcpServer;
    uint16_t mPort;

    QString mRoomId;
    QString mStreamId;
    QString mDisplayName;
};

#endif // TCPSERVERHANDLER_H
