#ifndef GROUP_H
#define GROUP_H

#include <QList>

#include "session.h"

class Group
{
public:
    Group();
    Group(int, QList<int>);
    int getTime();
    //void getSessions();
    QList<int> sessions;
    int time;
private:
};

#endif // GROUP_H
