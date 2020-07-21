#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QString>


class Participant
{
public:
    Participant(QString _streamId, QString _ipAddress);
private:
    QString mStreamId;
    QString mIpAddress;
    QString mLastTimestamp;
};

#endif // PARTICIPANT_H
