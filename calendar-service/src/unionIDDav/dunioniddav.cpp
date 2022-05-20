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
#include "dunioniddav.h"

DUnionIDDav::DUnionIDDav()
{
}

void DUnionIDDav::setAccount(const DAccount::Ptr &account)
{
}

DAccount::Ptr DUnionIDDav::getAccount() const
{
}

void DUnionIDDav::setScheduleType(const DScheduleType::List &typeList)
{
}

DScheduleType::List DUnionIDDav::getScheduleTypes()
{
}

void DUnionIDDav::uploadSchedule(const DSchedule::Ptr &schedule)
{
}

DSchedule::List DUnionIDDav::downloadSchedule()
{
}

void DUnionIDDav::deleteScheduleByPath(const QStringList &schedulePath)
{
}

void DUnionIDDav::setCalendarGeneralSettings(const DCalendarGeneralSettings &cgSet)
{
}

void DUnionIDDav::createAccountFile(const QString &fileName, const QString &content)
{
}

void DUnionIDDav::createTypeFile(const QString &fileName, const QString &content)
{
}

void DUnionIDDav::createCalendarFile(const QString &fileName, const QString &content)
{
}

void DUnionIDDav::deleteFolder(const QString &path)
{
}

void DUnionIDDav::createFolder(const QString &path)
{
}

bool DUnionIDDav::exists(const QString &path)
{
}
