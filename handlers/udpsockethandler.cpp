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

void UdpSocketHandler::sendDatagram(const QByteArray& arr, const QHostAddress& addr)
{
    int error = mUdpSocket->writeDatagram(arr, arr.size(), QHostAddress(addr), mPortNumber);
    if(error < 0)
    {
        qDebug() << "UDP sending error: " << error << " meaning: " << mUdpSocket->error() << " " << Q_FUNC_INFO;
    }
}
/**
 * Reads and parse incoming data, if the roomId is Debug the same data gets returned back
 * to the sender. We check if the data exists in the map and sends it to the
 * correct particpants in the map.
 */
void UdpSocketHandler::readPendingDatagrams()
{
    while (mUdpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = mUdpSocket->receiveDatagram();
        QHostAddress senderAddress = datagram.senderAddress();

        QByteArray data = datagram.data();
        //If 1 byte or less is being sent to the server, go to next datagram
        if(data.size()<=1)
        {
            continue;
        }
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

        const std::map<QString, std::map<QString, Participant*>> map = mRoomsHandler->getMap();
        if(map.count(roomId))
        {
            if (map.at(roomId).count(streamId))
            {
                std::map<QString, Participant*>::const_iterator i;
                for (i = map.at(roomId).begin(); i != map.at(roomId).end(); i++)
                {
                    if(i->second && i->second->getTcpSocket() && senderAddress != i->second->getTcpSocket()->peerAddress())
                    {
                        if (i->second->getTcpSocket()->isWritable())
                        {
                            sendDatagram(returnData, i->second->getTcpSocket()->peerAddress());
                        }
                        else
                        {
                            //TODO log this error
                        }
                    }
                    else
                    {
                        //TODO log this error
                    }
                }
            }
            else
            {
                //TODO log this error
                //qDebug() << "Could not find streamId in map" << Q_FUNC_INFO;
            }
        }
        else
        {
            //TODO log this error
            //qDebug() << "Could not find roomId" << roomId << " in map, func:" << Q_FUNC_INFO;
        }
    }
}
