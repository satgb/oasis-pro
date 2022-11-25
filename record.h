#ifndef RECORD_H
#define RECORD_H

#include <QString>

class Record
{
public:
    Record(QString type, int duration, int intensity);

    QString toString();

    QString type;
    int duration;
    int intensity;
};

#endif // RECORD_H
