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
    std::vector<QString> vec;


    int queryCode = data[0];
    data.remove(0, 1);

    qDebug() << "queryCode:" << queryCode;

    switch (queryCode)
    {
    case 0:
    {
        //q.prepare("SELECT r.id, r.password, u.username FROM room AS r, user AS u WHERE r.host = u.id AND r.id = :roomId AND r.password = :roomPassword");

        vec = parseData(data);
        qDebug() << "RoomId:" << vec[0];
        qDebug() << "RoomPassword:" << vec[1];

        QSqlQuery q(mDb->mDb);
        q.prepare("SELECT r.id, r.password, u.username FROM room AS r, user AS u WHERE r.host = u.id AND r.id = :roomId AND r.password = :roomPassword");
        q.bindValue(":roomId", vec[0]);
        q.bindValue(":roomPassword", vec[1]);
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

std::vector<QString> SqlTcpServerHandler::parseData(QByteArray arr)
{
    std::vector<QString> vec;

    int size = arr[0];
    arr.remove(0, 1);

    for (int i = 0; i < size; i++)
    {
        int length = arr[0];
        arr.remove(0, 1);
        QByteArray tmpArr = QByteArray(arr, length);
        QString str(tmpArr);
        arr.remove(0, length);

        vec.push_back(str);
    }
    return vec;
}


QByteArray SqlTcpServerHandler::buildResponseByteArray(std::vector<QString> vec)
{
    QByteArray arr;
    for (unsigned long i = 0; i < vec.size(); i++)
    {
        arr.prepend(vec[i].size());
        arr.prepend(vec[i].toUtf8().data());
    }
    arr.prepend(vec.size());
    return arr;
}

int SqlTcpServerHandler::sendTcpPacket(QTcpSocket *socket, QByteArray arr)
{
    qDebug() << arr;
    int ret = socket->write(arr, arr.size());
    if(ret < 0)
    {
        qDebug() << socket->errorString();
    }
    return ret;
}
