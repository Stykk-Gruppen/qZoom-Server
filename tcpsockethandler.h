#ifndef TCPSOCKETHANDLER_H
#define TCPSOCKETHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class TcpSocketHandler:  public QObject
{
    Q_OBJECT
public:
    explicit TcpSocketHandler(uint16_t _port, QObject *parent = nullptr);
    void initSocket();
    void sendHeader(QHostAddress receiverAddress, QByteArray data);

private:
    QTcpSocket* mTcpSocket;
    uint16_t mPort;
    QString mReceiverAddress;

public slots:

signals:

};

#endif // TCPSOCKETHANDLER_H
