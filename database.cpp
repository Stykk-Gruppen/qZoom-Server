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

/**
 * @brief Database::connectToDatabase
 * @return true if connection was successful
 */
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

/**
 * @brief Database::getDb
 * @return QSqlDatabase
 */
QSqlDatabase Database::getDb() const
{
    return mDb;
}
