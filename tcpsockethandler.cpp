#include "tcpsockethandler.h"

TcpSocketHandler::TcpSocketHandler()
{
     initTcpServer();
     mPort = 1337;
}


void TcpSocketHandler::initTcpServer()
{
    mTcpServer = new QTcpServer();
    //connect(mTcpServer, &QTcpServer::newConnection, this, &TcpSocketHandler::acceptTcpConnection);
    mTcpServer->listen(QHostAddress::Any, mPort);

}


void TcpSocketHandler::acceptTcpConnection()
{

    tcpServerConnection = mTcpServer->nextPendingConnection();
    if (!tcpServerConnection) {
        qDebug() << "Error: got invalid pending connection!";
    }

    //connect(tcpServerConnection, &QIODevice::readyRead, this, &TcpSocketHandler::readTcpPacket);
    //connect(tcpServerConnection, &QAbstractSocket::errorOccurred, this, &SocketHandler::displayError);
   // connect(tcpServerConnection, &QTcpSocket::disconnected, tcpServerConnection, &QTcpSocket::deleteLater);

    mTcpServer->close();
}
void TcpSocketHandler::readTcpPacket()
{
       /*QByteArray block;
       QDataStream out(&block, QIODevice::WriteOnly);
       out.setVersion(QDataStream::Qt_5_10);
       qDebug() << "sending password";
       //TODO verify password and send mysql password back
       out << "dbPassword";*/
}
