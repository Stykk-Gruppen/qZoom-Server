#include "tcpserverhandler.h"

TcpServerHandler::TcpServerHandler(RoomsHandler* _roomsHandler, int _portNumber, QObject *parent) : QObject(parent), mRoomsHandler(_roomsHandler)
{
    mPortNumber = _portNumber;
    initTcpServer();
}

/**
 * Creates a new QTcpServer objects, connects the newConnection
 * signal with our acceptTcpConnection slot and starts listening on mPortNumber.
 */
void TcpServerHandler::initTcpServer()
{
    mTcpServer = new QTcpServer();
    connect(mTcpServer, &QTcpServer::newConnection, this, &TcpServerHandler::acceptTcpConnection);
    mTcpServer->listen(QHostAddress::Any, mPortNumber);
    qDebug() << "TCP Listening on port:" << mPortNumber;
}

/**
 * When the QTcpServer gets a newConnection signal, it will get the corresponding QTcpSocket
 * and connect its readyRead signal with our readTcpPacket slot
 */
void TcpServerHandler::acceptTcpConnection()
{
    QTcpSocket* socket = mTcpServer->nextPendingConnection();
    if (!socket)
    {
        qDebug() << "Error: got invalid pending connection!";
    }

    connect(socket, &QIODevice::readyRead, this, &TcpServerHandler::readTcpPacket);
    //connect(tcpServerConnection, &QAbstractSocket::errorOccurred, this, &SocketHandler::displayError);
}

/**
 * When the readSocket disconnects, we first attempt to remove the user from the roomSession.
 * If the user existed and was successfully removed from the database, a header will be sent
 * to the other participants in the room to also remove said user from their clients.
 * @param readSocket QTcpSocket* the socket to the user
 * @param roomId QString which room the user belongs to
 * @param streamId QString identifier for the user
 */
void TcpServerHandler::setupDisconnectAction(QTcpSocket* readSocket, const QString& roomId, const QString& streamId)
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
            if(mRoomsHandler->getMap().count(roomId))
            {
                sendHeaderToEveryParticipant(roomId, streamId, defaultSendHeader, REMOVE_PARTICIPANT);
            }
        }
        mRoomsHandler->getMutex()->unlock();
    });
}
/**
 * @brief TcpServerHandler::printTcpPacketInfo
 * @param sender
 * @param streamId
 * @param roomId
 * @param displayName
 * @param entirePacket
 */
void TcpServerHandler::printTcpPacketInfo(QHostAddress sender, QString streamId,
                                          QString roomId, QString displayName,
                                          QByteArray entirePacket)
{
    time_t now = time(0);
    char* dt = ctime(&now);
    qDebug() << "Timetamp: " << dt;
    qDebug() << "Sender: " << sender;
    qDebug() << "streamId: " << streamId;
    qDebug() << "roomId: " << roomId;
    qDebug() << "displayName: " << displayName;
    qDebug() << "map: " << mRoomsHandler->getMap();
    qDebug() << "Packet: " << entirePacket;
}

/**
 * Reads and parse the data sent to this socket and takes various actions depending
 * on the data recieved
 */
void TcpServerHandler::readTcpPacket()
{

    //We get the sender object with qobject_cast, since we experienced multithreading problems using a member object
    QTcpSocket* readSocket = qobject_cast<QTcpSocket*>(sender());
    QHostAddress senderAddress = readSocket->peerAddress();
    QByteArray data = readSocket->readAll();

    int roomIdLength = data[0];
    data.remove(0, 1);

    QByteArray roomIdArray = QByteArray(data, roomIdLength);
    QString roomId(roomIdArray);
    data.remove(0, roomIdLength);

    int displayNameLength = data[0];
    data.remove(0, 1);

    QByteArray displayNameArray = QByteArray(data, displayNameLength);
    QString displayName(displayNameArray);
    data.remove(0, displayNameLength);

    int streamIdLength = data[0];
    data.remove(0, 1);

    //Finds the streamId string, stores it and removes it from the datagram
    QByteArray streamIdArray = QByteArray(data, streamIdLength);
    QString streamId(streamIdArray);
    data.remove(0, streamIdLength);
    setupDisconnectAction(readSocket, roomId, streamId);

    //Checks if the header is a VIDEO_HEADER.
    //If true, the number pointing out the VIDEO_HEADER gets removed from the header.
    QByteArray cleanVideoHeader;
    QByteArray cleanVideoHeaderWithStreamIdAndDisplayName;
    if ((int)data[0] == VIDEO_HEADER)
    {
        cleanVideoHeader = data;
        cleanVideoHeader.remove(0, 1);
        cleanVideoHeaderWithStreamIdAndDisplayName = cleanVideoHeader;
        cleanVideoHeaderWithStreamIdAndDisplayName.prepend(streamId.toLocal8Bit().data());
        cleanVideoHeaderWithStreamIdAndDisplayName.prepend(streamId.size());
        cleanVideoHeaderWithStreamIdAndDisplayName.prepend(displayName.toLocal8Bit().data());
        cleanVideoHeaderWithStreamIdAndDisplayName.prepend(displayName.size());
    }

   // printTcpPacketInfo(senderAddress,streamId,roomId,displayName,cleanVideoHeaderWithStreamIdAndDisplayName);

    //If the roomId is Debug, send back the recieved header
    if(roomId == "Debug")
    {
        //Append end of header char
        cleanVideoHeaderWithStreamIdAndDisplayName.append(27);
        //Prepend number of headers
        cleanVideoHeaderWithStreamIdAndDisplayName.prepend(int(1));
        sendHeader(readSocket, cleanVideoHeaderWithStreamIdAndDisplayName, VIDEO_HEADER);
        return;
    }

    mRoomsHandler->getMutex()->lock();
    if(mRoomsHandler->getMap().count(roomId))
    {
        //TODO remove data.size
        if (mRoomsHandler->getMap().at(roomId).count(streamId) && data.size() >= 1)
        {
            /* If both the roomId and streamId exists in the map, we expect a extra byte.
            *  It will let us know what kind of action the client wants to take
            */
            QByteArray defaultSendHeader;
            defaultSendHeader.prepend(streamId.toLocal8Bit().data());
            defaultSendHeader.prepend(streamId.size());

            switch((int)data[0])
            {
            case VIDEO_HEADER:
            {
                mRoomsHandler->updateVideoHeader(roomId, streamId, cleanVideoHeader);
                sendHeaderToEveryParticipant(roomId, streamId, cleanVideoHeaderWithStreamIdAndDisplayName, VIDEO_HEADER);
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
            case KICK_PARTICIPANT:
            {
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
                qDebug() << "Could not parse header code: " << data[0] << Q_FUNC_INFO;
                break;
            }
            }

        }
        else
        {
            /* If the roomId exists, but not the streamId. We check the database if the the participant
            *  exists in the roomSession. If true, they will be added to the map aswell.
            */
            QSqlQuery q(mRoomsHandler->getDb());
            q.prepare("SELECT * FROM roomSession, user WHERE roomSession.userId = user.id AND user.streamId = :streamId");
            q.bindValue(":streamId", streamId);
            if (q.exec() && q.size() > 0)
            {
                q.next();
                mRoomsHandler->initialInsert(roomId, streamId, displayName, cleanVideoHeader, readSocket);
                SendAndRecieveFromEveryParticipantInRoom(roomId, streamId, cleanVideoHeaderWithStreamIdAndDisplayName, readSocket);
            }
            else
            {
                qDebug() << "Could not find streamID (" << streamId << ") in roomSession (Database)";
            }
        }
    }
    else
    {
        /* If the roomId does not exist in the map, we check if the participant exists in the roomSession table on the database.
         * If true, they will be added to the map aswell.
         */
        QSqlQuery q(mRoomsHandler->getDb());
        q.prepare("SELECT * FROM roomSession AS rs, user AS u WHERE rs.roomId = :roomId AND rs.userId = u.id AND u.streamId = :streamId");
        q.bindValue(":roomId", roomId);
        q.bindValue(":streamId", streamId);
        if (q.exec() && q.size() > 0)
        {
            q.next();
            mRoomsHandler->initialInsert(roomId, streamId, displayName, cleanVideoHeader, readSocket);
        }
        else
        {
            qDebug() << "Could not find the roomId (" << roomId << ") and streamId(" << streamId << ") combo in the database";
        }
    }
    mRoomsHandler->getMutex()->unlock();
}

/**
 * Prepends the headerValue to the QByteArray and sends the result using the
 * receiverSocket.
 * @param receiverSocket QTcpSocket pointer
 * @param data QByteArray
 * @param headerValue int
 */
void TcpServerHandler::sendHeader(QTcpSocket* receiverSocket, QByteArray data, const int& headerValue)
{
    if(!receiverSocket)
    {
        qDebug() << "Did not find socket " << Q_FUNC_INFO;
        return;
    }
    data.prepend(headerValue);
    int error = receiverSocket->write(data, data.size());
    if(error < 0)
    {
        qDebug() << receiverSocket->errorString() << Q_FUNC_INFO;
    }
}

/**
 * Sends the header from the participant to everyone already in the map. Then compiles
 * all the headers from everyone else and returns in to the particpant.
 * @param roomId QString
 * @param streamId QString
 * @param header QByteArray
 * @param readSocket QTcpSocket pointer belonging to the particpant
 */
void TcpServerHandler::SendAndRecieveFromEveryParticipantInRoom(const QString& roomId, const QString& streamId, QByteArray header, QTcpSocket* readSocket)
{
    QByteArray tempArr;
    const std::map<QString, std::map<QString, Participant*>> map = mRoomsHandler->getMap();
    std::map<QString, Participant*>::const_iterator i;
    //Prepend number of headers
    header.prepend(int(1));
    //Append end of header char
    header.append(27);

    //Compile all the info about the other people in the map to return back to the particpant
    for (i = map.at(roomId).begin(); i != map.at(roomId).end(); i++)
    {
        //Do not add your own info to the compiled data
        if (i->first != streamId && i->second)
        {
            QByteArray participantHeader = i->second->getVideoHeader();
            //StreamID
            participantHeader.prepend(i->first.toLocal8Bit().data());
            participantHeader.prepend(i->first.size());

            participantHeader.prepend(i->second->getDisplayName().toLocal8Bit().data());
            participantHeader.prepend(i->second->getDisplayName().size());

            tempArr.append(participantHeader);
            //Append end of header char
            tempArr.append(27);

            //Use the socket of the other people in the map and send the particpant header to them.
            QTcpSocket* qTcpSocket = i->second->getTcpSocket();
            sendHeader(qTcpSocket, header, VIDEO_HEADER);
        }
    }
    //Prepend number of headers
    tempArr.prepend(map.at(roomId).size() - 1);
    sendHeader(readSocket, tempArr, VIDEO_HEADER);
}

/**
 * Go through the map and send the header to everyone else in the map.
 * @param roomId QString
 * @param streamId QString
 * @param header QByteArray
 * @param headerCode int
 */
void TcpServerHandler::sendHeaderToEveryParticipant(const QString& roomId, const QString& streamId, QByteArray header, const int& headerCode)
{
    //Prepend number of headers
    header.prepend(int(1));
    //Append end of header char
    header.append(27);
    std::map<QString, Participant*>::const_iterator i;
    const std::map<QString, std::map<QString, Participant*>> map = mRoomsHandler->getMap();
    for (i = map.at(roomId).begin(); i != map.at(roomId).end(); i++)
    {
        if (i->first != streamId && i->second)
        {
            QTcpSocket* qTcpSocket = i->second->getTcpSocket();
            sendHeader(qTcpSocket, header, headerCode);
        }
    }
}

