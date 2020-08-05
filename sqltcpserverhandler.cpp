#include "sqltcpserverhandler.h"

SqlTcpServerHandler::SqlTcpServerHandler(int _portNumber, Database* _db, QObject *parent) : QObject(parent)
{
    mPortNumber = _portNumber;
    mDb = _db;
    initTcpServer();
}

void SqlTcpServerHandler::initTcpServer()
{
    mTcpServer = new QTcpServer();
    connect(mTcpServer, &QTcpServer::newConnection, this, &SqlTcpServerHandler::acceptTcpConnection);
    mTcpServer->listen(QHostAddress::Any, mPortNumber);
    qDebug() << "SQL TCP Listening on port:" << mPortNumber;
}

void SqlTcpServerHandler::acceptTcpConnection()
{
    mTcpServerConnection = mTcpServer->nextPendingConnection();
    if (!mTcpServerConnection)
    {
        qDebug() << "Error: got invalid pending connection!";
    }

    connect(mTcpServerConnection, &QIODevice::readyRead, this, &SqlTcpServerHandler::readTcpPacket);
    //connect(tcpServerConnection, &QAbstractSocket::errorOccurred, this, &SocketHandler::displayError);
}

void SqlTcpServerHandler::readTcpPacket()
{
    QTcpSocket* readSocket = qobject_cast<QTcpSocket*>(sender());
    QHostAddress senderAddress = readSocket->peerAddress();
    QByteArray data = readSocket->readAll();

    //common variables used in cases
    int length;
    std::vector<QString> retVec;


    int queryCode = data[0];
    data.remove(0, 1);

    switch (queryCode)
    {
    case 0:
    {
        //q.prepare("SELECT r.id, r.password, u.username FROM room AS r, user AS u WHERE r.host = u.id AND r.id = :roomId AND r.password = :roomPassword");

        length = data[0];
        data.remove(0, 1);

        QByteArray roomIdArray = QByteArray(data, length);
        QString roomId(roomIdArray);
        data.remove(0, length);

        length = data[0];
        data.remove(0, 1);

        QByteArray roomPasswordArray = QByteArray(data, length);
        QString roomPassword(roomPasswordArray);
        data.remove(0, length);







        QSqlQuery q(mDb->mDb);
        q.prepare("SELECT r.id, r.password, u.username FROM room AS r, user AS u WHERE r.host = u.id AND r.id = :roomId AND r.password = :roomPassword");
        q.bindValue(":roomId", roomId);
        q.bindValue(":roomPassword", roomPassword);
        if (q.exec())
        {
            if (q.size() > 0)
            {
                retVec.push_back(q.value(0).toString());
                retVec.push_back(q.value(1).toString());
                retVec.push_back(q.value(2).toString());
                sendTcpPacket(mTcpServerConnection, buildResponseByteArray(retVec));
            }
            else
            {
                qDebug() << "SQL: Did not find room";
            }
        }
        else
        {
            qDebug() << "Failed Query" << queryCode;
        }
    }
    }
}

QByteArray SqlTcpServerHandler::buildResponseByteArray(std::vector<QString> vec)
{
    QByteArray arr;
    for (unsigned long i = 0; i < vec.size(); i++)
    {
        arr.prepend(vec[i].size());
        arr.prepend(vec[i].toUtf8().data());
    }
    return arr;
}

int SqlTcpServerHandler::sendTcpPacket(QTcpSocket *socket, QByteArray arr)
{
    int ret = socket->write(arr, arr.size());
    if(ret < 0)
    {
        qDebug() << socket->errorString();
    }
    return ret;
}
