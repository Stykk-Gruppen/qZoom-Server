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

        //qDebug() << data;
        //roomId is the first x bytes, then streamId
        int roomIdLength = data[0];
        data.remove(0, 1);

        //Finds the roomId header, stores it and removes it from the datagram
        QByteArray roomIdArray = QByteArray(data, roomIdLength);
        QString roomId(roomIdArray);
        //QString test = QString(roomIdArray);
        // qDebug() << "roomId String: " << test;
        /// qDebug() << roomIdArray;
        //char* roomId = roomIdArray.data();
        data.remove(0, roomIdLength);
        returnData = data;

        int streamIdLength = data[0];
        data.remove(0, 1);

        //Finds the streamId header, stores it and removes it from the datagram
        QByteArray streamIdArray = QByteArray(data, streamIdLength);
        QString streamId(streamIdArray);
        //char* streamId = streamIdArray.data();
        data.remove(0, streamIdLength);



        // qDebug() << "roomId: " << roomId;
        //qDebug() << "streamId: " << streamId;
        //roomId = "Delta";
        //streamId = "Bravo";
        //qDebug() << "roomId: " << roomId;
        //qDebug() << "streamId: " << streamId;
        //qDebug() << " datagram being returned: " <<  returnData;
        //QtConcurrent::run(this, &UdpSocketHandler::sendDatagram, returnData);
        //continue;
        //mRoomsHandler->printMap();
        if(mRoomsHandler->mMap.count(roomId))
        {
            if (mRoomsHandler->mMap[roomId].count(streamId))
            {
                mRoomsHandler->mMutex->lock();
                mRoomsHandler->updateTimestamp(roomId, streamId);
                mRoomsHandler->mMutex->unlock();
                std::map<QString, std::vector<QString>>::iterator i;
                for (i = mRoomsHandler->mMap[roomId].begin(); i != mRoomsHandler->mMap[roomId].end(); i++)
                {
                    QtConcurrent::run(this, &UdpSocketHandler::sendDatagram, returnData);
                    qDebug() << "Sending to: " << mSenderAddress << " from: " << mRoomsHandler->mMap[roomId][streamId][1] << i->first;
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

    }
}

