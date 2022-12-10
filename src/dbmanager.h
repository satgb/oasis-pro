#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QList>
#include <QApplication>

#include "profile.h"
#include "session.h"

class DBManager
{
public:
    static const QString DATABASE_PATH;

    DBManager();
    bool DBInit();
    bool addProfile(int id, double batteryLvl);
    Profile* getProfile(int id);
    bool addSession(int id, QString type, int duration, int intensity);
    QVector<Session*> getSessions(int id);
    bool deleteSessions(int id);

private:
    QSqlDatabase oasisDB;
};

#endif // DBMANAGER_H
