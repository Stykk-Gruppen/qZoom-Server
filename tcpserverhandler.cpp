#include "tcpserverhandler.h"

TcpServerHandler::TcpServerHandler(RoomsHandler* _roomsHandler, int _portNumber, QObject *parent) : QObject(parent), mRoomsHandler(_roomsHandler)
{     
    mPortNumber = _portNumber;
    initTcpServer();
}

void TcpServerHandler::initTcpServer()
{
    mTcpServer = new QTcpServer();
    connect(mTcpServer, &QTcpServer::newConnection, this, &TcpServerHandler::acceptTcpConnection);
    mTcpServer->listen(QHostAddress::Any, mPortNumber);
    qDebug() << "TCP Listening on port:" << mPortNumber;
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

/*
void TcpServerHandler::sendParticipantRemovalNotice(QString roomId, QString streamId)
{
    QByteArray data;
    data.prepend(streamId.toLocal8Bit().data());
    data.prepend(streamId.size());
    data.prepend(int(REMOVE_PARTICIPANT));
    std::map<QString, Participant*>::iterator i;
    for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
    {
        QTcpSocket* qTcpSocket = i->second->getTcpSocket();
        sendTcpPacket(qTcpSocket, data);
    }
}

void TcpServerHandler::sendParticipantMutedNotice(QString roomId, QString streamId)
{
    QByteArray data;
    data.prepend(streamId.toLocal8Bit().data());
    data.prepend(streamId.size());
    data.prepend(int(VIDEO_DISABLED));
    std::map<QString, Participant*>::iterator i;
    for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
    {
        QTcpSocket* qTcpSocket = i->second->getTcpSocket();
        sendTcpPacket(qTcpSocket, data);
    }
}
*/

/**
 * When this socket disconnects, we first attempt to remove the user from the roomSession.
 * If the user existed and was successfully removed from the database, a header will be sent
 * to the other participants in the room to also remove said user from their clients.
 * @param readSocket QTcpSocket* the socket to the user
 * @param roomId QString which room the user belongs to
 * @param streamId QString identifier for the user
 */
void TcpServerHandler::setupDisconnectAction(QTcpSocket* readSocket, QString roomId, QString streamId)
{
    connect(readSocket, &QTcpSocket::disconnected, [=] ()
    {
        bool sqlSuccess = mRoomsHandler->removeParticipant(roomId, streamId);
        if(sqlSuccess)
        {
            QByteArray defaultSendHeader;
            defaultSendHeader.prepend(streamId.toLocal8Bit().data());
            defaultSendHeader.prepend(streamId.size());
            //TODO change to REMOVE_PARTICIPANT?
            sendHeaderToEveryParticipant(roomId, streamId, defaultSendHeader, VIDEO_DISABLED);

            //sendParticipantRemovalNotice(roomId, streamId);
        }

    });
}

void TcpServerHandler::readTcpPacket()
{
    QTcpSocket* readSocket = qobject_cast<QTcpSocket*>(sender());

    QHostAddress senderAddress = readSocket->peerAddress();

    QByteArray data = readSocket->readAll();
   // QByteArray originalData = data;
    int roomIdLength = data[0];
    data.remove(0, 1);

    QByteArray roomIdArray = QByteArray(data, roomIdLength);
    QString roomId(roomIdArray);
    data.remove(0, roomIdLength);

    //DisplayName and StreamId will be sent back to the client
    //We need to make a copy of this data before we start removing it
    QByteArray headerDataWithDisplayNameAndStreamId = data;

    int displayNameLength = data[0];
    data.remove(0, 1);

    QByteArray displayNameArray = QByteArray(data, displayNameLength);
    QString displayName(displayNameArray);
    data.remove(0, displayNameLength);



    int streamIdLength = data[0];
    data.remove(0, 1);

    //Finds the streamId header, stores it and removes it from the datagram
    QByteArray streamIdArray = QByteArray(data, streamIdLength);
    QString streamId(streamIdArray);
    data.remove(0, streamIdLength);

    qDebug() << "streamId: " << streamId;
    qDebug() << "roomId: " << roomId;
    qDebug() << "displayName: " << displayName;


    setupDisconnectAction(readSocket, roomId, streamId);

    /*qDebug() << "streamId: " << streamId;
    qDebug() << "roomId: " << roomId;
    qDebug() << "ipv4: " << mTcpServerConnection->peerAddress().toIPv4Address();
    qDebug() << "ipv4 string: " << QString(mTcpServerConnection->peerAddress().toIPv4Address());*/

    //If the roomId is Debug, send back the recieved header
    if(roomId == "Debug")
    {
        headerDataWithDisplayNameAndStreamId.append(27);
        headerDataWithDisplayNameAndStreamId.prepend(int(1));
        sendHeader(readSocket, headerDataWithDisplayNameAndStreamId, VIDEO_HEADER);
        return;
    }
    //sendTcpPacket(mTcpServerConnection,returnData);

    mRoomsHandler->mMutex->lock();
    if(mRoomsHandler->mMap.count(roomId))
    {
        if (mRoomsHandler->mMap[roomId].count(streamId))
        {
            qDebug() << "Found room and streamId, case: " <<data[0];
            QByteArray defaultSendHeader;
            defaultSendHeader.prepend(streamId.toLocal8Bit().data());
            defaultSendHeader.prepend(streamId.size());

            switch(data[0])
            {
            case VIDEO_HEADER:
            {
                mRoomsHandler->updateHeader(roomId, streamId, headerDataWithDisplayNameAndStreamId);
                //sendUpdatedDisplayNameToEveryParticipantInRoom(roomId, streamId, displayName);
                sendHeaderToEveryParticipant(roomId, streamId, headerDataWithDisplayNameAndStreamId, VIDEO_HEADER);
                break;
            }
            case NEW_DISPLAY_NAME:
            {
                mRoomsHandler->updateDisplayName(roomId, streamId, displayName);

                defaultSendHeader.prepend(displayName.toLocal8Bit().data());
                defaultSendHeader.prepend(displayName.size());

                sendHeaderToEveryParticipant(roomId, streamId, defaultSendHeader, NEW_DISPLAY_NAME);
                break;
            }
            case VIDEO_DISABLED:
            {
                sendHeaderToEveryParticipant(roomId, streamId, defaultSendHeader, VIDEO_DISABLED);
                break;
            }
            case AUDIO_DISABLED:
            {
                sendHeaderToEveryParticipant(roomId, streamId, defaultSendHeader, AUDIO_DISABLED);
                break;
            }
            default:
            {
                qDebug() << "Could not parse header code";
            }
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
                mRoomsHandler->initialInsert(roomId, streamId, displayName, headerDataWithDisplayNameAndStreamId, readSocket);
                SendAndRecieveFromEveryParticipantInRoom(roomId, streamId, headerDataWithDisplayNameAndStreamId, readSocket);
            }
            else
            {
                qDebug() << "Could not find streamID (" << roomId << ") in roomSession (Database)";
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
                mRoomsHandler->initialInsert(roomId, streamId, displayName, headerDataWithDisplayNameAndStreamId, readSocket);
                qDebug() << "Added: " << roomId << " to Map with ipv4: ";
            }
        }
        else
        {
            qDebug() << "Could not find roomId (" << roomId << ") in Database " << "streamId: " << streamId;
        }
    }
    mRoomsHandler->mMutex->unlock();
}

void TcpServerHandler::sendHeader(QTcpSocket* receiverSocket, QByteArray data, int headerValue)
{
    //Prepend headervalue
    data.prepend(headerValue);
    int ret = receiverSocket->write(data, data.size());
    if(ret < 0)
    {
        qDebug() << receiverSocket->errorString();
    }
}

/*
void TcpServerHandler::sendYourHeaderToEveryParticipantInRoom(QString roomId, QString streamId, QByteArray header)
{
    std::map<QString, Participant*>::iterator i;
    //Prepend number of headers
    header.prepend(int(1));
    //Append end of header char
    header.append(27);
    for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
    {
        if (i->first != streamId)
        {
            QTcpSocket* qTcpSocket = i->second->getTcpSocket();
            qDebug() << "UNMUTE, sending header from:" << streamId << " to: " << i->first ;
            sendHeader(qTcpSocket, header, VIDEO_HEADER);
        }
    }
}
*/

void TcpServerHandler::SendAndRecieveFromEveryParticipantInRoom(QString roomId, QString streamId, QByteArray header, QTcpSocket* readSocket)
{
    QByteArray tempArr;
    std::map<QString, Participant*>::iterator i;
    //Prepend number of headers
    header.prepend(int(1));
    //Append end of header char
    header.append(27);
    for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
    {
        if (i->first != streamId)
        {
            QByteArray participantHeader = i->second->getHeader();
            tempArr.append(participantHeader);
            //Append end of header char
            tempArr.append(27);
            QTcpSocket* qTcpSocket = i->second->getTcpSocket();
            qDebug() << "REJOIN, sending header from:" << streamId << " to: " << i->first ;

            sendHeader(qTcpSocket, header, VIDEO_HEADER);
        }
    }
    //Prepend number of headers
    tempArr.prepend(mRoomsHandler->mMap[roomId].size() - 1);
    qDebug() << "Sending all headers " << /*tempArr << */ " in map to: " << streamId;
    sendHeader(readSocket, tempArr, VIDEO_HEADER);
    //qDebug() << "After sending all headers in map to: " << streamId;
}

/*
void TcpServerHandler::sendUpdatedDisplayNameToEveryParticipantInRoom(QString roomId, QString streamId, QString displayName)
{
    QByteArray header;
    header.prepend(streamId.toLocal8Bit().data());
    header.prepend(streamId.size());

    header.prepend(displayName.toLocal8Bit().data());
    header.prepend(displayName.size());

    std::map<QString, Participant*>::iterator i;
    for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
    {

        if (i->first != streamId)
        {
            qDebug() << "Sending new displayName (" << displayName << ") to :" << i->first;
            QTcpSocket* qTcpSocket = i->second->getTcpSocket();
            sendHeader(qTcpSocket, header, NEW_DISPLAY_NAME);
        }
    }
}
*/

void TcpServerHandler::sendHeaderToEveryParticipant(QString roomId, QString streamId, QByteArray header, int headerCode)
{
    //Prepend number of headers
    header.prepend(int(1));
    //Append end of header char
    header.append(27);
    std::map<QString, Participant*>::iterator i;
    for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
    {
        qDebug() << "Sending new displayName to :" << i->first;
        if (i->first != streamId)
        {
            QTcpSocket* qTcpSocket = i->second->getTcpSocket();
            sendHeader(qTcpSocket, header, headerCode);
        }
    }
}

