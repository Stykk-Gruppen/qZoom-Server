#include "sockethandler.h"

SocketHandler::SocketHandler(QObject *parent) : QObject(parent)
{
    mPort = 1337;
    mStreamIdLength = 5;
    mRoomIdLength = 5;

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(removeOldParticipantsFromQMap()));
    initSocket();
}

void SocketHandler::initSocket()
{
    mUdpSocket = new QUdpSocket(this);
    mUdpSocket->bind(QHostAddress::Any, mPort, QAbstractSocket::ShareAddress);

    //Connects readyRead to readPendingDatagram function,
    //which means when the socket recieves a packet the function will run.
    connect(mUdpSocket, &QUdpSocket::readyRead, this, &SocketHandler::readPendingDatagrams);
}

void SocketHandler::readPendingDatagrams()
{
    while (mUdpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = mUdpSocket->receiveDatagram();
        QByteArray temp = QByteArray(datagram.data(), mRoomIdLength);

        char* roomId = QByteArray(temp.data(), mRoomIdLength).data();
        datagram.data().remove(0, mRoomIdLength);
        char* streamId = QByteArray(datagram.data(), mStreamIdLength).data();

        if(mRoomsMap.contains(roomId))
        {
            QList<char*> roomParticipants = mRoomsMap.values(roomId);
            if(roomParticipants.contains(streamId))
            {
                for (int i = 0; i < roomParticipants.size(); i++)
                {
                    QString participantData = roomParticipants[i];
                    participantData.remove(0, 5);
                    QtConcurrent::run(this, &SocketHandler::sendDatagram, datagram.data(), participantData);
                }
            }
            else
            {
                QSqlQuery q(Database::mDb);
                q.prepare("SELECT roomSession.ipAddress FROM roomSession, user WHERE roomSession.userId = user.id AND user.streamId = :streamId");
                q.bindValue(":streamId", streamId);
                if (q.exec() && q.size() > 0)
                {
                    std::string participantData = streamId;
                    //participantData += streamId;
                    participantData += std::to_string(QDateTime::currentSecsSinceEpoch());
                    participantData += q.value(0).toString().toStdString();

                    mRoomsMap.insert(roomId, &participantData[0]);
                    qDebug() << "Added streamId, ipAddress and timestamp: " << &participantData[0] << " to the QMap after confirming with database";
                }
                else
                {
                    qDebug() << "Could not find streamID in roomSession (Database)";
                }
            }
        }
        else
        {
            QSqlQuery q(Database::mDb);
            q.prepare("SELECT rs.roomId, rs.ipAddress, u.streamId FROM roomSession AS rs, user AS u WHERE rs.roomId = :roomId AND rs.userId = u.id");
            q.bindValue(":roomId", roomId);
            if (q.exec() && q.size() > 0)
            {
                while (q.next())
                {
                    std::string participantData = "";
                    participantData += q.value(2).toString().toStdString();
                    participantData += std::to_string(QDateTime::currentSecsSinceEpoch());
                    participantData += q.value(1).toString().toStdString();
                    mRoomsMap.insert(roomId, &participantData[0]);
                    qDebug() << "Added: " << &participantData[0] << " to QMap";
                }
            }
            else
            {
                qDebug() << "Could not find roomId in Database";
            }
        }
    }
}

int SocketHandler::sendDatagram(QByteArray arr, QString participantAddress)
{
    int ret = mUdpSocket->writeDatagram(arr, arr.size(), QHostAddress(participantAddress), mPort);
    if(ret < 0)
    {
        qDebug() << mUdpSocket->error();
    }
    return ret;
}

void SocketHandler::removeOldParticipantsFromQMap()
{
    int counter = 0;
    QMultiMap<char*, char*>::iterator i;
    for (i = mRoomsMap.begin(); i != mRoomsMap.end(); i++)
    {
        QList<char*> roomParticipants = mRoomsMap.values(i.value());
        QString currentRoomId = mRoomsMap.value(i.value());
        for (int j = 0; j < roomParticipants.size(); j++)
        {
            QString participantData = roomParticipants[j];
            QString participantStreamId = participantData.mid(0, mStreamIdLength);
            int participantTimestampUnix = participantData.mid(mStreamIdLength, 19).toInt();
            QDateTime participantTimestamp;
            participantTimestamp.setTime_t(participantTimestampUnix);

            if (participantTimestamp.secsTo(QDateTime::currentDateTime()) > 60) //If the participant hasn't been active in the last minute
            {
                QSqlQuery q(Database::mDb);
                q.prepare("DELETE FROM roomSession WHERE streamId = :streamId AND roomId = :roomId");
                q.bindValue(":streamId", participantStreamId);
                q.bindValue(":roomId", currentRoomId);
                if (q.exec())
                {
                    counter++;
                }
                else
                {
                    qDebug() << "Failed Query" << Q_FUNC_INFO;
                }
            }
        }
    }
    qDebug() << QDateTime::currentDateTime().toString("d.MMMM yyyy hh:mm:ss") << "Successfully removed" << counter << "participant(s) from the QMap.";
}

void SocketHandler::startRemovalTimer(int seconds)
{
    int milliseconds = seconds * 1000;
    mTimer->start(milliseconds);
}




















