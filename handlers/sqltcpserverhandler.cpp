#include "sqltcpserverhandler.h"
/**
 * @brief SqlTcpServerHandler::SqlTcpServerHandler
 * @param _portNumber
 * @param _db
 * @param roomsHandler
 * @param parent
 */
SqlTcpServerHandler::SqlTcpServerHandler(int _portNumber, Database* _db,
                                         RoomsHandler* roomsHandler, QObject *parent) : QObject(parent)
{
    mPortNumber = _portNumber;
    mDb = _db;
    mRoomsHandler = roomsHandler;
    initTcpServer();
}
/**
 * Creates a new QTcpServer objects, connects the newConnection
 * signal with our acceptTcpConnection slot and starts listening on mPortNumber.
 */
void SqlTcpServerHandler::initTcpServer()
{
    mTcpServer = new QTcpServer();
    connect(mTcpServer, &QTcpServer::newConnection, this, &SqlTcpServerHandler::acceptTcpConnection);
    mTcpServer->listen(QHostAddress::Any, mPortNumber);
    qDebug() << "SQL TCP Listening on port:" << mPortNumber;
}

/**
 * When the QTcpServer gets a newConnection signal, it will get the corresponding QTcpSocket
 * and connect its readyRead signal with our readTcpPacket slot
 */
void SqlTcpServerHandler::acceptTcpConnection()
{
    QTcpSocket* socket = mTcpServer->nextPendingConnection();
    if (!socket)
    {
        qDebug() << "Error: got invalid pending connection!";
    }
    connect(socket, &QIODevice::readyRead, this, &SqlTcpServerHandler::readTcpPacket);
}
/**
 * Reads and parse the data sent to this socket, the first byte tells us what
 * kind of query we need to execute and what to send back to the socket.
 */
void SqlTcpServerHandler::readTcpPacket()
{
    //We get the sender object with qobject_cast, since we experienced multithreading problems using a member object
    QTcpSocket* readSocket = qobject_cast<QTcpSocket*>(sender());
    QHostAddress senderAddress = readSocket->peerAddress();
    QByteArray data = readSocket->readAll();

    //common variables used in cases
    std::vector<QString> retVec;
    std::vector<QString> vec;

    int queryCode = data[0];
    data.remove(0, 1);

    switch (queryCode)
    {
    case 0:
    {
        vec = parseData(data);
        QSqlQuery q(mDb->getDb());
        q.prepare("SELECT r.id, r.password, u.username FROM room AS r, user AS u WHERE r.host = u.id AND r.id = :roomId AND r.password = :roomPassword");
        q.bindValue(":roomId", vec[0]);
        q.bindValue(":roomPassword", vec[1]);
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(q.value(0).toString());
                retVec.push_back(q.value(1).toString());
                retVec.push_back(q.value(2).toString());
                sendTcpPacket(readSocket, buildResponseByteArray(retVec));
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
        QSqlQuery q(mDb->getDb());
        q.prepare("INSERT INTO roomSession (roomId, userId) VALUES (:roomId, :userId)");
        q.bindValue(":roomId", vec[0]);
        q.bindValue(":userId", vec[1].toInt());
        if (q.exec())
        {
            retVec.push_back(QString::number(q.numRowsAffected()));
            sendTcpPacket(readSocket, buildResponseByteArray(retVec));
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
        QSqlQuery q(mDb->getDb());
        q.prepare("INSERT INTO room (id, host, password) VALUES (:id, :host, :password)");
        q.bindValue(":id", vec[0]);
        q.bindValue(":host", vec[1].toInt());
        q.bindValue(":password", vec[2]);
        if (q.exec())
        {
            retVec.push_back(QString::number(q.numRowsAffected()));
            sendTcpPacket(readSocket, buildResponseByteArray(retVec));
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
        QSqlQuery q(mDb->getDb());
        q.prepare("INSERT INTO user (streamId, username, password, isGuest) VALUES (substring(MD5(RAND()),1,16), :username, substring(MD5(RAND()),1,16), TRUE)");
        q.bindValue(":username", vec[0]);
        if (q.exec())
        {
            retVec.push_back(QString::number(q.numRowsAffected()));
            sendTcpPacket(readSocket, buildResponseByteArray(retVec));
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
        QSqlQuery q(mDb->getDb());
        q.prepare("SELECT id, password FROM user WHERE username = :username");
        q.bindValue(":username", vec[0]);
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(QString::number(q.value(0).toInt()));
                retVec.push_back(q.value(1).toString());
                sendTcpPacket(readSocket, buildResponseByteArray(retVec));
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
        QSqlQuery q(mDb->getDb());
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
                retVec.push_back(q.value(3).toString());
                sendTcpPacket(readSocket, buildResponseByteArray(retVec));
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
        QSqlQuery q(mDb->getDb());
        q.prepare("SELECT id, password FROM room WHERE host = :userId");
        q.bindValue(":userId", vec[0].toInt());
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(q.value(0).toString());
                retVec.push_back(q.value(1).toString());
                sendTcpPacket(readSocket, buildResponseByteArray(retVec));
            }
            else
            {
                qDebug() << "SQL: Failed select" << queryCode;
                sendTcpPacket(readSocket, sendFalse());
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
        const std::map<QString, std::map<QString, Participant*>> map = mRoomsHandler->getMap();
        std::map<QString, std::map<QString, Participant*>>::const_iterator i;

        for (i = map.begin(); i != map.end(); i++)
        {
            if (i->first == vec[3])
            {
                qDebug() << "Someone tried to update an active room. Host ID:" << vec[2].toInt();
                retVec.push_back(QString::number(0));
                sendTcpPacket(readSocket, buildResponseByteArray(retVec));
                break;
            }
        }

        QSqlQuery q(mDb->getDb());
        q.prepare("UPDATE room SET id = :roomId, password = :roomPassword WHERE host = :host");
        q.bindValue(":roomId", vec[0]);
        q.bindValue(":roomPassword", vec[1]);
        q.bindValue(":host", vec[2].toInt());
        if (q.exec())
        {
            retVec.push_back(QString::number(q.numRowsAffected()));
            sendTcpPacket(readSocket, buildResponseByteArray(retVec));
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
        QSqlQuery q(mDb->getDb());
        q.prepare("SELECT streamId FROM user WHERE id = :id");
        q.bindValue(":id", vec[0].toInt());
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(q.value(0).toString());
                sendTcpPacket(readSocket, buildResponseByteArray(retVec));
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
        QSqlQuery q(mDb->getDb());
        q.prepare("SELECT id FROM user WHERE username = :username");
        q.bindValue(":username", vec[0]);
        if (q.exec())
        {
            if (q.size() > 0)
            {
                q.next();
                retVec.push_back(QString::number(q.value(0).toInt()));
                sendTcpPacket(readSocket, buildResponseByteArray(retVec));
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
/**
 * If a select query fails, we need to send a single 0 byte back to the
 * client to let it know.
 * @return QByteArray containing a single 0 int
 */
QByteArray SqlTcpServerHandler::sendFalse() const
{
    QByteArray arr;
    arr.prepend(int(0));
    return arr;
}
/**
 * This function will parse the data to be used in the queries. The
 * data should always be a int first, telling us how long the string is,
 * followed by the string.
 * @param arr QByteArray containing the data
 * @return std::vector<QString> with all the strings parsed
 */
std::vector<QString> SqlTcpServerHandler::parseData(QByteArray arr) const
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

/**
 * This function will iterate through the vector and build a
 * QByteArray with the strings and their lengths.
 * @param vec std::vector<QString>
 * @return QByteArray with the compiled data
 */
QByteArray SqlTcpServerHandler::buildResponseByteArray(std::vector<QString> vec) const
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
/**
 * Sends the QByteArray arr using the QTcpSocket object
 * @param socket QTcpSocket pointer
 * @param arr QByteArray data to send
 */
void SqlTcpServerHandler::sendTcpPacket(QTcpSocket *socket, QByteArray arr)
{
    int error = socket->write(arr, arr.size());
    if(error < 0)
    {
        qDebug() << socket->errorString();
    }
}
