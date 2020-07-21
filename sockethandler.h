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

class SocketHandler : public QObject, public Database
{
    Q_OBJECT
public:
    explicit SocketHandler(QObject *parent = nullptr);
    QUdpSocket* mUdpSocket;
    void readPendingDatagrams(); //Må kanskje være void for connect enn så lenge
    int sendDatagram(QByteArray, QString);
    void initSocket();
    void startRemovalTimer(int seconds);

public slots:
    void removeOldParticipantsFromQMap();
    void printQMap();

private:
    uint16_t mPort;
    QMultiMap<char*, char*> mRoomsMap;
    QMultiMap<char*, QMultiMap<char*, std::vector<QString>>> mRoomsMultiMap;
    QMap <char*, char*> test;
    int mStreamIdLength;
    int mRoomIdLength;
    QTimer* mTimer;

signals:

};

#endif // SOCKETHANDLER_H
