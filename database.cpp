#include "database.h"
#include "config.cpp"
#include <iostream>

Database::Database()
{
    if (connectToDatabase())
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

bool Database::connectToDatabase()
{
    mDb = QSqlDatabase::addDatabase("QMYSQL");
    mDb.setHostName(dbHostName);
    mDb.setDatabaseName(dbDatabaseName);
    mDb.setUserName(dbUserName);
    mDb.setPassword(dbPassword);
    //qDebug() << mDb.lastError();
    return mDb.open();
}

QSqlDatabase Database::getDb() const
{
    return mDb;
}
