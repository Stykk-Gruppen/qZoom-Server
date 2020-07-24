#include "database.h"
#include "config.cpp"
#include <iostream>

Database::Database()
{
    if (connectDatabase())
    {
        qDebug() << "The database is open";
    }
    else
    {
        qDebug() << "Failed to open the database";
    }
}

Database::~Database()
{
    mDb.close();
    QSqlDatabase::removeDatabase("QMYSQL");
}

bool Database::connectDatabase()
{
    mDb = QSqlDatabase::addDatabase("QMYSQL");
    mDb.setHostName(dbHostName);
    mDb.setDatabaseName(dbDatabaseName);
    mDb.setUserName(dbUserName);
    mDb.setPassword(dbPassword);
    //qDebug() << mDb.lastError();
    return mDb.open();
}
