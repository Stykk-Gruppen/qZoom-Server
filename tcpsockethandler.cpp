#include "tcpsockethandler.h"

TcpSocketHandler::TcpSocketHandler(uint16_t _port, QObject *parent) : QObject(parent)
{
    mPort = _port;
    initSocket();
}

TcpSocketHandler::~TcpSocketHandler()
{
    mTcpSocket->close();
}

void TcpSocketHandler::initSocket()
{
    mTcpSocket = new QTcpSocket(this);

    //Connects readyRead to readPendingDatagram function,
    //which means when the socket recieves a packet the function will run.
    //connect(mTcpSocket, &QTcpSocket::readyRead, this, &TcpSocketHandler::readPendingDatagrams);

    //mUdpSocket->bind(QHostAddress::LocalHost, mPort, QAbstractSocket::ShareAddress);
    //mUdpSocket->bind(QHostAddress::Any, mPort, QAbstractSocket::ShareAddress);
    connect(mTcpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
}

void TcpSocketHandler::bytesWritten(qint64 bytes)
{
    qDebug() << "Tcpsocket wrote " << bytes << " bytes";
    mBytesWritten = bytes;
}
void TcpSocketHandler::sendHeader(QHostAddress receiverAddress, QByteArray data)
{
    mTcpSocket->connectToHost(receiverAddress, mPort);
    //qDebug() << "After ConnectToHost, addr: " << mAddress << " port: " << mPort;
    if(!mTcpSocket->waitForConnected(3000))
    {
        qDebug() << "TcpSocketError: " << mTcpSocket->errorString();
        qDebug() << " addr: " << receiverAddress << " port: " << mPort;
        return;
    }
    qDebug() << "SUCESS addr: " << receiverAddress << " port: " << mPort;
    mTcpSocket->write(data, data.length());
}
