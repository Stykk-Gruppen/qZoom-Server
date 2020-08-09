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
    bool readConfigurationFile();
    void parseLine(QByteArray line);
    bool connectToDatabase();
    QStringList mConfigurationOptions {"hostname", "databasename", "username", "password"};
    QStringList mConfigurationVariables;
    QSqlDatabase mDb;
    QString findOptionVariable(int optionIndex);
};

#endif // DATABASE_H
