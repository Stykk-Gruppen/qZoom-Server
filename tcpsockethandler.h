#ifndef TCPSOCKETHANDLER_H
#define TCPSOCKETHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class TcpSocketHandler: public QObject
{
    Q_OBJECT
public:
    explicit TcpSocketHandler(QObject *parent = nullptr);;
    QTcpServer* mTcpServer;
    uint16_t mPort;
    void initTcpServer();
    QTcpSocket *tcpServerConnection = nullptr;
    void acceptTcpConnection();
    void readTcpPacket();
private:

public slots:

signals:

};

#endif // TCPSOCKETHANDLER_H
