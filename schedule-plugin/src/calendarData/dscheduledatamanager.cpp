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
#include "dscheduledatamanager.h"

#include "accountmanager.h"

DScheduleDataManager *DScheduleDataManager::getInstance()
{
    static DScheduleDataManager scheduleManager;
    return &scheduleManager;
}

QString DScheduleDataManager::createSchedule(const DSchedule::Ptr &schedule)
{
    AccountItem::Ptr account = gAccounManager->getLocalAccountItem();
    if (account.isNull()) {
        qWarning() << "account is null";
        return QString();
    }
    return account->createSchedule(schedule);
}

DSchedule::Ptr DScheduleDataManager::queryScheduleByScheduleID(const QString &scheduleID)
{
    DSchedule::Ptr schedule;
    AccountItem::Ptr account = gAccounManager->getLocalAccountItem();
    if (account.isNull()) {
        qWarning() << "account is null";
        return nullptr;
    }
    return account->getScheduleByID(scheduleID);
}

bool DScheduleDataManager::deleteScheduleByScheduleID(const QString &scheduleID)
{
    AccountItem::Ptr account = gAccounManager->getLocalAccountItem();
    if (account.isNull()) {
        qWarning() << "account is null";
        return false;
    }
    account->deleteScheduleByID(scheduleID);
    return true;
}

bool DScheduleDataManager::updateSchedule(const DSchedule::Ptr &schedule)
{
    AccountItem::Ptr account = gAccounManager->getLocalAccountItem();
    if (account.isNull()) {
        qWarning() << "account is null";
        return false;
    }
    account->updateSchedule(schedule);
    return true;
}

bool DScheduleDataManager::isFestivalSchedule(const QString &scheduleTypeID)
{
    return gAccounManager->getLocalAccountItem()->getScheduleTypeByID(scheduleTypeID)->privilege() == DScheduleType::Privilege::None;
}

DSchedule::Map DScheduleDataManager::querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params)
{
    DSchedule::Map scheduleMap;
    AccountItem::Ptr account = gAccounManager->getLocalAccountItem();
    return account.isNull() ? scheduleMap : account->querySchedulesWithParameter(params);
}

DSchedule::Map DScheduleDataManager::queryScheduleByRRule(const QDateTime &dtStart, const QDateTime &dtEnd, const DScheduleQueryPar::RRuleType &rrultTyep)
{
    DScheduleQueryPar::Ptr queryPar(new DScheduleQueryPar);
    queryPar->setDtStart(dtStart);
    queryPar->setDtEnd(dtEnd);
    queryPar->setQueryType(DScheduleQueryPar::Query_RRule);
    queryPar->setRruleType(rrultTyep);
    return querySchedulesWithParameter(queryPar);
}

DSchedule::Map DScheduleDataManager::queryScheduleByLimit(const QDateTime &dtStart, const QDateTime &dtEnd, int topNum)
{
    DScheduleQueryPar::Ptr queryPar(new DScheduleQueryPar);
    queryPar->setDtStart(dtStart);
    queryPar->setDtEnd(dtEnd);
    queryPar->setQueryType(DScheduleQueryPar::Query_Top);
    queryPar->setQueryTop(topNum);
    return querySchedulesWithParameter(queryPar);
}

DSchedule::Map DScheduleDataManager::queryScheduleBySummary(const QDateTime &dtStart, const QDateTime &dtEnd, const QString &summary)
{
    DScheduleQueryPar::Ptr queryPar(new DScheduleQueryPar);
    queryPar->setDtStart(dtStart);
    queryPar->setDtEnd(dtEnd);
    queryPar->setKey(summary);
    queryPar->setQueryType(DScheduleQueryPar::Query_None);
    return querySchedulesWithParameter(queryPar);
}

DScheduleDataManager::DScheduleDataManager()
{
    gAccounManager->resetAccount();
}
