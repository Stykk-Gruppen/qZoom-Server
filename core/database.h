#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
#include <QSqlQuery>

class Database
{
public:
    Database();
    ~Database();
    QSqlDatabase getDb() const;

private:
    bool connectToDatabase();
    QSqlDatabase mDb;
};

#endif // DATABASE_H
