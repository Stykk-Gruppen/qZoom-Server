#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>

#include <QNetworkDatagram>
#include <QProcess>
#include <mutex>
#include <QList>
#include <QtConcurrent/QtConcurrent>
#include <QMultiMap>
#include <QDateTime>
#include "database.h"
#include "config.cpp"

class SocketHandler : public QObject, public Database
{
    Q_OBJECT
public:
    explicit SocketHandler(QObject *parent = nullptr);
    QUdpSocket* mUdpSocket;
    int sendDatagram(QByteArray, QString);
    void initUdpSocket();
    void startRemovalTimer(int seconds);
public slots:
    void removeOldParticipantsFromQMap();
    void readPendingDatagrams(); //Må kanskje være void for connect enn så lenge
private:
    uint16_t mPort;
    QMultiMap<char*, char*> mRoomsMap;
    int mStreamIdLength;
    int mRoomIdLength;
    QTimer* mTimer;
};

#endif // SOCKETHANDLER_H
