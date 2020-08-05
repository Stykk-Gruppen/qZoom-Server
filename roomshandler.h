#ifndef ROOMSHANDLER_H
#define ROOMSHANDLER_H

#include <QObject>
#include "database.h"
#include <QNetworkDatagram>
#include <QProcess>
#include <mutex>
#include <QList>
#include <QtConcurrent/QtConcurrent>
#include <QMultiMap>
#include <QDateTime>
#include "database.h"
#include "config.cpp"
#include <QMutex>
#include <QTcpSocket>
#include "participant.h"

class RoomsHandler
{
public:
    RoomsHandler(Database* _database);
    void startRemovalTimer(int seconds);
    void initialInsert(QString roomId, QString streamId, QString ipAddress,
                       QByteArray header, QTcpSocket* qTcpSocket);
    void updateVideoHeader(QString roomId, QString streamId, QByteArray header);
    bool removeParticipant(QString roomId, QString streamId);
    void updateDisplayName(QString roomId, QString streamId, QString displayName);
    std::map<QString, std::map<QString, Participant*>> mMap;
    std::mutex* mMutex;
    Database* mDatabase;

private:
    uint16_t mPort = 1337;
    void removeGuestFromUserTable(QString streamId);
};

#endif // ROOMSHANDLER_H
