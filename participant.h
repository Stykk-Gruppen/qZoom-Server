#ifndef PARTICIPANT_H
#define PARTICIPANT_H
#include <QString>
#include <QTcpSocket>

class Participant
{
public:
    Participant(QString _displayName, QByteArray _header, QTcpSocket* _tcpSocket);
    ~Participant();
    void setHeader(QByteArray header);
    void setDisplayName(QString displayName);
    QString getDisplayName();
    QByteArray getHeader();
    QTcpSocket* getTcpSocket();
private:
    QString mDisplayName;
    QByteArray mHeader;
    QTcpSocket* mTcpSocket;

};

#endif // PARTICIPANT_H
