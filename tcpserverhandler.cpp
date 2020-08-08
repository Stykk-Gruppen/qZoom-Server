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
}

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
        mRoomsHandler->getMutex()->lock();
        bool sqlSuccess = mRoomsHandler->removeParticipant(roomId, streamId);
        if(sqlSuccess)
        {
            QByteArray defaultSendHeader;
            defaultSendHeader.prepend(streamId.toLocal8Bit().data());
            defaultSendHeader.prepend(streamId.size());
            //TODO change to REMOVE_PARTICIPANT?
            if(mRoomsHandler->getMap().count(roomId))
            {
                sendHeaderToEveryParticipant(roomId, streamId, defaultSendHeader, REMOVE_PARTICIPANT);
            }
        }
        mRoomsHandler->getMutex()->unlock();
    });
}

void TcpServerHandler::readTcpPacket()
{

    QTcpSocket* readSocket = qobject_cast<QTcpSocket*>(sender());

    QHostAddress senderAddress = readSocket->peerAddress();

    QByteArray data = readSocket->readAll();
    //qDebug() << "data: " << data;

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
    qDebug() << "map: " << mRoomsHandler->getMap();
    //qDebug() << "map adr: " << &mRoomsHandler->mMap;
    //qDebug() << "data: " << data;
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

    mRoomsHandler->getMutex()->lock();
    if(mRoomsHandler->getMap().count(roomId))
    {
        //TODO remove data.size
        if (mRoomsHandler->getMap().at(roomId).count(streamId) && data.size() >= 1)
        {
            //qDebug() << "Found room and streamId, case: " << data[0];
            QByteArray defaultSendHeader;
            defaultSendHeader.prepend(streamId.toLocal8Bit().data());
            defaultSendHeader.prepend(streamId.size());

            switch((int)data[0])
            {
            case VIDEO_HEADER:
            {
                qDebug() << "video header case";
                mRoomsHandler->updateVideoHeader(roomId, streamId, data);
                sendHeaderToEveryParticipant(roomId, streamId, headerDataWithDisplayNameAndStreamId, VIDEO_HEADER);
                break;
            }
            case NEW_DISPLAY_NAME:
            {
                qDebug() << "new name case";
                mRoomsHandler->updateDisplayName(roomId, streamId, displayName);

                defaultSendHeader.prepend(displayName.toLocal8Bit().data());
                defaultSendHeader.prepend(displayName.size());

                sendHeaderToEveryParticipant(roomId, streamId, defaultSendHeader, NEW_DISPLAY_NAME);
                break;
            }
            case VIDEO_DISABLED:
            {
                qDebug() << "video disabled case";
                sendHeaderToEveryParticipant(roomId, streamId, defaultSendHeader, VIDEO_DISABLED);
                break;
            }
            case AUDIO_DISABLED:
            {
                qDebug() << "audio disabled case";
                sendHeaderToEveryParticipant(roomId, streamId, defaultSendHeader, AUDIO_DISABLED);
                break;
            }
            case KICK_PARTICIPANT:
            {
                qDebug() << "kick participant case";
                //qDebug() << data;
                data.remove(0, 1);
                QByteArray tempArr = data;
                int participantStreamIdLength = tempArr[0];
                tempArr.remove(0, 1);
                QByteArray participantStreamIdArray = QByteArray(tempArr, participantStreamIdLength);
                QString participantStreamId(participantStreamIdArray);
                QTcpSocket* sckt = mRoomsHandler->getMap().at(roomId).at(participantStreamId)->getTcpSocket();
                sendHeader(sckt, data, KICK_PARTICIPANT);
                break;
            }
            default:
            {
                qDebug() << "Could not parse header code: " << data[0];
                break;
            }
            }

        }
        else
        {
            qDebug() << "found room, didnt find streamId";
            QSqlQuery q(mRoomsHandler->getDb());
            q.prepare("SELECT * FROM roomSession, user WHERE roomSession.userId = user.id AND user.streamId = :streamId");
            q.bindValue(":streamId", streamId);
            if (q.exec() && q.size() > 0)
            {
                q.next();
                mRoomsHandler->initialInsert(roomId, streamId, displayName, data, readSocket);
                SendAndRecieveFromEveryParticipantInRoom(roomId, streamId, headerDataWithDisplayNameAndStreamId, readSocket);
            }
            else
            {
                qDebug() << "Could not find streamID (" << streamId << ") in roomSession (Database)";
            }
        }
    }
    else
    {
        qDebug() << "did not find room or streamId";
        QSqlQuery q(mRoomsHandler->getDb());
        q.prepare("SELECT * FROM roomSession AS rs, user AS u WHERE rs.roomId = :roomId AND rs.userId = u.id AND u.streamId = :streamId");
        q.bindValue(":roomId", roomId);
        q.bindValue(":streamId", streamId);
        if (q.exec() && q.size() > 0)
        {
            while (q.next())
            {
                mRoomsHandler->initialInsert(roomId, streamId, displayName, data, readSocket);
                qDebug() << "Added: " << roomId << " to Map with ipv4: ";
            }
        }
        else
        {
            qDebug() << "Could not find the roomId (" << roomId << ") and streamId(" << streamId << ") combo in the database";
        }
    }
    mRoomsHandler->getMutex()->unlock();
}

void TcpServerHandler::sendHeader(QTcpSocket* receiverSocket, QByteArray data, int headerValue)
{
    //Prepend headervalue
    if(!receiverSocket)
    {
        qDebug() << "Did not find socket " << Q_FUNC_INFO;
        return;
    }
    //qDebug() << "header data: " << data;
    data.prepend(headerValue);
    int ret = receiverSocket->write(data, data.size());
    if(ret < 0)
    {
        qDebug() << receiverSocket->errorString();
    }
}

void TcpServerHandler::SendAndRecieveFromEveryParticipantInRoom(const QString& roomId, const QString& streamId, QByteArray header, QTcpSocket* readSocket)
{
    QByteArray tempArr;
    const std::map<QString, std::map<QString, Participant*>> map = mRoomsHandler->getMap();
    std::map<QString, Participant*>::const_iterator i;
    //Prepend number of headers
    header.prepend(int(1));
    //Append end of header char
    header.append(27);
    for (i = map.at(roomId).begin(); i != map.at(roomId).end(); i++)
    {
        //TODO figure out why map gets populated by a streamId and nullptr
        if (i->first != streamId && i->second)
        {
            QByteArray participantHeader = i->second->getVideoHeader();
            //StreamID
            participantHeader.prepend(i->first.toLocal8Bit().data());
            participantHeader.prepend(i->first.size());

            participantHeader.prepend(i->second->getDisplayName().toLocal8Bit().data());
            participantHeader.prepend(i->second->getDisplayName().size());

            // participantHeader.prepend(roomId.toLocal8Bit().data());
            // participantHeader.prepend(roomId.size());

            tempArr.append(participantHeader);
            //Append end of header char
            tempArr.append(27);
            QTcpSocket* qTcpSocket = i->second->getTcpSocket();
            //qDebug() << "REJOIN, sending header from:" << streamId << " to: " << i->first ;
            sendHeader(qTcpSocket, header, VIDEO_HEADER);
        }
    }
    //Prepend number of headers
    tempArr.prepend(map.at(roomId).size() - 1);
    qDebug() << "Sending all headers " << /*tempArr << */ " in map to: " << streamId;
    sendHeader(readSocket, tempArr, VIDEO_HEADER);
    //qDebug() << "After sending all headers in map to: " << streamId;
}

void TcpServerHandler::sendHeaderToEveryParticipant(QString roomId, QString streamId, QByteArray header, int headerCode)
{
    //Prepend number of headers
    header.prepend(int(1));
    //Append end of header char
    header.append(27);
    std::map<QString, Participant*>::const_iterator i;
    const std::map<QString, std::map<QString, Participant*>> map = mRoomsHandler->getMap();
    for (i = map.at(roomId).begin(); i != map.at(roomId).end(); i++)
    {
        //TODO figure out why map gets populated by a streamId and nullptr
        if (i->first != streamId && i->second)
        {
            qDebug() << "Sending new headers from " << streamId << " to :" << i->first << " header code: " << headerCode;
            qDebug() << Q_FUNC_INFO << " map: " << map;
            QTcpSocket* qTcpSocket = i->second->getTcpSocket();
            sendHeader(qTcpSocket, header, headerCode);
        }
    }
}

