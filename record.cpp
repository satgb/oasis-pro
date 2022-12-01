#include "record.h"

Record::Record(QString type, int duration, int intensity)
{
    this->type = type;
    this->duration = duration;
    this->intensity = intensity;
}

QString Record::toString()
{
    QString newString = "Type:" + type + ", Duration:" + QString::number(duration) + ", Intensity:" + QString::number(intensity);

    return newString;
}
