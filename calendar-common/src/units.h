/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef UNITS_H
#define UNITS_H

#include <QString>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QMap>

const QString serviceBaseName = "com.deepin.dataserver.Calendar";
const QString serviceBasePath = "/com/deepin/dataserver/Calendar";
#define accountServiceInterface "com.deepin.dataserver.Calendar.Account"

static const QMap<QString, QString> GTypeColor = {
    {"0cecca8a-291b-46e2-bb92-63a527b77d46", "#FF5E97"},
    {"10af78a1-3c25-4744-91db-6fbe5e88083b", "#FF9436"},
    {"263d6c79-32b6-4b00-bf0d-741e50a9550f", "#FFDC00"},
    {"35e70047-98bb-49b9-8ad8-02d1c942f5d0", "#5BDD80"},
    {"406fc0df-87ce-4b3f-b1bc-65d89d791dbc", "#00B99B"},
    {"5bf13e88-e99f-4975-80a8-149fe0a315e3", "#4293FF"},
    {"6cfd1459-1085-47e9-8ca6-379d47ec319a", "#5D51FF"},
    {"70080e96-e68d-40af-9cca-2f41021f6142", "#A950FF"},
    {"8ac5c8bb-55ce-4264-8b0a-5d32116cf983", "#717171"}};

QString dtToString(const QDateTime &dt);
QDateTime dtFromString(const QString &st);

QString dateToString(const QDate &date);
QDate dateFromString(const QString &date);

//获取新数据库路径
QString getDBPath();

//是否为中文环境
bool isChineseEnv();

//获取家配置目录
QString getHomeConfigPath();

//时间转换
QDateTime dtConvert(const QDateTime &datetime);

//TODO:获取系统版本(专业版，社区版等)

struct SQILIT_MUTEX {
    SQILIT_MUTEX() {}
    SQILIT_MUTEX(const SQILIT_MUTEX &) {}
    SQILIT_MUTEX &operator=(const SQILIT_MUTEX &) {return *this;}
    void lock()
    {
        m.lock();
    }
    void unlock()
    {
        m.unlock();
    }

    QMutex m;
};

struct DbPathMutex {
    SQILIT_MUTEX m;
    bool locked = false;

    bool transactionLocked = false;
    Qt::HANDLE transactionThreadId = nullptr;
    void lock()
    {
        if (transactionLocked && transactionThreadId == QThread::currentThreadId()) {
        } else {
            m.lock();
            locked = true;
        }
    }
    void unlock()
    {
        if (transactionLocked && transactionThreadId == QThread::currentThreadId()) {
        } else {
            m.unlock();
            locked = true;
        }
    }

    void transactionLock()
    {
        m.lock();
        transactionLocked = true;
        transactionThreadId = QThread::currentThreadId();
    }

    void transactionUnlock()
    {
        transactionLocked = false;
        transactionThreadId = nullptr;
        m.unlock();
    }
};

static QMap<QString, DbPathMutex> dbpathMutexMap;
static DbPathMutex &getDbMutex(QString dbpath)
{
    if (!dbpathMutexMap.contains(dbpath)) {
        dbpathMutexMap.insert(dbpath, DbPathMutex());
    }
    return dbpathMutexMap[dbpath];
}

class SqliteQuery : public QSqlQuery
{
public:
    explicit SqliteQuery(QSqlDatabase db)
        : QSqlQuery(db)
        , db(db)
    {
    }
    explicit SqliteQuery(QString connectionName)
        : QSqlQuery(QSqlDatabase::database(connectionName))
        , db(QSqlDatabase::database(connectionName))
    {
    }
    SqliteQuery(const QString &query, QSqlDatabase db)
        : QSqlQuery(query, db)
        , db(db)
    {
    }

    bool exec(QString sql)
    {
        getDbMutex(db.databaseName()).lock();
        bool f = QSqlQuery::exec(sql);
        getDbMutex(db.databaseName()).unlock();
        return f;
    }
    bool exec()
    {
        getDbMutex(db.databaseName()).lock();
        bool f = QSqlQuery::exec();
        getDbMutex(db.databaseName()).unlock();
        return f;
    }
    void transaction()
    {
        getDbMutex(db.databaseName()).transactionLock();
        db.transaction();
    }

    void commit()
    {
        db.commit();
        getDbMutex(db.databaseName()).transactionUnlock();

    }
    void rollback()
    {
        db.rollback();
        getDbMutex(db.databaseName()).transactionUnlock();

    }


    QSqlDatabase db;
};

#endif // UNITS_H
