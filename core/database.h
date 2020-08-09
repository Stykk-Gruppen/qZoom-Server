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
    QList<int> mConfigurationVariablesIndices;
    QStringList mConfigurationOptions {"hostname", "databasename", "username", "password"};
    enum mConfigurationOptionsEnum {HOSTNAME, DATABASENAME, USERNAME, PASSWORD};
    QStringList mConfigurationVariables;
    QSqlDatabase mDb;
    int findOptionIndex(QString option);
    int findRealIndex(int index);
};

#endif // DATABASE_H
