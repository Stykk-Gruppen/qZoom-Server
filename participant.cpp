#include "participant.h"

Participant::Participant(QString _displayName, QByteArray _header, QTcpSocket* _tcpSocket)
    : mDisplayName(_displayName), mVideoHeader(_header), mTcpSocket(_tcpSocket)
{

}

Participant::~Participant()
{
    mTcpSocket->close();
    delete mTcpSocket;
}

void Participant::setHeader(QByteArray header)
{
    mVideoHeader = header;
}

void Participant::setDisplayName(QString displayName)
{
    mDisplayName = displayName;
}

QString Participant::getDisplayName() const
{
    return mDisplayName;
}

QByteArray Participant::getVideoHeader() const
{
    return mVideoHeader;
}

QTcpSocket* Participant::getTcpSocket() const
{
    return mTcpSocket;
}
