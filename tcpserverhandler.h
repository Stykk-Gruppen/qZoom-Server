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
    explicit TcpServerHandler(RoomsHandler* _roomsHandler, int portNumber, QObject *parent = nullptr);
    void initTcpServer();
    void acceptTcpConnection();
    void readTcpPacket();

private:
   // QByteArray returnCodesArray;
    //Should match enum in Client::TcpSocketHandler
    //enum mTcpReturnValues { STREAM_ID_NOT_FOUND, ROOM_ID_NOT_FOUND, SESSION_STARTED };
    enum mTcpHeaderValues { VIDEO_HEADER, REMOVE_PARTICIPANT, NEW_DISPLAY_NAME, VIDEO_DISABLED, AUDIO_DISABLED };
    //static int sendTcpPacket(QTcpSocket*, QByteArray arr);
    static void sendHeader(QTcpSocket* receiverSocket, QByteArray data, int headerValue);
    void SendAndRecieveFromEveryParticipantInRoom(QString roomId, QString streamId, QByteArray header, QTcpSocket *readSocket);
    void sendHeaderToEveryParticipant(QString roomId, QString streamId, QByteArray header, int headerCode);

    RoomsHandler* mRoomsHandler;
    QTcpServer* mTcpServer;
    int mPortNumber;
};

#endif // TCPSERVERHANDLER_H
