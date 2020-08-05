#ifndef SQLTCPSERVERHANDLER_H
#define SQLTCPSERVERHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtSql>
#include <QSqlQuery>
#include "database.h"

class SqlTcpServerHandler : public QObject
{
    Q_OBJECT
public:
    explicit SqlTcpServerHandler(int _portNumber, Database* db, QObject *parent = nullptr);

private:
    void initTcpServer();
    void acceptTcpConnection();
    void readTcpPacket();
    int sendTcpPacket(QTcpSocket *socket, QByteArray arr);
    QByteArray buildResponseByteArray(std::vector<QString> vec);
    std::vector<QString> parseData(QByteArray arr);
    QTcpServer* mTcpServer;
    QTcpSocket* mTcpServerConnection;
    Database* mDb;
    int mPortNumber;

signals:

};

#endif // SQLTCPSERVERHANDLER_H
