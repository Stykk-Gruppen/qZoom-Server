#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
#include <QSqlQuery>

class Database
{
public:
    Database();
    ~Database();
    bool connectDatabase();
    QSqlDatabase mDb;
};

#endif // DATABASE_H
