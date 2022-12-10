#include "dbmanager.h"

const QString DBManager::DATABASE_PATH = "/database/oasis.db";

DBManager::DBManager()
{
    oasisDB = QSqlDatabase::addDatabase("QSQLITE");
    oasisDB.setDatabaseName("oasis.db");

    if(!oasisDB.open())
        throw "Error: Database could not be opened";

    if (!DBInit())
        throw "Error: Database could not be initialized";
}

bool DBManager::DBInit()
{
    oasisDB.transaction();

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS profiles ( pid INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT, battery_level REAL NOT NULL);");
    query.exec("CREATE TABLE IF NOT EXISTS records ( rid INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT, pid INT NOT NULL, type TEXT NOT NULL, duration INT NOT NULL, intensity INT NOT NULL);");

    return oasisDB.commit();
}

bool DBManager::addProfile(int id, double batteryLvl)
{
    oasisDB.transaction();

    QSqlQuery query;
    query.prepare("REPLACE INTO profiles (pid, battery_level) VALUES (:pid, :battery_level);");
    query.bindValue(":pid", id);
    query.bindValue(":battery_level", batteryLvl);
    query.exec();

    return oasisDB.commit();
}

Profile* DBManager::getProfile(int id)
{
    oasisDB.transaction();

    QSqlQuery query;
    query.prepare("SELECT * FROM profiles WHERE pid=:pid");
    query.bindValue(":pid", id);
    query.exec();

    if (!oasisDB.commit())
        throw "Error: Query failed to execute";

   // profile does not exist
    if (!query.next())
    {
        addProfile(id, 100.0);
        Profile* pro = new Profile(id, 100);
        return pro;
    }

    // profile exists
    Profile* pro = new Profile(query.value(0).toInt(), query.value(1).toDouble());
    return pro;
}

bool DBManager::addSession(int id, QString type, int duration, int intensity)
{
    oasisDB.transaction();

    QSqlQuery query;
    query.prepare("INSERT INTO records (pid, type, duration, intensity) VALUES (:pid, :type, :duration, :intensity);");
    query.bindValue(":pid", id);
    query.bindValue(":type", type);
    query.bindValue(":duration", duration);
    query.bindValue(":intensity", intensity);
    query.exec();

    return oasisDB.commit();
}

QVector<Session*> DBManager::getSessions(int id)
{
    QSqlQuery query;
    QVector<Session*> qvr;
    oasisDB.transaction();

    query.prepare("SELECT type,duration,intensity FROM records WHERE pid=:pid ORDER BY rid;");
    query.bindValue(":pid", id);
    query.exec();

    while (query.next())
    {
        QString type = query.value(0).toString();
        int duration = query.value(1).toString().toInt();
        int intensity = query.value(2).toString().toInt();
        Session* s = new Session(type, duration, intensity);
        qvr.push_back(s);
    }

    return qvr;
}

bool DBManager::deleteSessions(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM records WHERE pid=:pid");
    query.bindValue(":pid", id);

    return query.exec();
}


