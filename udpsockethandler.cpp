#include "udpsockethandler.h"

UdpSocketHandler::UdpSocketHandler(RoomsHandler* _roomsHandler, int _portNumber, QObject *parent) : QObject(parent), mRoomsHandler(_roomsHandler)
{
    mPortNumber = _portNumber;
    initSocket();
}
/**
 * Creates a new QUdpSocket objects, connects the readyRead
 * signal with our readPendingDatagrams slot and starts listening on mPortNumber.
 */
void UdpSocketHandler::initSocket()
{
    mUdpSocket = new QUdpSocket(this);

    //Connects readyRead to readPendingDatagram function,
    //which means when the socket recieves a packet the function will run.
    connect(mUdpSocket, &QUdpSocket::readyRead, this, &UdpSocketHandler::readPendingDatagrams);

    //Starts listening for UDP datagrams on mPortNumber for any address.
    mUdpSocket->bind(QHostAddress::Any, mPortNumber, QAbstractSocket::ShareAddress);
    qDebug() << "UDP Listening on port:" << mPortNumber;
}
/**
 * Sends the QByteArray arr over UDP to the QHostAddress addr
 * using the QUdpSocket
 * @param arr QByteArray
 * @param addr QHostAddress
 */
void UdpSocketHandler::sendDatagram(QByteArray arr, QHostAddress addr)
{
    int error = mUdpSocket->writeDatagram(arr, arr.size(), QHostAddress(addr), mPortNumber);
    if(error < 0)
    {
        qDebug() << "UDP sending error: " << error << " meaning: " << mUdpSocket->error() << " " << Q_FUNC_INFO;
    }
}

void UdpSocketHandler::readPendingDatagrams()
{
    while (mUdpSocket->hasPendingDatagrams())
    {
        QString firstHeader = "";
        QNetworkDatagram datagram = mUdpSocket->receiveDatagram();
        QHostAddress senderAddress = datagram.senderAddress();

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
            sendDatagram(returnData, senderAddress);
            continue;
        }

       // mRoomsHandler->mMutex->lock();//Maybe we do not need this mutex?
        if(mRoomsHandler->getMap().count(roomId))

        {
            if (mRoomsHandler->getMap()[roomId].count(streamId))
            {
                std::map<QString, Participant*>::iterator i;
                for (i = mRoomsHandler->getMap()[roomId].begin(); i != mRoomsHandler->getMap()[roomId].end(); i++)
                {
                    if(i->second && i->second->getTcpSocket() && senderAddress != i->second->getTcpSocket()->peerAddress())
                    {
                        if (i->second->getTcpSocket()->isWritable())
                        {
                            sendDatagram(returnData,i->second->getTcpSocket()->peerAddress());
                            //QtConcurrent::run(this, &UdpSocketHandler::sendDatagram, returnData, i->second->getTcpSocket()->peerAddress());
                            //qDebug() << "Sending from: " << mSenderAddress.toIPv4Address() << " to: " << i->second->getTcpSocket()->peerAddress().toIPv4Address() << i->first;
                        }
                        else
                        {
                            //mRoomsHandler->removeParticipant(roomId, streamId);
                            //sendParticipantRemovalNotice(roomId, streamId);
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
        //mRoomsHandler->mMutex->unlock();
    }
}

