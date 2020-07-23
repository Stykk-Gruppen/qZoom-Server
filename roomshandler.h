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
    void printQMap();

public slots:
    //void removeOldParticipantsFromQMap();

protected:
    QMultiMap<QString, QMultiMap<QString, std::vector<QString>>> mRoomsMultiMap;
    std::map<QString, std::map<QString, std::vector<QString>>> mMap;
    void initialInsert(QString roomId, QString streamId, QString ipAddress, QString firstHeader);
    void updateTimestamp(QString roomId, QString streamId);
    void removeOldParticipantsFromQMap();

private:
    QTimer* mTimer;
};

#endif // ROOMSHANDLER_H
