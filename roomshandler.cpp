#include "roomshandler.h"

RoomsHandler::RoomsHandler(Database* _database)
{
    mDatabase = _database;
    mMutex = new std::mutex;
}

/**
 * Populates the map with key and value, gets run if someone is joining a room
 * and exists in the database, but not in the map.
 * @param roomId QString containing the key for the room
 * @param streamId QString containing the key for the participant
 * @param displayName QString with name of participant
 * @param header QByteArray the participant video header data
 * @param qTcpSocket QTcpSocket pointer, the particpant socket, to preserve their connection
 */
void RoomsHandler::initialInsert(const QString& roomId, const QString& streamId,
                                 const QString& displayName, const QByteArray& header, QTcpSocket* qTcpSocket)
{
    mMap[roomId][streamId] = new Participant(displayName, header, qTcpSocket);
    qDebug() << "Added streamId, displayName, header and QTcpSocket:" << streamId << displayName << "to the map after confirming with database";
}

/**
 * Updates the header QByteArray in the map at using the keys
 * roomId and streamId
 * @param roomId QString
 * @param streamId QString
 * @param header QByteArray
 */
void RoomsHandler::updateVideoHeader(const QString& roomId, const QString& streamId, const QByteArray& header)
{
    mMap.at(roomId).at(streamId)->setHeader(header);
}

/**
 * Updates the displayName QString in the map at using the keys
 * roomId and streamId
 * @param roomId QString
 * @param streamId QString
 * @param displayName QString
 */
void RoomsHandler::updateDisplayName(const QString& roomId, const QString& streamId, const QString& displayName)
{
    mMap.at(roomId).at(streamId)->setDisplayName(displayName);
}

/**
 * Attempts to remove the participant with the streamId from the database.
 * Will only be removed if the participant is a guest user.
 * @param streamId
 */
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

/**
 * @brief RoomsHandler::getMap
 * @return std::map<QString, std::map<QString, Participant *>>
 */
const std::map<QString, std::map<QString, Participant *> > RoomsHandler::getMap() const
{
    return mMap;
}
/**
 * @brief RoomsHandler::getMutex
 * @return  std::mutex*
 */
std::mutex *RoomsHandler::getMutex() const
{
    return mMutex;
}
/**
 * Attempts to remove the key roomId from the map
 * Will check if the room is empty before deleting
 * @param roomId QString
 */
void RoomsHandler::removeEmptyRoom(const QString& roomId)
{
    if(mMap.at(roomId).size() < 1)
    {
        mMap.erase(roomId);
        qDebug() << "roomId " << roomId << " was empty,  deleting";
    }
}

/**
 * Remove a participant from a room, will also attempt to delete the room, if no
 * participants is left inside the room. Will also attempt to remove the particpant
 * from the roomSession in the database. However the function will return true even if no
 * particpant was found in said room in the database.
 * @param roomId QString
 * @param streamId QString
 * @return true if query was executed, regardless of numRowsAffected()
 */
bool RoomsHandler::removeParticipant(const QString& roomId, const QString& streamId)
{

    mMap.at(roomId).erase(streamId);
    removeEmptyRoom(roomId);

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
            //TODO if this number is 0, log data for debugging
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
/**
 * @brief RoomsHandler::getDb
 * @return QSqlDatabase
 */
QSqlDatabase RoomsHandler::getDb() const
{
    return mDatabase->getDb();
}
