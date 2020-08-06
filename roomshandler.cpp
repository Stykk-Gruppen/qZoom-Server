#include "roomshandler.h"

RoomsHandler::RoomsHandler(Database* _database)
{
    mDatabase = _database;
    mMutex = new std::mutex;
}

void RoomsHandler::initialInsert(QString roomId, QString streamId, QString displayName, QByteArray header, QTcpSocket* qTcpSocket)
{
    mMap[roomId][streamId] = new Participant(displayName, header, qTcpSocket);
    qDebug() << "Added streamId, displayName, header and QTcpSocket:" << streamId << displayName << "to the map after confirming with database";
}

void RoomsHandler::updateVideoHeader(QString roomId, QString streamId, QByteArray header)
{
    mMap[roomId][streamId]->setHeader(header);
}

void RoomsHandler::updateDisplayName(QString roomId, QString streamId, QString displayName)
{
    mMap[roomId][streamId]->setDisplayName(displayName);
}

void RoomsHandler::removeGuestFromUserTable(QString streamId)
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

std::map<QString, std::map<QString, Participant *> > RoomsHandler::getMap() const
{
    return mMap;
}

std::mutex *RoomsHandler::getMutex() const
{
    return mMutex;
}

bool RoomsHandler::removeParticipant(QString roomId, QString streamId)
{
    //qDebug() << mMap;
    /*if(!mMap[roomId][streamId])
    {
        return false;
        qDebug() << "roomId and streamId combo did not exist in map" << Q_FUNC_INFO;
    }*/
    mMap[roomId].erase(streamId);
    if(mMap[roomId].size()<1)
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
