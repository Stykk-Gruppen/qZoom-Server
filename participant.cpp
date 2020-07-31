#include "participant.h"

Participant::Participant(QString _displayName, QByteArray _header, QTcpSocket* _tcpSocket)
    : mDisplayName(_displayName), mHeader(_header), mTcpSocket(_tcpSocket)
{

}

Participant::~Participant()
{
    mTcpSocket->close();
    delete mTcpSocket;
}

void Participant::setHeader(QByteArray header)
{
    mHeader = header;
}

void Participant::setDisplayName(QString displayName)
{
    mDisplayName = displayName;
}

QString Participant::getDisplayName()
{
    return mDisplayName;
}

QByteArray Participant::getHeader()
{
    return mHeader;
}

QTcpSocket* Participant::getTcpSocket()
{
    return mTcpSocket;
}
