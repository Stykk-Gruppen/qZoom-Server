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

class RoomsHandler : public Database
{
public:
    RoomsHandler();
    void startRemovalTimer(int seconds);
    void printMap();
    void initialInsert(QString roomId, QString streamId, QString ipAddress, QByteArray firstHeader);
    void updateTimestamp(QString roomId, QString streamId);
    void removeOldParticipantsFromQMap();

    std::map<QString, std::map<QString, std::vector<QByteArray>>> mMap;
    std::mutex* mMutex;

private:
    bool mAbortRemoval;

};

#endif // ROOMSHANDLER_H
