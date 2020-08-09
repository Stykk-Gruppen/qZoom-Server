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
/**
 * @brief Participant::setHeader
 * @param header QByteArray
 */
void Participant::setHeader(QByteArray header)
{
    mVideoHeader = header;
}
/**
 * @brief Participant::setDisplayName
 * @param displayName QString
 */
void Participant::setDisplayName(QString displayName)
{
    mDisplayName = displayName;
}
/**
 * @brief Participant::getDisplayName
 * @return QString
 */
QString Participant::getDisplayName() const
{
    return mDisplayName;
}
/**
 * @brief Participant::getVideoHeader
 * @return QByteArray
 */
QByteArray Participant::getVideoHeader() const
{
    return mVideoHeader;
}
/**
 * @brief Participant::getTcpSocket
 * @return QTcpSocket pointer
 */
QTcpSocket* Participant::getTcpSocket() const
{
    return mTcpSocket;
}
