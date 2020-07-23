#include "roomshandler.h"

RoomsHandler::RoomsHandler()
{
    //connect(mTimer, SIGNAL(timeout()), this, SLOT(removeOldParticipantsFromQMap()));
    //QObject::connect(mTimer, &QTimer::timeout, removeOldParticipantsFromQMap);
}

void RoomsHandler::removeOldParticipantsFromQMap()
{
    int qMapParticipantsCounter = 0;
    int qMapRoomsCounter = 0;
    int databaseCounter = 0;
    std::map<QString, std::map<QString, std::vector<QString>>>::iterator i;
    for (i = mMap.begin(); i != mMap.end(); i++)
    {
        std::map<QString, std::vector<QString>>::iterator j;
        for (j = i->second.begin(); j != i->second.end(); j++)
        {
            std::vector<QString> tempVector = j->second;
            int participantTimestampUnix = tempVector[1].toInt();
            QDateTime participantTimestamp;
            participantTimestamp.setTime_t(participantTimestampUnix);

            if (participantTimestamp.secsTo(QDateTime::currentDateTime()) > 10) //If the participant hasn't been active in the last minute
            {
                QSqlQuery q(Database::mDb);
                q.prepare("DELETE FROM roomSession WHERE streamId = :streamId AND roomId = :roomId");
                q.bindValue(":streamId", j->first);
                q.bindValue(":roomId", i->first);
                if (q.exec())
                {
                    databaseCounter++;
                }
                else
                {
                    qDebug() << "Failed Query" << Q_FUNC_INFO;
                }
                mMap[i->first].erase(j->first);
                j--;
                qMapParticipantsCounter++;
                /*
                if (i->isEmpty())
                {
                    mRoomsMultiMap.remove(i.key());
                    qMapRoomsCounter++;
                }
                */
            }
        }
    }
    qDebug() << QDateTime::currentDateTime().toString("d.MMMM yyyy hh:mm:ss") << "Successfully removed"
             << qMapParticipantsCounter << "(QMap P)" << qMapRoomsCounter << "(QMap R)" << databaseCounter << "(Database).";
}

void RoomsHandler::startRemovalTimer(int seconds)
{
    int milliseconds = seconds * 1000;
    mTimer->start(milliseconds);
}

void RoomsHandler::initialInsert(QString roomId, QString streamId, QString ipAddress, QString firstHeader)
{
    std::vector<QString> tempVector = {ipAddress, QString::number(QDateTime::currentSecsSinceEpoch()), firstHeader};
    mMap[roomId][streamId] = tempVector;
    qDebug() << "Added streamId, ipAddress and timestamp:" << tempVector[0] << tempVector[1] << "to the QMap after confirming with database";
}

void RoomsHandler::printQMap()
{
    std::map<QString, std::map<QString, std::vector<QString>>>::iterator i;
    for (i = mMap.begin(); i != mMap.end(); i++)
    {
        std::map<QString, std::vector<QString>>::iterator j;
        for (j = i->second.begin(); j != i->second.begin(); j++)
        {
            qDebug() << "Room:" << i->first << j->first;
            for (unsigned long k = 0; k < j->second.size(); k++)
            {
                qDebug() << k << j->second[k];
            }
        }
    }
}

void RoomsHandler::updateTimestamp(QString roomId, QString streamId)
{
    mMap[roomId][streamId][1] = QString::number(QDateTime::currentSecsSinceEpoch());
}

