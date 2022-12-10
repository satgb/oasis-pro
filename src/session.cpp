#include "session.h"

Session::Session(QString type, int duration, int intensity)
{
    this->type = type;
    this->duration = duration;
    this->intensity = intensity;
}

QString Session::toString()
{
    QString newString = "Type:" + type + ", Duration:" + QString::number(duration) + ", Intensity:" + QString::number(intensity);

    return newString;
}
