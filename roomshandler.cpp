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

void RoomsHandler::updateHeader(QString roomId, QString streamId, QByteArray header)
{
    mMap[roomId][streamId]->setHeader(header);
}

void RoomsHandler::updateDisplayName(QString roomId, QString streamId, QString displayName)
{
    mMap[roomId][streamId]->setDisplayName(displayName);
}

void RoomsHandler::removeParticipant(QString roomId, QString streamId)
{
    mMap[roomId].erase(streamId);
 // q.prepare("SELECT r.id, r.password, u.username FROM room AS r, user AS u WHERE r.host = u.id AND r.id = :roomId AND r.password = :roomPassword");
    QSqlQuery q(Database::mDb);
    q.prepare("DELETE FROM roomSession, user WHERE roomSession.userId = user.Id AND user.streamId = :streamId AND roomId = :roomId");
    q.bindValue(":streamId", streamId);
    q.bindValue(":roomId", roomId);
    if (q.exec())
    {
        qDebug() << "Removed participant" << streamId << "from the database";
    }
    else
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO << q.lastError();
        qDebug() << "streamId " << streamId << " and roomId: " << roomId;
    }
}

