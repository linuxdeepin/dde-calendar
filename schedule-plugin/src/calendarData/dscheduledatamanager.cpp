// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dscheduledatamanager.h"
#include "commondef.h"

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
        qCWarning(CommonLogger) << "Failed to create schedule: Local account is null";
        return QString();
    }
    qCInfo(CommonLogger) << "Creating new schedule with ID:" << schedule->uid() << "Title:" << schedule->summary();
    return account->createSchedule(schedule);
}

DSchedule::Ptr DScheduleDataManager::queryScheduleByScheduleID(const QString &scheduleID)
{
    DSchedule::Ptr schedule;
    AccountItem::Ptr account = gAccounManager->getLocalAccountItem();
    if (account.isNull()) {
        qCWarning(CommonLogger) << "Failed to query schedule: Local account is null";
        return nullptr;
    }
    qCDebug(CommonLogger) << "Querying schedule with ID:" << scheduleID;
    return account->getScheduleByID(scheduleID);
}

bool DScheduleDataManager::deleteScheduleByScheduleID(const QString &scheduleID)
{
    AccountItem::Ptr account = gAccounManager->getLocalAccountItem();
    if (account.isNull()) {
        qCWarning(CommonLogger) << "Failed to delete schedule: Local account is null";
        return false;
    }
    qCInfo(CommonLogger) << "Deleting schedule with ID:" << scheduleID;
    account->deleteScheduleByID(scheduleID);
    return true;
}

bool DScheduleDataManager::updateSchedule(const DSchedule::Ptr &schedule)
{
    AccountItem::Ptr account = gAccounManager->getLocalAccountItem();
    if (account.isNull()) {
        qCWarning(CommonLogger) << "Failed to update schedule: Local account is null";
        return false;
    }
    qCInfo(CommonLogger) << "Updating schedule with ID:" << schedule->uid() << "Title:" << schedule->summary();
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
    qCDebug(CommonLogger) << "Querying schedules by RRule - Start:" << dtStart << "End:" << dtEnd << "RRuleType:" << rrultTyep;
    DScheduleQueryPar::Ptr queryPar(new DScheduleQueryPar);
    queryPar->setDtStart(dtStart);
    queryPar->setDtEnd(dtEnd);
    queryPar->setQueryType(DScheduleQueryPar::Query_RRule);
    queryPar->setRruleType(rrultTyep);
    return querySchedulesWithParameter(queryPar);
}

DSchedule::Map DScheduleDataManager::queryScheduleByLimit(const QDateTime &dtStart, const QDateTime &dtEnd, int topNum)
{
    qCDebug(CommonLogger) << "Querying schedules by limit - Start:" << dtStart << "End:" << dtEnd << "Top:" << topNum;
    DScheduleQueryPar::Ptr queryPar(new DScheduleQueryPar);
    queryPar->setDtStart(dtStart);
    queryPar->setDtEnd(dtEnd);
    queryPar->setQueryType(DScheduleQueryPar::Query_Top);
    queryPar->setQueryTop(topNum);
    return querySchedulesWithParameter(queryPar);
}

DSchedule::Map DScheduleDataManager::queryScheduleBySummary(const QDateTime &dtStart, const QDateTime &dtEnd, const QString &summary)
{
    qCDebug(CommonLogger) << "Querying schedules by summary - Start:" << dtStart << "End:" << dtEnd << "Summary:" << summary;
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
