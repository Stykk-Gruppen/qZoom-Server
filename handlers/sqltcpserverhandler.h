#ifndef SQLTCPSERVERHANDLER_H
#define SQLTCPSERVERHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtSql>
#include <QSqlQuery>
#include "core/database.h"
#include "handlers/roomshandler.h"

class SqlTcpServerHandler : public QObject
{
    Q_OBJECT
public:
    explicit SqlTcpServerHandler(int _portNumber, Database* db, RoomsHandler* roomsHandler,
                                 QObject *parent = nullptr);

private:
    void initTcpServer();
    void acceptTcpConnection();
    void readTcpPacket();
    void sendTcpPacket(QTcpSocket *socket, QByteArray arr);
    int mPortNumber;
    QByteArray buildResponseByteArray(std::vector<QString> vec) const;
    QByteArray sendFalse() const;
    std::vector<QString> parseData(QByteArray arr) const;
    RoomsHandler* mRoomsHandler;
    QTcpServer* mTcpServer;
    Database* mDb;

signals:

};

#endif // SQLTCPSERVERHANDLER_H
