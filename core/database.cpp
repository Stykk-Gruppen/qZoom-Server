#include "database.h"
#include <iostream>
/**
 * @brief Database::Database
 */
Database::Database()
{
    if(!readConfigurationFile())
    {
        qDebug() << "Failed to read conf file";
        exit(-1);
    }
    if (connectToDatabase())
    {
        qDebug() << "The database is open";
    }
    else
    {
        qDebug() << "Failed to open the database";
        exit(-1);
    }
}
/**
 * Iterates the mConfigurationOptions QStringList to find the QString
 * matching option, and returns the index where it is found
 * @param optionIndex QString
 * @return int index of matching QString or -1 on failure
 */
int Database::findOptionIndex(const QString& option)
{
    for(int j= 0 ; j < mConfigurationOptions.size(); j++)
    {
         if(option.compare(mConfigurationOptions[j]) == 0)
         {
            return j;
         }
    }
    return -1;
}
/**
 * Iterates the mConfigurationVariablesIndices QList to find
 * the int matching lookupIndex, and returns the location of found
 * int.
 * @param lookupIndex
 * @return int index of matching int
 */
int Database::findRealIndex(const int& lookupIndex)
{
    for(int i = 0; i < mConfigurationVariablesIndices.size(); i++)
    {
        if(mConfigurationVariablesIndices[i] == lookupIndex)
        {
            return i;
        }
    }
    return -1;
}
/**
 * Iterates through the QByteArray and looks for the char '='
 * If found it will swap from appending bytes to the option QString
 * to the var QString. It is \b very \b strict when it comes to spaces before
 * and after '='.
 * @param line QByteArray
 */
void Database::parseLine(const QByteArray& line)
{
    QString option;
    QString var;
    bool toggle = true;
    //We start at 1 to look ahead, and append with i-1
    for(int i = 1; i <= line.size(); i++)
    {
        //Do not add newline to the string
        if(line[i-1] == '\n')
        {
            break;
        }
        if(toggle && line[i] == '=')
        {
            toggle = false;
            //Skip forward ' = '
            i= i+3;
        }
        if(toggle)
        {
            option.append(line.at(i-1));
        }
        else
        {
            var.append(line.at(i-1));
        }
    }
    mConfigurationVariables.append(var);
    mConfigurationVariablesIndices.append(findOptionIndex(option));
}
/**
 * @brief Database::readConfigurationFile
 * @return
 */
bool Database::readConfigurationFile()
{
    QFile file("/usr/local/qZoom-Server/config/qZoom-Server.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    while (!file.atEnd())
    {
        QByteArray line = file.readLine();
        parseLine(line);
    }
    return true;
}
/**
 * @brief Database::~Database
 */
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
    mDb.setHostName(mConfigurationVariables[findRealIndex(HOSTNAME)]);
    mDb.setDatabaseName(mConfigurationVariables[findRealIndex(DATABASENAME)]);
    mDb.setUserName(mConfigurationVariables[findRealIndex(USERNAME)]);
    mDb.setPassword(mConfigurationVariables[findRealIndex(PASSWORD)]);
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
