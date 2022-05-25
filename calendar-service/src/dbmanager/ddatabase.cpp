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
#include "ddatabase.h"

#include <QUuid>
#include <QFile>

DDataBase::DDataBase(QObject *parent)
    : QObject(parent)
    , m_DBPath("")
    , m_connectionName("")
{
}

DDataBase::~DDataBase()
{
}

QString DDataBase::getDBPath() const
{
    return m_DBPath;
}

void DDataBase::setDBPath(const QString &DBPath)
{
    m_DBPath = DBPath;
}

QString DDataBase::createUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString DDataBase::getConnectionName() const
{
    return m_connectionName;
}

void DDataBase::setConnectionName(const QString &connectionName)
{
    m_connectionName = connectionName;
}

void DDataBase::initDBData()
{
    createDB();
}

void DDataBase::dbOpen()
{
    QStringList cntNames = QSqlDatabase::connectionNames();
    if (cntNames.contains(getConnectionName())) {
        m_database = QSqlDatabase::database(getConnectionName());
    } else {
        m_database = QSqlDatabase::addDatabase("QSQLITE", getConnectionName());
    }
    m_database.setDatabaseName(getDBPath());
    if (!m_database.isOpen())
        m_database.open();
}

bool DDataBase::dbFileExists()
{
    QFile file;
    file.setFileName(getDBPath());
    return file.exists();
}
