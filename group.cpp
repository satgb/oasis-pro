#include "group.h"

Group::Group()
{

}

Group::Group(int t, QList<int> s)
{
    time = t;
    sessions = s;
}

int Group::getTime()
{
    return time;
}
