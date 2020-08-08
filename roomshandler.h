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
    void initialInsert(const QString& roomId, const QString& streamId, const QString& displayName,
                       const QByteArray& header, QTcpSocket* qTcpSocket);
    void updateVideoHeader(const QString& roomId, const QString& streamId, const QByteArray& header);
    bool removeParticipant(const QString& roomId, const QString& streamId);
    void updateDisplayName(const QString& roomId, const QString& streamId, const QString& displayName);
    QSqlDatabase getDb() const;
    std::mutex *getMutex() const;
    const std::map<QString, std::map<QString, Participant *> > getMap() const;

private:
    void removeGuestFromUserTable(const QString& streamId);
    std::map<QString, std::map<QString, Participant*>> mMap;
    std::mutex* mMutex;
    uint16_t mPort = 1337;
    Database* mDatabase;
};

#endif // ROOMSHANDLER_H
