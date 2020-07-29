#include "tcpserverhandler.h"

TcpServerHandler::TcpServerHandler(RoomsHandler* _roomsHandler, QObject *parent) : QObject(parent), mRoomsHandler(_roomsHandler)
{     
     mPort = 1337;
     initTcpServer();
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
    if (!mTcpServerConnection)
    {
        qDebug() << "Error: got invalid pending connection!";
    }

    connect(mTcpServerConnection, &QIODevice::readyRead, this, &TcpServerHandler::readTcpPacket);
    //connect(tcpServerConnection, &QAbstractSocket::errorOccurred, this, &SocketHandler::displayError);
    connect(mTcpServerConnection, &QTcpSocket::disconnected, mTcpServerConnection, &QTcpSocket::deleteLater);
    //mTcpServer->close();
}

void TcpServerHandler::readTcpPacket()
{

    QByteArray data = mTcpServerConnection->readAll();
    qDebug() << mTcpServerConnection->peerAddress();
    QByteArray originalData = data;
    QByteArray header;

    int roomIdLength = data[0];
    data.remove(0, 1);

    QByteArray roomIdArray = QByteArray(data, roomIdLength);
    QString roomId(roomIdArray);
    data.remove(0, roomIdLength);
    header = data;

    QByteArray returnData = data;

    int streamIdLength = data[0];
    data.remove(0, 1);

    //Finds the streamId header, stores it and removes it from the datagram
    QByteArray streamIdArray = QByteArray(data, streamIdLength);
    QString streamId(streamIdArray);
    //char* streamId = streamIdArray.data();
    data.remove(0, streamIdLength);

    /*qDebug() << "streamId: " << streamId;
    qDebug() << "roomId: " << roomId;
    qDebug() << "ipv4: " << mTcpServerConnection->peerAddress().toIPv4Address();
    qDebug() << "ipv4 string: " << QString(mTcpServerConnection->peerAddress().toIPv4Address());*/

    //If the roomId is Debug, send back the recieved header
    if(roomId ==" Debug")
    {
         returnData.append(27);
         returnData.prepend(int(1));
         sendTcpPacket(mTcpServerConnection, returnData);
         return;
    }
    //sendTcpPacket(mTcpServerConnection,returnData);

    mRoomsHandler->mMutex->lock();
    if(mRoomsHandler->mMap.count(roomId))
    {
        if (mRoomsHandler->mMap[roomId].count(streamId))
        {
            //Will end up here if the user reconnects or updates before the map has been cleared.
            if(data == "BUMP")
            {
                mRoomsHandler->updateTimestamp(roomId, streamId);
            }
            else
            {
                mRoomsHandler->updateHeader(roomId, streamId, header);

                QByteArray tempArr;
                std::map<QString, std::vector<QByteArray>>::iterator i;
                for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
                {
                    qDebug() << "REJOIN Receiving header from:" << i->first;
                    if (i->first != streamId)
                    {
                        QString participantHeader = i->second[2];
                        /*
                        if (participantHeader.size() > 0)
                        {
                            tempArr.append(i->second[2]);
                            tempArr.append(27);
                        }
                        */
                        tempArr.append(i->second[2]);
                        tempArr.append(27);
                        QtConcurrent::run(sendHeader, QHostAddress(i->second[0].toUInt()), header, mPort);
                    }
                }
                tempArr.prepend(mRoomsHandler->mMap[roomId].size() - 1);
                //sendTcpPacket(mTcpServerConnection, tempArr);
                sendHeader(mSenderAddress, tempArr, mPort);
            }
        }
        else
        {
            qDebug() << "found room, didnt find streamId";
            QSqlQuery q(mRoomsHandler->Database::mDb);
            q.prepare("SELECT * FROM roomSession, user WHERE roomSession.userId = user.id AND user.streamId = :streamId");
            q.bindValue(":streamId", streamId);
            if (q.exec() && q.size() > 0)
            {
                q.next();
                mRoomsHandler->mMutex->lock();
                mRoomsHandler->initialInsert(roomId, streamId, QString::number(mTcpServerConnection->peerAddress().toIPv4Address()), header);
                mRoomsHandler->mMutex->unlock();
                std::map<QString, std::vector<QByteArray>>::iterator i;
                QByteArray tempArr;
                for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
                {
                    qDebug() << "Sending and receiving header from:" << i->first;
                    if (i->first != streamId)
                    {
                        QByteArray participantHeader = i->second[2];
                        tempArr.append(i->second[2]);
                        tempArr.append(27);
                        QtConcurrent::run(sendHeader, QHostAddress(i->second[0].toUInt()), header, mPort);
                    }
                }
                tempArr.prepend(mRoomsHandler->mMap[roomId].size() - 1);
                //sendTcpPacket(mTcpServerConnection, tempArr);
                sendHeader(mSenderAddress, tempArr, mPort);
            }
            else
            {
                qDebug() << "Could not find streamID (" << streamId << ") in roomSession (Database)";
                returnCodesArray.append(mTcpReturnValues::STREAM_ID_NOT_FOUND);
                sendTcpPacket(mTcpServerConnection,returnCodesArray);
                returnCodesArray.clear();
            }
        }
    }
    else
    {
        qDebug() << "did not find room or streamId";
        QSqlQuery q(mRoomsHandler->Database::mDb);
        q.prepare("SELECT * FROM roomSession AS rs, user AS u WHERE rs.roomId = :roomId AND rs.userId = u.id AND u.streamId = :streamId");
        q.bindValue(":roomId", roomId);
        q.bindValue(":streamId", streamId);
        if (q.exec() && q.size() > 0)
        {
            while (q.next())
            {
                mRoomsHandler->mMutex->lock();
                mRoomsHandler->initialInsert(roomId, streamId, QString::number(mTcpServerConnection->peerAddress().toIPv4Address()), header);
                mRoomsHandler->mMutex->unlock();
                qDebug() << "Added: " << roomId << " to Map with ipv4: ";
            }
            // When you create a new room, there is no information to send back, but we still need to reply
            returnCodesArray.append(mTcpReturnValues::SESSION_STARTED);
            sendTcpPacket(mTcpServerConnection, returnCodesArray);
            returnCodesArray.clear();
        }
        else
        {
            qDebug() << "Could not find roomId (" << roomId << ") in Database " << "streamId: " << streamId;
            returnCodesArray.append(mTcpReturnValues::ROOM_ID_NOT_FOUND);
            sendTcpPacket(mTcpServerConnection, returnCodesArray);
            returnCodesArray.clear();
        }
    }
    mRoomsHandler->mMutex->unlock();
    mTcpServerConnection->close();
}

int TcpServerHandler::sendTcpPacket(QTcpSocket *socket, QByteArray arr)
{
    int ret = socket->write(arr, arr.size());
    if(ret < 0)
    {
        qDebug() << socket->errorString();
    }
    return ret;
}

void TcpServerHandler::sendHeader(QHostAddress receiverAddress, QByteArray data, uint16_t port)
{
    //Important to add 0 as it signals that it is a header.
    data.prepend(int(0));
    TcpSocketHandler tcpSocket(port);
    tcpSocket.sendHeader(receiverAddress, data);
}
