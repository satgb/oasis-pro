#ifndef SESSION_H
#define SESSION_H

#include <QString>

class Session
{
public:
    Session(QString, int, int);
    QString toString();

    QString type;
    int duration;
    int intensity;
};

#endif // SESSION_H
