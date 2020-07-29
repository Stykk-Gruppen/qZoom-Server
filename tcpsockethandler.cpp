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
}

void TcpSocketHandler::sendHeader(QHostAddress receiverAddress, QByteArray data)
{
    mTcpSocket->bind(receiverAddress, mPort, QAbstractSocket::ShareAddress);
    mTcpSocket->write(data, data.length());
}
