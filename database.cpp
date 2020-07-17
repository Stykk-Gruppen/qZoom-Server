#include "database.h"
#include "config.cpp"

Database::Database()
{
    connectDatabase();
}

bool Database::connectDatabase()
{
   // QPluginLoader loader("/usr/local/Qt-5.15.0/plugins/sqldrivers/libqsqlmysql.so");
    QPluginLoader loader("/home/tarves/Qt/5.15.0/Src/qtbase/plugins/sqldrivers/libqsqlmysql.so");
    loader.load();
    loader.errorString();
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(dbHostName);
    db.setDatabaseName(dbDatabaseName);
    db.setUserName(dbUserName);
    db.setPassword(dbPassword);
    qDebug() << db.lastError();
    return db.open();
}
