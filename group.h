#ifndef GROUP_H
#define GROUP_H

#include <QList>

#include "session.h"

class Group
{
public:
    Group(QList<int>);

    QList<int> sessions;
};

#endif // GROUP_H
