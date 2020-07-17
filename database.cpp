#include "database.h"
#include "config.cpp"

Database::Database()
{
    connectDatabase();
}

bool Database::connectDatabase()
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(dbHostName);
    db.setDatabaseName(dbDatabaseName);
    db.setUserName(dbUserName);
    db.setPassword(dbPassword);
    qDebug() << db.lastError();
    return db.open();
}
