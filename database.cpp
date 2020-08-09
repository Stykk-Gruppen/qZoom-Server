#include "database.h"
#include <iostream>

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
    }
}

QString Database::findOptionVariable(int optionIndex)
{
    for(int j=0;j<mConfigurationOptions.size();j++)
    {
         if(mConfigurationOptions[optionIndex].compare(mConfigurationOptions[j])==0)
         {
            return mConfigurationVariables[j];
         }
    }
    return "";
}

void Database::parseLine(QByteArray line)
{
    QString option;
    QString var;
    bool toggle = true;
    //We start at 1 to look ahead, and append with i-1
    for(int i=1;i<=line.size();i++)
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
            i=i+3;
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
}

bool Database::readConfigurationFile()
{
    QFile file("test.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        parseLine(line);
    }
    return true;
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
    mDb.setHostName(findOptionVariable(0));
    mDb.setDatabaseName(findOptionVariable(1));
    mDb.setUserName(findOptionVariable(2));
    mDb.setPassword(findOptionVariable(3));
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
