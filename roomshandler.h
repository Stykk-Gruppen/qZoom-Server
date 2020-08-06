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
    QSqlDatabase getDb() const;
    std::mutex *getMutex() const;
    std::map<QString, std::map<QString, Participant *> > getMap() const;

private:
    void removeGuestFromUserTable(QString streamId);
    std::map<QString, std::map<QString, Participant*>> mMap;
    std::mutex* mMutex;
    uint16_t mPort = 1337;
    Database* mDatabase;
};

#endif // ROOMSHANDLER_H
