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
    void parseLine(const QByteArray& line);
    int findOptionIndex(const QString& option);
    int findRealIndex(const int& index);
    bool connectToDatabase();
    bool readConfigurationFile();
    QList<int> mConfigurationVariablesIndices;
    QStringList mConfigurationOptions {"hostname", "databasename", "username", "password"};
    enum mConfigurationOptionsEnum {HOSTNAME, DATABASENAME, USERNAME, PASSWORD};
    QStringList mConfigurationVariables;
    QSqlDatabase mDb;

};

#endif // DATABASE_H
