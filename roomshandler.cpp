#include "roomshandler.h"

RoomsHandler::RoomsHandler()
{
    mMutex = new std::mutex;
}

void RoomsHandler::removeOldParticipantsFromQMap()
{

    //std::vector<QString> vec = {"192.19.293.23", "1595613767", "Header"};
    //mMap["Delta"]["Stian"] = vec;

    mMutex->lock();
    Database* db = new Database(); //Each thread requires their own database connection.
    int mapParticipantsCounter = 0;
    int mapRoomsCounter = 0;
    int databaseCounter = 0;
    std::map<QString, std::map<QString, std::vector<QByteArray>>>::iterator i = mMap.begin();
    while (i != mMap.end())
    {
        std::map<QString, std::vector<QByteArray>>::iterator j = i->second.begin();
        while (j != i->second.end())
        {
            int participantTimestampUnix = j->second[1].toInt();
            QDateTime participantTimestamp;
            participantTimestamp.setTime_t(participantTimestampUnix);

            if ((participantTimestamp.secsTo(QDateTime::currentDateTime()) > 60)) //If the participant hasn't been active in the last minute
            {
                QSqlQuery q(db->mDb);
                q.prepare("DELETE FROM roomSession WHERE streamId = :streamId AND roomId = :roomId");
                q.bindValue(":streamId", j->first);
                q.bindValue(":roomId", i->first);
                if (q.exec())
                {
                    databaseCounter++;
                }
                else
                {
                    qDebug() << "Failed Query" << Q_FUNC_INFO << q.lastError();
                }

                j = mMap[i->first].erase(j);
                mapParticipantsCounter++;
            }
            else
            {
                ++j;
            }
        }
        if (i->second.size() == 0)
        {
            i = mMap.erase(i);
            mapRoomsCounter++;
        }
        else
        {
            ++i;
        }
    }
    qDebug() << QDateTime::currentDateTime().toString("d.MMMM yyyy hh:mm:ss") << "Successfully removed"
             << mapParticipantsCounter << "(QMap P)" << mapRoomsCounter << "(QMap R)" << databaseCounter << "(Database).";
    delete db;
    mMutex->unlock();
}

void RoomsHandler::initialInsert(QString roomId, QString streamId, QString ipAddress, QByteArray header)
{
    std::vector<QByteArray> tempVector = {ipAddress.toUtf8(), QString::number(QDateTime::currentSecsSinceEpoch()).toUtf8(), header};
    mMap[roomId][streamId] = tempVector;
    qDebug() << "Added streamId, ipAddress and timestamp:" << streamId << tempVector[0] << tempVector[1] << "to the QMap after confirming with database";
}

void RoomsHandler::printMap()
{
    mMutex->lock();
    qDebug() << "Printing start";
    std::map<QString, std::map<QString, std::vector<QByteArray>>>::iterator i;
    for (i = mMap.begin(); i != mMap.end(); i++)
    {
        std::map<QString, std::vector<QByteArray>>::iterator j;
        for (j = i->second.begin(); j != i->second.begin(); j++)
        {
            qDebug() << "Room:" << i->first << j->first;
            for (unsigned long k = 0; k < j->second.size(); k++)
            {
                qDebug() << k << j->second[k];
            }
        }
    }
    qDebug() << "Printing end";
    mMutex->unlock();
}

void RoomsHandler::updateHeader(QString roomId, QString streamId, QByteArray header)
{
    mMap[roomId][streamId][2] = header;
}

void RoomsHandler::updateTimestamp(QString roomId, QString streamId)
{
    mMap[roomId][streamId][1] = QString::number(QDateTime::currentSecsSinceEpoch()).toUtf8();
}

void RoomsHandler::startRemovalTimer(int seconds)
{
    qDebug() << "Removing inactive participants every" << seconds << "seconds.";
    int milliseconds = seconds * 1000;
    mAbortRemoval = false;
    std::thread t([=]()
    {
        while (!mAbortRemoval)
        {
            printMap();
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            removeOldParticipantsFromQMap();
        }
    });
    t.detach();
}

void RoomsHandler::removeParticipant(QString roomId, QString streamId)
{
    Database* db = new Database(); //Each thread requires their own database connection.

    mMap[roomId].erase(streamId);

    QSqlQuery q(db->mDb);
    q.prepare("DELETE FROM roomSession WHERE streamId = :streamId AND roomId = :roomId");
    q.bindValue(":streamId", streamId);
    q.bindValue(":roomId", roomId);
    if (q.exec())
    {
        qDebug() << "Removed participant" << streamId << "from the database";
    }
    else
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO << q.lastError();
    }






    delete db;
}

