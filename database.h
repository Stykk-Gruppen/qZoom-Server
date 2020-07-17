#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>
#include <QSqlQuery>

class Database
{
public:
    Database();
    bool connectDatabase();
    QSqlDatabase* db;

};

#endif // DATABASE_H
