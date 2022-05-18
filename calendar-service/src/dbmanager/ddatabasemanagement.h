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
#ifndef DDATABASEMANAGEMENT_H
#define DDATABASEMANAGEMENT_H

#include <QString>
#include <QSqlDatabase>

/**
 * @brief The DDataBaseManagement class
 * 数据库初始化 ，管理数据的创建和对旧版本的数据迁移
 */
class DDataBaseManagement
{
public:
    DDataBaseManagement();

private:
    QString newDatabasePath() const;
    void setNewDatabasePath(const QString &newDatabasePath);

    QString oldDatabasePath() const;
    void setOldDatabasePath(const QString &oldDatabasePath);

    //数据库所在文件夹是否存在，若不存在则创建该文件夹
    bool databaseExists(const QString &databasePath);
    //创建帐户管理数据库
    void createAccountManagerDB(const QString &databasePath);
    //创建本地数据库
    void createLocalAccountDB(const QString &databasePath);
    //初始化帐户管理数据库
    void initAccountManagerDB(const QString &databasePath);
    //初始化本地帐户数据库
    void initLocalAccountDB(const QString &databasePath);

private:
    //新数据库路径地址
    QString m_newDatabasePath;
    //旧数据库路径地址
    QString m_oldDatabasePath;
    QString m_newDatabaseName;
    QString m_oldDatabaseName;

    QSqlDatabase m_database;
};

#endif // DDATABASEMANAGEMENT_H
