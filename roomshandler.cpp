#include "roomshandler.h"

RoomsHandler::RoomsHandler(Database* _database)
{
    mDatabase = _database;
    mMutex = new std::mutex;
}

void RoomsHandler::initialInsert(const QString& roomId, const QString& streamId,
                                 const QString& displayName, const QByteArray& header, QTcpSocket* qTcpSocket)
{
    mMap[roomId][streamId] = new Participant(displayName, header, qTcpSocket);
    qDebug() << "Added streamId, displayName, header and QTcpSocket:" << streamId << displayName << "to the map after confirming with database";
}

void RoomsHandler::updateVideoHeader(const QString& roomId, const QString& streamId, const QByteArray& header)
{
    mMap.at(roomId).at(streamId)->setHeader(header);
}

void RoomsHandler::updateDisplayName(const QString& roomId, const QString& streamId, const QString& displayName)
{
    mMap.at(roomId).at(streamId)->setDisplayName(displayName);
}

void RoomsHandler::removeGuestFromUserTable(const QString& streamId)
{
    QSqlQuery q(mDatabase->getDb());
    bool isGuest = false;
    //DELETE FROM roomSession WHERE roomId = :roomId AND userId IN (SELECT id from user WHERE streamId = :streamId);
    q.prepare("SELECT isGuest FROM user WHERE streamId = :streamId");
    q.bindValue(":streamId", streamId);
    if (q.exec())
    {
        if (q.size() > 0 && q.next())
        {

            isGuest = q.value(0).toBool();
        }
        else
        {
            qDebug() << "Failed to find streamId: " << streamId << " " << q.lastQuery();
            return;
        }
    }
    else
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO << " error: " << q.lastError() << " query: " <<q.lastQuery() ;
        return;
    }
    if(isGuest)
    {
        q.prepare("DELETE FROM user WHERE streamId = :streamId");
        q.bindValue(":streamId", streamId);
        if (q.exec())
        {
            if(q.numRowsAffected() >= 1)
            {
                qDebug() << "Removed user: " << streamId;
                return;
            }
            else
            {
                qDebug() << "Number of rows deleted: " << q.numRowsAffected() << " " << Q_FUNC_INFO;
                return;
            }
        }
        else
        {
            qDebug() << "Failed Query" << Q_FUNC_INFO << " error: " << q.lastError() << " query: " <<q.lastQuery() ;
            return;
        }
    }
}

const std::map<QString, std::map<QString, Participant *> > RoomsHandler::getMap() const
{
    return mMap;
}

std::mutex *RoomsHandler::getMutex() const
{
    return mMutex;
}

bool RoomsHandler::removeParticipant(const QString& roomId, const QString& streamId)
{
    //qDebug() << mMap;
    /*if(!mMap[roomId][streamId])
    {
        return false;
        qDebug() << "roomId and streamId combo did not exist in map" << Q_FUNC_INFO;
    }*/
    mMap.at(roomId).erase(streamId);
    if(mMap.at(roomId).size() < 1)
    {
        mMap.erase(roomId);
        qDebug() << "roomId " << roomId << " was empty,  deleting";
    }

    QSqlQuery q(mDatabase->getDb());
    //DELETE FROM roomSession WHERE roomId = :roomId AND userId IN (SELECT id from user WHERE streamId = :streamId);
    q.prepare("DELETE FROM roomSession WHERE roomId = :roomId AND userId IN (SELECT id from user WHERE streamId = :streamId)");
    q.bindValue(":streamId", streamId);
    q.bindValue(":roomId", roomId);
    if (q.exec())
    {
        if(q.numRowsAffected() >= 1)
        {
            removeGuestFromUserTable(streamId);
            qDebug() << "Removed participant" << streamId << "from the roomSession" << roomId;
            qDebug() << "Map after erase: " << mMap;
            qDebug() << "adr: " << &mMap;
        }
        else
        {
            qDebug() << "Number of rows deleted " << q.numRowsAffected();
        }
    }
    else
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO << q.lastError();
        qDebug() << "streamId " << streamId << " and roomId: " << roomId;
        return false;
    }
    return true;
}

QSqlDatabase RoomsHandler::getDb() const
{
    return mDatabase->getDb();
}
