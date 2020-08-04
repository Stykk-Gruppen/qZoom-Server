#include "roomshandler.h"

RoomsHandler::RoomsHandler()
{
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
    }

    QSqlQuery q(Database::mDb);
    //DELETE FROM roomSession WHERE roomId = :roomId AND userId IN (SELECT id from user WHERE streamId = :streamId);
    q.prepare("DELETE FROM roomSession WHERE roomId = :roomId AND userId IN (SELECT id from user WHERE streamId = :streamId)");
    q.bindValue(":streamId", streamId);
    q.bindValue(":roomId", roomId);
    if (q.exec())
    {
        if(q.numRowsAffected() >= 1)
        {
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

