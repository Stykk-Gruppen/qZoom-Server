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
    std::vector<QString> retVec;
    std::vector<QString> vec;

    int queryCode = data[0];
    data.remove(0, 1);

    qDebug() << "queryCode:" << queryCode;

    switch (queryCode)
    {
    case 0:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->mDb);
        q.prepare("SELECT r.id, r.password, u.username FROM room AS r, user AS u WHERE r.host = u.id AND r.id = :roomId AND r.password = :roomPassword");
        q.bindValue(":roomId", vec[0].toInt());
        q.bindValue(":roomPassword", vec[1]);
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(QString::number(q.value(0).toInt()));
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
        break;
    }
    case 1:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->mDb);
        q.prepare("INSERT INTO roomSession (roomId, userId) VALUES (:roomId, :userId)");
        q.bindValue(":roomId", vec[0].toInt());
        q.bindValue(":userId", vec[1].toInt());
        if (q.exec())
        {
            retVec.push_back(QString::number(q.numRowsAffected()));
            sendTcpPacket(mTcpServerConnection, buildResponseByteArray(retVec));
        }
        else
        {
            qDebug() << "Failed Query" << queryCode;
        }
        break;
    }
    case 2:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->mDb);
        q.prepare("INSERT INTO room (id, host, password) VALUES (:id, :host, :password)");
        q.bindValue(":id", vec[0].toInt());
        q.bindValue(":host", vec[1].toInt());
        q.bindValue(":password", vec[2]);
        if (q.exec())
        {
            retVec.push_back(QString::number(q.numRowsAffected()));
            sendTcpPacket(mTcpServerConnection, buildResponseByteArray(retVec));
        }
        else
        {
            qDebug() << "Failed Query" << queryCode;
        }
        break;
    }
    case 3:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->mDb);
        q.prepare("INSERT INTO user (streamId, username, password, isGuest) VALUES (substring(MD5(RAND()),1,16), :username, substring(MD5(RAND()),1,16), TRUE)");
        q.bindValue(":username", vec[0]);
        if (q.exec())
        {
            retVec.push_back(QString::number(q.numRowsAffected()));
            sendTcpPacket(mTcpServerConnection, buildResponseByteArray(retVec));
        }
        else
        {
            qDebug() << "Failed Query" << queryCode;
        }
        break;
    }
    case 4:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->mDb);
        q.prepare("SELECT id, password FROM user WHERE username = :username");
        q.bindValue(":username", vec[0]);
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(QString::number(q.value(0).toInt()));
                retVec.push_back(q.value(1).toString());
                sendTcpPacket(mTcpServerConnection, buildResponseByteArray(retVec));
            }
            else
            {
                qDebug() << "SQL: Failed select" << queryCode;
            }
        }
        else
        {
            qDebug() << "Failed Query" << queryCode;
        }
        break;
    }
    case 5:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->mDb);
        q.prepare("SELECT streamId, username, password, timeCreated FROM user WHERE id = :userId");
        q.bindValue(":userId", vec[0].toInt());
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(q.value(0).toString());
                retVec.push_back(q.value(1).toString());
                retVec.push_back(q.value(2).toString());
                sendTcpPacket(mTcpServerConnection, buildResponseByteArray(retVec));
            }
            else
            {
                qDebug() << "SQL: Failed select" << queryCode;
            }
        }
        else
        {
            qDebug() << "Failed Query" << queryCode;
        }
        break;
    }
    case 6:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->mDb);
        q.prepare("SELECT id, password FROM room WHERE host = :userId");
        q.bindValue(":userId", vec[0].toInt());
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(QString::number(q.value(0).toInt()));
                retVec.push_back(q.value(1).toString());
                sendTcpPacket(mTcpServerConnection, buildResponseByteArray(retVec));
            }
            else
            {
                qDebug() << "SQL: Failed select" << queryCode;
            }
        }
        else
        {
            qDebug() << "Failed Query" << queryCode;
        }
        break;
    }
    case 7:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->mDb);
        q.prepare("UPDATE room SET id = :roomId, password = :roomPassword WHERE host = :host");
        q.bindValue(":roomId", vec[0].toInt());
        q.bindValue(":roomPassword", vec[1]);
        q.bindValue(":host", vec[2].toInt());
        if (q.exec())
        {
            retVec.push_back(QString::number(q.numRowsAffected()));
            sendTcpPacket(mTcpServerConnection, buildResponseByteArray(retVec));
        }
        else
        {
            qDebug() << "Failed Query" << queryCode;
        }
        break;
    }
    case 8:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->mDb);
        q.prepare("SELECT streamId FROM user WHERE id = :id");
        q.bindValue(":id", vec[0].toInt());
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(q.value(0).toString());
                sendTcpPacket(mTcpServerConnection, buildResponseByteArray(retVec));
            }
            else
            {
                qDebug() << "SQL: Failed select" << queryCode;
            }
        }
        else
        {
            qDebug() << "Failed Query" << queryCode;
        }
        break;
    }
    case 9:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->mDb);
        q.prepare("SELECT id FROM user WHERE username = :username");
        q.bindValue(":username", vec[0]);
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(QString::number(q.value(0).toInt()));
                sendTcpPacket(mTcpServerConnection, buildResponseByteArray(retVec));
            }
            else
            {
                qDebug() << "SQL: Failed select" << queryCode;
            }
        }
        else
        {
            qDebug() << "Failed Query" << queryCode;
        }
        break;
    }
    default:
    {
        qDebug() << "The queryCode:" << queryCode << "Found no match.";
        break;
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
        arr.append(vec[i].size());
        arr.append(vec[i].toUtf8().data());
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
