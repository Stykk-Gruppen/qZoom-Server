#include "tcpserverhandler.h"

TcpServerHandler::TcpServerHandler(QObject *parent) : QObject(parent)
{
     initTcpServer();
     mPort = 1337;
}

void TcpServerHandler::initTcpServer()
{
    mTcpServer = new QTcpServer();
    connect(mTcpServer, &QTcpServer::newConnection, this, &TcpServerHandler::acceptTcpConnection);
    mTcpServer->listen(QHostAddress::Any, mPort);
}

void TcpServerHandler::acceptTcpConnection()
{

    mTcpServerConnection = mTcpServer->nextPendingConnection();
    if (!mTcpServerConnection) {
        qDebug() << "Error: got invalid pending connection!";
    }

    connect(mTcpServerConnection, &QIODevice::readyRead, this, &TcpServerHandler::readTcpPacket);
    //connect(tcpServerConnection, &QAbstractSocket::errorOccurred, this, &SocketHandler::displayError);
    connect(mTcpServerConnection, &QTcpSocket::disconnected, mTcpServerConnection, &QTcpSocket::deleteLater);

    mTcpServer->close();
}

void TcpServerHandler::readTcpPacket()
{
       /*QByteArray block;
       QDataStream out(&block, QIODevice::WriteOnly);
       out.setVersion(QDataStream::Qt_5_10);
       qDebug() << "sending password";
       //TODO verify password and send mysql password back
       out << "dbPassword";*/
    QByteArray originalData = mTcpServerConnection->readAll();
    QByteArray data = originalData;

    int roomIdLength = data[0];
    data.remove(0, 1);

    QByteArray roomIdArray = QByteArray(data, roomIdLength);
    QString roomId(roomIdArray);
    //QString test = QString(roomIdArray);
    // qDebug() << "roomId String: " << test;
    /// qDebug() << roomIdArray;
    //char* roomId = roomIdArray.data();
    data.remove(0, roomIdLength);

    int streamIdLength = data[0];
    data.remove(0, 1);

    //Finds the streamId header, stores it and removes it from the datagram
    QByteArray streamIdArray = QByteArray(data, streamIdLength);
    QString streamId(streamIdArray);
    //char* streamId = streamIdArray.data();
    data.remove(0, streamIdLength);

    if(mMap.count(roomId))
    {
        if (mMap[roomId].count(roomId))
        {
            std::map<QString, std::vector<QString>>::iterator i;
            for (i = mMap[roomId].begin(); i != mMap[roomId].end(); i++)
            {
                RoomsHandler::updateTimestamp(roomId, streamId);
                QtConcurrent::run(this, &TcpServerHandler::sendTcpPacket, originalData);
                qDebug() << "Sending to: " << mSenderAddress<< " with: " << mMap[roomId][streamId][1]; //Plass 1 er ipAddressen
            }
        }
        else
        {
            QSqlQuery q(Database::mDb);
            q.prepare("SELECT * FROM roomSession, user WHERE roomSession.userId = user.id AND user.streamId = :streamId");
            q.bindValue(":streamId", streamId);
            if (q.exec() && q.size() > 0)
            {
                q.next();
                RoomsHandler::initialInsert(roomId, streamId, mSenderAddress.toString(), QString(originalData));
                std::map<QString, std::vector<QString>>::iterator i;
                for (i = mMap[roomId].begin(); i != mMap[roomId].end(); i++)
                {
                    if (i->second[0] != streamId)
                    {
                        QString participantHeader = i->second[3];
                        /*
                        participantHeader.prepend(i->first);
                        participantHeader.prepend(i->first.size());
                        participantHeader.prepend(roomId);
                        participantHeader.prepend(roomId.size());
                        */
                        //QtConcurrent::run(sendHeader, QHostAddress(i->second[1]), originalData, mPort);
                        //QtConcurrent::run(sendHeader, mSenderAddress, participantHeader, mPort);
                    }
                }
            }
            else
            {
                qDebug() << "Could not find streamID (" << streamId << ") in roomSession (Database)";
            }
        }
    }
    else
    {
        QSqlQuery q(Database::mDb);
        q.prepare("SELECT rs.ipAddress FROM roomSession AS rs, user AS u WHERE rs.roomId = :roomId AND rs.userId = u.id AND u.streamId = :streamId");
        q.bindValue(":roomId", roomId);
        q.bindValue(":streamId", streamId);
        if (q.exec() && q.size() > 0)
        {
            while (q.next())
            {
                RoomsHandler::initialInsert(roomId, streamId, mSenderAddress.toString(), QString(originalData));
                qDebug() << "Added: " << roomId << " to QMap";
            }
        }
        else
        {
            qDebug() << "Could not find roomId (" << roomId << ") in Database";
        }
    }
}

int TcpServerHandler::sendTcpPacket(QByteArray arr)
{
    int ret = mTcpServerConnection->write(arr, arr.size());
    if(ret < 0)
    {
        qDebug() << mTcpServerConnection->errorString();
    }
    return ret;
}

void TcpServerHandler::sendHeader(QHostAddress receiverAddress, QByteArray data, uint16_t port)
{
    TcpSocketHandler tcpSocket(port);
    tcpSocket.sendHeader(receiverAddress, data);
}
