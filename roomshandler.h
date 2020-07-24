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

class RoomsHandler : public Database
{
public:
    RoomsHandler();
    void startRemovalTimer(int seconds);
    void printMap();

protected:
    QMultiMap<QString, QMultiMap<QString, std::vector<QString>>> mRoomsMultiMap;
    std::map<QString, std::map<QString, std::vector<QString>>> mMap;
    void initialInsert(QString roomId, QString streamId, QString ipAddress, QString firstHeader);
    void updateTimestamp(QString roomId, QString streamId);
    void removeOldParticipantsFromQMap();

private:
    bool mAbortRemoval;
};

#endif // ROOMSHANDLER_H
