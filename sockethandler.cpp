#include "sockethandler.h"

SocketHandler::SocketHandler(QObject *parent) : QObject(parent)
{
    mPort = 1337;
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
    int ret;
    while (mUdpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = mUdpSocket->receiveDatagram();
        QByteArray temp = QByteArray(datagram.data(), 5);

        char* roomId = QByteArray(datagram.data(), 5).data();
        datagram.data().remove(0, 5);
        char* streamId = QByteArray(datagram.data(), 5).data();

        if(mRooms.contains(roomId))
        {
            QList<char*> roomParticipants = mRooms.values(roomId);
            if(roomParticipants.contains(streamId))
            {
                for (int i = 0; i < roomParticipants.size(); i++)
                {
                    QString participantData = roomParticipants[i];
                    participantData.remove(0, 5);
                    ret = QtConcurrent::run(this, &SocketHandler::sendDatagram, temp, participantData);
                }
            }
            else
            {
                QSqlQuery q(*Database::db);
                q.prepare("SELECT roomSession.ipAddress FROM roomSession, user WHERE roomSession.userId = user.id AND user.streamId = :streamId");
                q.bindValue(":streamId", streamId);
                if (q.exec() && q.size() > 0)
                {
                    ret = 0;
                    std::string participantData = streamId;
                    participantData += streamId;

                    mRooms.insert(roomId, &participantData[0]);
                    qDebug() << "Added streamId + ipAddress: " << &participantData[0] << " to the QMap after confirming with database";
                }
                else
                {
                    ret = 2;
                    qDebug() << "Could not find streamID in roomSession (Database)";
                }
            }
        }
        else
        {
            QSqlQuery q(*Database::db);
            q.prepare("SELECT rs.roomId, rs.ipAddress, u.streamId FROM roomSession AS rs, user AS u WHERE rs.roomId = :roomId AND rs.userId = u.id");
            q.bindValue(":roomId", roomId);
            if (q.exec() && q.size() > 0)
            {
                while (q.next())
                {
                    std::string participantData = "";
                    participantData += q.value(2).toString().toStdString();
                    participantData += q.value(1).toString().toStdString();
                    mRooms.insert(roomId, &participantData[0]);
                    qDebug() << "Added: " << &participantData[0] << " to QMap";
                }
                ret = 0;
            }
            else
            {
                ret = 2;
                qDebug() << "Could not find roomId in Database";
            }
        }

        /*
        QByteArray arrayWithoutHeaderByte;
        arrayWithoutHeaderByte.append(datagram.data());
        arrayWithoutHeaderByte.remove(0, 1);
        if(audioOrVideoInt == 0)
        {

        }
        else if (audioOrVideoInt == 1)
        {

        }
        else
        {
            qDebug() << "UDP Header byte was not 1 or 0 in socketHandler, stopping program";
            exit(-1);
        }
        */
    }
    //return ret;

    //qDebug() << "buffer size " << mBuffer.size() << "after signal: " << signalCount;


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
