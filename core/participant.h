#ifndef PARTICIPANT_H
#define PARTICIPANT_H
#include <QString>
#include <QTcpSocket>

class Participant
{
public:
    Participant(QString _displayName, QByteArray _header, QTcpSocket* _tcpSocket);
    ~Participant();
    void setHeader(const QByteArray& header);
    void setDisplayName(const QString& displayName);
    QString getDisplayName() const;
    QByteArray getVideoHeader() const;
    QTcpSocket* getTcpSocket();

private:
    QString mDisplayName;
    QByteArray mVideoHeader;
    QTcpSocket* mTcpSocket;

};

#endif // PARTICIPANT_H
