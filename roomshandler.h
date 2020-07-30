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
#include "tcpsockethandler.h"
#include <QTcpSocket>

class RoomsHandler : public Database
{
public:
    RoomsHandler();
    void startRemovalTimer(int seconds);
    void printMap();
    void initialInsert(QString roomId, QString streamId, QString ipAddress, QByteArray header, QTcpSocket* qTcpSocket);
    void updateTimestamp(QString roomId, QString streamId);
    void updateHeader(QString roomId, QString streamId, QByteArray header);
    void removeOldParticipantsFromQMap();
    void removeParticipant(QString roomId, QString streamId);

    std::map<QString, std::map<QString, std::vector<QVariant>>> mMap;
    std::mutex* mMutex;

private:
    bool mAbortRemoval;
    uint16_t mPort = 1337;

};

#endif // ROOMSHANDLER_H
