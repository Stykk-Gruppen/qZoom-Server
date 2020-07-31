#include "tcpserverhandler.h"

TcpServerHandler::TcpServerHandler(RoomsHandler* _roomsHandler, QObject *parent) : QObject(parent), mRoomsHandler(_roomsHandler)
{     
    mPort = 1338;
    initTcpServer();
}

void TcpServerHandler::initTcpServer()
{
    mTcpServer = new QTcpServer();
    connect(mTcpServer, &QTcpServer::newConnection, this, &TcpServerHandler::acceptTcpConnection);
    mTcpServer->listen(QHostAddress::Any, mPort);
    qDebug() << "TCP Listening on port:" << mPort;
}

void TcpServerHandler::acceptTcpConnection()
{
    QTcpSocket* mTcpServerConnection = mTcpServer->nextPendingConnection();
    if (!mTcpServerConnection)
    {
        qDebug() << "Error: got invalid pending connection!";
    }

    connect(mTcpServerConnection, &QIODevice::readyRead, this, &TcpServerHandler::readTcpPacket);
    //connect(tcpServerConnection, &QAbstractSocket::errorOccurred, this, &SocketHandler::displayError);

    //mTcpServer->close();
}



void TcpServerHandler::sendParticipantRemovalNotice(QString roomId, QString streamId)
{
    QByteArray data;
    std::map<QString, Participant*>::iterator i;
    for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
    {
        data.prepend(streamId.toLocal8Bit().data());
        data.prepend(streamId.size());
        // 1 = remove participant
        data.prepend(int(1));
        QTcpSocket* qTcpSocket = i->second->getTcpSocket();
        sendTcpPacket(qTcpSocket, data);
    }
}


void TcpServerHandler::readTcpPacket()
{
    QTcpSocket* readSocket = qobject_cast<QTcpSocket*>(sender());
    QHostAddress senderAddress = readSocket->peerAddress();
    QByteArray data = readSocket->readAll();
    QByteArray originalData = data;
    QByteArray header;

    int roomIdLength = data[0];
    data.remove(0, 1);

    QByteArray roomIdArray = QByteArray(data, roomIdLength);
    //QString roomId(roomIdArray);
    mRoomId = QString(roomIdArray);
    data.remove(0, roomIdLength);
    header = data;

    int displayNameLength = data[0];
    data.remove(0, 1);

    QByteArray displayNameArray = QByteArray(data, displayNameLength);
    //QString displayName(displayNameArray);
    mDisplayName = QString(displayNameArray);
    data.remove(0, displayNameLength);

    QByteArray returnData = data;

    int streamIdLength = data[0];
    data.remove(0, 1);

    //Finds the streamId header, stores it and removes it from the datagram
    QByteArray streamIdArray = QByteArray(data, streamIdLength);
    //QString streamId(streamIdArray);
    mStreamId = QString(streamIdArray);
    data.remove(0, streamIdLength);

    qDebug() << "streamId: " << mStreamId;
    qDebug() << "roomId: " << mRoomId;
    qDebug() << "displayName: " << mDisplayName;

    connect(readSocket, &QTcpSocket::disconnected, [=] () {
        mRoomsHandler->removeParticipant(mRoomId, mStreamId);
        sendParticipantRemovalNotice(mRoomId, mStreamId);
    });
    // connect(readSocket, &QTcpSocket::disconnected, this, &TcpServerHandler::socketDiconnected);

    /*qDebug() << "streamId: " << streamId;
    qDebug() << "roomId: " << roomId;
    qDebug() << "ipv4: " << mTcpServerConnection->peerAddress().toIPv4Address();
    qDebug() << "ipv4 string: " << QString(mTcpServerConnection->peerAddress().toIPv4Address());*/

    //If the roomId is Debug, send back the recieved header
    if(mRoomId == "Debug")
    {
        returnData.append(27);
        returnData.prepend(int(1));
        sendTcpPacket(readSocket, returnData);
        return;
    }
    //sendTcpPacket(mTcpServerConnection,returnData);

    mRoomsHandler->mMutex->lock();
    if(mRoomsHandler->mMap.count(mRoomId))
    {
        if (mRoomsHandler->mMap[mRoomId].count(mStreamId))
        {
            //Will end up here if the user reconnects or updates before the map has been cleared.
            if(data == "DISPLAY_NAME_UPDATE")
            {
                mRoomsHandler->updateDisplayName(mRoomId, mStreamId, mDisplayName);
                sendUpdatedDisplayNameToEveryParticipantInRoom();
            }
            else
            {
                mRoomsHandler->updateHeader(mRoomId, mStreamId, header);
                SendAndRecieveFromEveryParticipantInRoom(header,readSocket);
            }
        }
        else
        {
            qDebug() << "found room, didnt find streamId";
            QSqlQuery q(mRoomsHandler->Database::mDb);
            q.prepare("SELECT * FROM roomSession, user WHERE roomSession.userId = user.id AND user.streamId = :streamId");
            q.bindValue(":streamId", mStreamId);
            if (q.exec() && q.size() > 0)
            {
                q.next();
                mRoomsHandler->initialInsert(mRoomId, mStreamId, mDisplayName, header, readSocket);
                SendAndRecieveFromEveryParticipantInRoom(header,readSocket);
                /*
=======

                mRoomsHandler->initialInsert(roomId, streamId, QString::number(mTcpServerConnection->peerAddress().toIPv4Address()), header, mTcpServerConnection);

>>>>>>> 753274aeea8811c7bad285fdf3501173356f8137
                std::map<QString, std::vector<QVariant>>::iterator i;

                QByteArray tempArr;
                for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
                {
                    qDebug() << "Sending and receiving header from:" << i->first;
                    if (i->first != streamId)
                    {
                        QByteArray participantHeader = i->second[2].toByteArray();
                        tempArr.append(i->second[2].toByteArray());
                        tempArr.append(27);
                        QPointer<QTcpSocket> qTcpSocket = i->second[3].value<QPointer<QTcpSocket>>();
                        QtConcurrent::run(sendHeader, qTcpSocket, header);

                    }
                }
                tempArr.prepend(mRoomsHandler->mMap[roomId].size() - 1);
                //sendTcpPacket(mTcpServerConnection, tempArr);
                //Sends all headers currently in the map back to sender.

                sendHeader(mTcpServerConnection, tempArr);
                */



            }
            else
            {
                qDebug() << "Could not find streamID (" << mStreamId << ") in roomSession (Database)";
                returnCodesArray.append(mTcpReturnValues::STREAM_ID_NOT_FOUND);
                sendTcpPacket(readSocket, returnCodesArray);
                returnCodesArray.clear();
            }
        }
    }
    else
    {
        qDebug() << "did not find room or streamId";
        QSqlQuery q(mRoomsHandler->Database::mDb);
        q.prepare("SELECT * FROM roomSession AS rs, user AS u WHERE rs.roomId = :roomId AND rs.userId = u.id AND u.streamId = :streamId");
        q.bindValue(":roomId", mRoomId);
        q.bindValue(":streamId", mStreamId);
        if (q.exec() && q.size() > 0)
        {
            while (q.next())
            {
                mRoomsHandler->initialInsert(mRoomId, mStreamId, mDisplayName, header, readSocket);
                qDebug() << "Added: " << mRoomId << " to Map with ipv4: ";
            }
            // When you create a new room, there is no information to send back, but we still need to reply
            returnCodesArray.append(mTcpReturnValues::SESSION_STARTED);
            sendTcpPacket(readSocket, returnCodesArray);
            returnCodesArray.clear();
        }
        else
        {
            qDebug() << "Could not find roomId (" << mRoomId << ") in Database " << "streamId: " << mStreamId;
            returnCodesArray.append(mTcpReturnValues::ROOM_ID_NOT_FOUND);
            sendTcpPacket(readSocket, returnCodesArray);
            returnCodesArray.clear();
        }
    }
    mRoomsHandler->mMutex->unlock();
    readSocket->close();
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

void TcpServerHandler::sendHeader(QTcpSocket* receiverSocket, QByteArray data, int headerValue)
{
    //Important to add 0 as it signals that it is a header.
    data.prepend(headerValue);
    sendTcpPacket(receiverSocket, data);
}

void TcpServerHandler::SendAndRecieveFromEveryParticipantInRoom(QByteArray header, QTcpSocket* readSocket)
{
    QByteArray tempArr;
    std::map<QString, Participant*>::iterator i;
    for (i = mRoomsHandler->mMap[mRoomId].begin(); i != mRoomsHandler->mMap[mRoomId].end(); i++)
    {
        qDebug() << "REJOIN Receiving header from:" << i->first;
        if (i->first != mStreamId)
        {
            QByteArray participantHeader = i->second->getHeader();
            tempArr.append(participantHeader);
            tempArr.append(27);
            QTcpSocket* qTcpSocket = i->second->getTcpSocket();
            QtConcurrent::run(sendHeader, qTcpSocket, header, VIDEO_HEADER);
        }
    }
    tempArr.prepend(mRoomsHandler->mMap[mRoomId].size() - 1);
    sendHeader(readSocket, tempArr, VIDEO_HEADER);
}

void TcpServerHandler::sendUpdatedDisplayNameToEveryParticipantInRoom()
{
    QByteArray header;
    header.prepend(mStreamId.toLocal8Bit().data());
    header.prepend(mStreamId.size());

    header.prepend(mDisplayName.toLocal8Bit().data());
    header.prepend(mDisplayName.size());

    std::map<QString, Participant*>::iterator i;
    for (i = mRoomsHandler->mMap[mRoomId].begin(); i != mRoomsHandler->mMap[mRoomId].end(); i++)
    {
        qDebug() << "Sending new displayName to :" << i->first;
        if (i->first != mStreamId)
        {
            QTcpSocket* qTcpSocket = i->second->getTcpSocket();
            QtConcurrent::run(sendHeader, qTcpSocket, header, NEW_DISPLAY_NAME);
        }
    }
}

