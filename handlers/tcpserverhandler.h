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
    enum mTcpHeaderValues { VIDEO_HEADER, REMOVE_PARTICIPANT, NEW_DISPLAY_NAME, VIDEO_DISABLED, AUDIO_DISABLED, KICK_PARTICIPANT };
    static void sendHeader(QTcpSocket* receiverSocket, QByteArray data, const int& headerValue);
    void SendAndRecieveFromEveryParticipantInRoom(const QString& roomId, const QString& streamId, QByteArray header, QTcpSocket* readSocket);
    void sendHeaderToEveryParticipant(const QString& roomId, const QString& streamId, QByteArray header, const int& headerCode);
    void setupDisconnectAction(QTcpSocket* readSocket, const QString& roomId, const QString& streamId);
    RoomsHandler* mRoomsHandler;
    QTcpServer* mTcpServer;
    int mPortNumber;
    void printTcpPacketInfo(QHostAddress sender, QString streamId,QString roomId, QString displayName, QByteArray entirePacket);
};

#endif // TCPSERVERHANDLER_H
