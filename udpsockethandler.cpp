#include "udpsockethandler.h"

UdpSocketHandler::UdpSocketHandler(RoomsHandler* _roomsHandler, QObject *parent) : QObject(parent), mRoomsHandler(_roomsHandler)
{
    mPort = 1337;
    initSocket();
}

void UdpSocketHandler::initSocket()
{
    mUdpSocket = new QUdpSocket(this);

    //Connects readyRead to readPendingDatagram function,
    //which means when the socket recieves a packet the function will run.
    connect(mUdpSocket, &QUdpSocket::readyRead, this, &UdpSocketHandler::readPendingDatagrams);

    //mUdpSocket->bind(QHostAddress::LocalHost, mPort, QAbstractSocket::ShareAddress);
    mUdpSocket->bind(QHostAddress::Any, mPort, QAbstractSocket::ShareAddress);
    qDebug() << "UDP Listening on port:" << mPort;
}

int UdpSocketHandler::sendDatagram(QByteArray arr, QHostAddress addr)
{
    //qDebug() << participantAddress;
    int ret = mUdpSocket->writeDatagram(arr, arr.size(), QHostAddress(addr), mPort);
    if(ret < 0)
    {
        qDebug() << mUdpSocket->error();
    }
    return ret;
}

void UdpSocketHandler::readPendingDatagrams()
{
    while (mUdpSocket->hasPendingDatagrams())
    {
        QString firstHeader = "";
        QNetworkDatagram datagram = mUdpSocket->receiveDatagram();
        mSenderAddress = datagram.senderAddress();

        QByteArray originalData = datagram.data();
        QByteArray data = originalData;
        QByteArray returnData;

        //roomId is the first x bytes, then streamId
        //Finds roomId length, stores it and removes it from the datagram
        int roomIdLength = data[0];
        data.remove(0, 1);

        //Finds the roomId header, stores it and removes it from the datagram
        QByteArray roomIdArray = QByteArray(data, roomIdLength);
        QString roomId(roomIdArray);
        data.remove(0, roomIdLength);

        //Stores a copy of the current QByteArray, so we can return a version without the removal of streamId
        returnData = data;

        //Finds roomId length, stores it and removes it from the datagram
        int streamIdLength = data[0];
        data.remove(0, 1);

        //Finds the streamId header, stores it and removes it from the datagram
        QByteArray streamIdArray = QByteArray(data, streamIdLength);
        QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        //If the roomId is Debug, send back the same datagram
        if(roomId == "Debug")
        {
            sendDatagram(returnData, mSenderAddress);
            continue;
        }

        mRoomsHandler->mMutex->lock();
        if(mRoomsHandler->mMap.count(roomId))
        {
            if (mRoomsHandler->mMap[roomId].count(streamId))
            {
                std::map<QString, Participant*>::iterator i;
                for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
                {
                    if(mSenderAddress != i->second->getTcpSocket()->peerAddress())
                    {
                        if (i->second->getTcpSocket()->isWritable())
                        {
                            QtConcurrent::run(this, &UdpSocketHandler::sendDatagram, returnData, i->second->getTcpSocket()->peerAddress());
                            //qDebug() << "Sending from: " << mSenderAddress.toIPv4Address() << " to: " << i->second->getTcpSocket()->peerAddress().toIPv4Address() << i->first;
                        }
                        else
                        {
                            mRoomsHandler->removeParticipant(roomId, streamId);
                            sendParticipantRemovalNotice(roomId, streamId);
                        }
                       // qDebug() << "Sending from: " << mSenderAddress.toIPv4Address() << " to: " << i->second[0].toUInt() << i->first;
                    }
                }
            }
            else
            {
                //qDebug() << "Could not find streamId in map" << Q_FUNC_INFO;
            }
        }
        else
        {
            //qDebug() << "Could not find roomId" << roomId << " in map, func:" << Q_FUNC_INFO;
        }
        mRoomsHandler->mMutex->unlock();
    }
}

void UdpSocketHandler::sendParticipantRemovalNotice(QString roomId, QString streamId)
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

int UdpSocketHandler::sendTcpPacket(QTcpSocket *socket, QByteArray arr)
{
    int ret = socket->write(arr, arr.size());
    if(ret < 0)
    {
        qDebug() << socket->errorString();
    }
    return ret;
}
