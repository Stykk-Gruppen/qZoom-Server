#include "udpsockethandler.h"

UdpSocketHandler::UdpSocketHandler(RoomsHandler* _roomsHandler, QObject *parent) : QObject(parent), mRoomsHandler(_roomsHandler)
{
    mPort = 1337;
    initSocket();
    //connect(mTimer, SIGNAL(timeout()), this, SLOT(removeOldParticipantsFromQMap()));
}

void UdpSocketHandler::initSocket()
{
    mUdpSocket = new QUdpSocket(this);

    //Connects readyRead to readPendingDatagram function,
    //which means when the socket recieves a packet the function will run.
    connect(mUdpSocket, &QUdpSocket::readyRead, this, &UdpSocketHandler::readPendingDatagrams);

    //mUdpSocket->bind(QHostAddress::LocalHost, mPort, QAbstractSocket::ShareAddress);
    mUdpSocket->bind(QHostAddress::Any, mPort, QAbstractSocket::ShareAddress);
}

int UdpSocketHandler::sendDatagram(QByteArray arr)
{
    //qDebug() << participantAddress;
    int ret = mUdpSocket->writeDatagram(arr, arr.size(), mSenderAddress, mPort);
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
            sendDatagram(returnData);
            continue;
        }

        mRoomsHandler->mMutex->lock();
        if(mRoomsHandler->mMap.count(roomId))
        {
            if (mRoomsHandler->mMap[roomId].count(streamId))
            {
                mRoomsHandler->updateTimestamp(roomId, streamId);
                std::map<QString, std::vector<QByteArray>>::iterator i;
                for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
                {
                    QtConcurrent::run(this, &UdpSocketHandler::sendDatagram, returnData);
                    qDebug() << "Sending to: " << mSenderAddress << " from: " << mRoomsHandler->mMap[roomId][streamId][1] << i->first;
                }
            }
            else
            {
                qDebug() << "Could not find streamId in map" << Q_FUNC_INFO;
            }
        }
        else
        {
            qDebug() << "Could not find roomId" << roomId << " in map, func:" << Q_FUNC_INFO;
        }
        mRoomsHandler->mMutex->unlock();
    }
}

