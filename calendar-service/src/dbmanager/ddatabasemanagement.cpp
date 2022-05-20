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
#include "ddatabasemanagement.h"

#include "ddatabase.h"
#include "vcalformat.h"
#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

const QString localDBName = "localAccount.db";

DDataBaseManagement::DDataBaseManagement()
    : m_newDatabaseName("accountmanager.db")
    , m_oldDatabaseName("scheduler.db")
{
    //旧文件路径
    QString oldDbPatch = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/.config/deepin/dde-daemon/calendar");
    setOldDatabasePath(oldDbPatch);
    //新文件路径
    QString newDbPatch = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/.config/deepin/dde-calendar-service");
    setNewDatabasePath(newDbPatch);
    QString newDB(newDatabasePath() + "/" + m_newDatabaseName);
    //如果新数据库不存在
    if (!databaseExists(newDatabasePath())) {
        QString localAccountDB(newDatabasePath() + "/" + localDBName);
        DAccountManagerDataBase accountManager;
        accountManager.setDBPath(newDB);
        accountManager.setLoaclDB(localDBName);
        accountManager.initDBData();

        DAccount::List accountList = accountManager.getAccountList();
        for (auto account : accountList) {
            if (account->accountType() == DAccount::Account_Local) {
                DAccountDataBase localDB(account);
                localDB.setDBPath(localAccountDB);
                localDB.initDBData();
            }
        }

        //判断是否存在旧的数据库
        QString oldDB(oldDatabasePath() + "/" + m_oldDatabaseName);
        if (databaseExists(oldDB)) {
            //TODO:对数据进行迁移
        }
    }
}

QString DDataBaseManagement::newDatabasePath() const
{
    return m_newDatabasePath;
}

void DDataBaseManagement::setNewDatabasePath(const QString &newDatabasePath)
{
    m_newDatabasePath = newDatabasePath;
}

QString DDataBaseManagement::oldDatabasePath() const
{
    return m_oldDatabasePath;
}

void DDataBaseManagement::setOldDatabasePath(const QString &oldDatabasePath)
{
    m_oldDatabasePath = oldDatabasePath;
}

bool DDataBaseManagement::databaseExists(const QString &databasePath)
{
    QDir dir;
    bool exist = false;
    if (dir.exists(databasePath)) {
        exist = true;
    } else {
        dir.mkpath(databasePath);
    }
    return exist;
}
