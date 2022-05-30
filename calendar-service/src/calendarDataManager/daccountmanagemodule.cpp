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
#include "daccountmanagemodule.h"

#include "units.h"
#include "dalarmmanager.h"

#define UPDATEREMINDJOBTIMEINTERVAL 1000 * 60 * 10 //提醒任务更新时间间隔毫秒数（10分钟）

DAccountManageModule::DAccountManageModule(QObject *parent)
    : QObject(parent)
    , m_accountManagerDB(new DAccountManagerDataBase)
    , m_unionIDDbus(new DUnionIDDbus(unionid_service, unionid_path, QDBusConnection::sessionBus()))
{
    //新文件路径
    QString newDbPatch = getDBPath();
    QString newDB(newDbPatch + "/" + "accountmanager.db");
    m_accountManagerDB->setDBPath(newDB);
    m_accountManagerDB->dbOpen();

    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    //将云端帐户信息基本数据与本地数据合并
    unionIDDataMerging();

    //根据获取到的帐户信息创建对应的帐户服务
    foreach (auto account, m_accountList) {
        DAccountModule::Ptr accountModule = DAccountModule::Ptr(new DAccountModule(account));
        m_accountModuleMap[account->accountID()] = accountModule;
        DAccountService::Ptr accountService = DAccountService::Ptr(new DAccountService(account->dbusPath(), account->dbusInterface(), accountModule, this));
        if (!sessionBus.registerObject(accountService->getPath(), accountService->getInterface(), accountService.data(), options)) {
            qWarning() << "registerObject accountService failed:" << sessionBus.lastError();
        } else {
            m_AccountServiceMap[account->accountType()].insert(account->accountID(), accountService);
        }
    }
    m_generalSetting = m_accountManagerDB->getCalendarGeneralSettings();
}

QString DAccountManageModule::getAccountList()
{
    QString accountStr;
    DAccount::toJsonListString(m_accountList, accountStr);
    return accountStr;
}

QString DAccountManageModule::getCalendarGeneralSettings()
{
    QString cgSetStr;
    m_generalSetting = m_accountManagerDB->getCalendarGeneralSettings();
    DCalendarGeneralSettings::toJsonString(m_generalSetting, cgSetStr);
    return cgSetStr;
}

void DAccountManageModule::setCalendarGeneralSettings(const QString &cgSet)
{
    DCalendarGeneralSettings::Ptr cgSetPtr = DCalendarGeneralSettings::Ptr(new DCalendarGeneralSettings);
    DCalendarGeneralSettings::fromJsonString(cgSetPtr, cgSet);
    if (m_generalSetting != cgSetPtr) {
        m_accountManagerDB->setCalendarGeneralSettings(cgSetPtr);
        DCalendarGeneralSettings::Ptr tmpSetting = DCalendarGeneralSettings::Ptr(m_generalSetting->clone());
        m_generalSetting = cgSetPtr;
        if (tmpSetting->firstDayOfWeek() != m_generalSetting->firstDayOfWeek()) {
            emit firstDayOfWeekChange(m_generalSetting->firstDayOfWeek());
        }
        if (tmpSetting->timeShowType() != m_generalSetting->timeShowType()) {
            emit timeFormatTypeChange(m_generalSetting->timeShowType());
        }
    }
}

void DAccountManageModule::calendarOpen(const bool &isOpen)
{
    //TODO:根据日历的打开关闭设置是否自动退出
}

int DAccountManageModule::getfirstDayOfWeek()
{
    return static_cast<int>(m_generalSetting->firstDayOfWeek());
}

void DAccountManageModule::setFirstDayOfWeek(const int firstday)
{
    if (m_generalSetting->firstDayOfWeek() != firstday) {
        m_generalSetting->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(firstday));
        m_accountManagerDB->setCalendarGeneralSettings(m_generalSetting);
    }
}

int DAccountManageModule::getTimeFormatType()
{
    return static_cast<int>(m_generalSetting->timeShowType());
}

void DAccountManageModule::setTimeFormatType(const int timeType)
{
    if (m_generalSetting->timeShowType() != timeType) {
        m_generalSetting->setTimeShowType(static_cast<DCalendarGeneralSettings::TimeShowType>(timeType));
        m_accountManagerDB->setCalendarGeneralSettings(m_generalSetting);
    }
}

void DAccountManageModule::remindJob(const QString &alarmID)
{
    DRemindData::Ptr remindData = m_accountManagerDB->getRemindData(alarmID);
    DSchedule::Ptr schedule;
    QString scheduleStr = m_accountModuleMap[remindData->accountID()]->getScheduleByScheduleID(remindData->scheduleID());
    DSchedule::fromJsonString(schedule, scheduleStr);
    schedule->setDtStart(remindData->dtStart());
    schedule->setDtEnd(remindData->dtEnd());
    schedule->setRecurrenceId(remindData->recurrenceId());
    DAlarmManager alarm;
    int notifyid = alarm.remindJob(remindData, schedule);
    remindData->setNotifyid(notifyid);
    m_accountManagerDB->updateRemindInfo(remindData);
}

void DAccountManageModule::updateRemindSchedules(bool isClear)
{
    QDateTime dtStart = QDateTime::currentDateTime();
    QDateTime dtEnd = dtStart.addMSecs(UPDATEREMINDJOBTIMEINTERVAL);

    //获取未提醒的日程相关信息
    DRemindData::List noRemindList = m_accountManagerDB->getValidRemindJob();

    //获取每个账户下需要提醒的日程信息
    DRemindData::List accountRemind;
    QMap<QString, DAccountModule::Ptr>::const_iterator iter = m_accountModuleMap.constBegin();
    for (; iter != m_accountModuleMap.constEnd(); ++iter) {
        DSchedule::List scheduleList = iter.value()->getRemindScheduleList(dtStart, dtEnd);
        foreach (auto &remind, scheduleList) {
            DRemindData::Ptr remindData = DRemindData::Ptr(new DRemindData);
            remindData->setScheduleID(remind->uid());
            remindData->setAccountID(iter.key());
            remindData->setDtStart(remind->dtStart());
            remindData->setDtEnd(remind->dtEnd());
            remindData->setRecurrenceId(remind->recurrenceId());
            //由于获取的都是需要提醒的日程,所以日程的提醒列表size必然是大于0的
            remindData->setDtRemind(remind->alarms()[0]->time());
            remindData->setNotifyid(0);
            remindData->setRemindCount(0);
            accountRemind.append(remindData);
        }
    }

    //获取未提醒的稍后日程信息,由于15分钟后，1xxx后等不会修改提醒次数
    //所以需要根据提醒时间，日程id，日程重复id来判断是否是没有被触发点提醒日程
    for (int i = 0; i < noRemindList.size(); i++) {
        for (int j = accountRemind.size() - 1; j < 0; j--) {
            if (accountRemind.at(j)->scheduleID() == noRemindList.at(i)->scheduleID()
                && accountRemind.at(j)->recurrenceId() == noRemindList.at(i)->recurrenceId()
                && accountRemind.at(j)->dtRemind() == noRemindList.at(i)->dtRemind())
                //如果该日程没有被触发提醒过(创建后没有被提醒，而不是提醒后点了15分钟后等不改变提醒次数的日程)
                //则移除
                accountRemind.removeAt(j);
        }
    }

    if (isClear) {
        //清空数据库
        m_accountManagerDB->clearRemindJobDatabase();

        foreach (auto remind, noRemindList) {
            m_accountManagerDB->createRemindInfo(remind);
        }
    }
    //添加从账户中获取到的需要提醒的日程信息
    foreach (auto remind, accountRemind) {
        m_accountManagerDB->createRemindInfo(remind);
    }
    accountRemind.append(noRemindList);

    DAlarmManager alarmManager;
    //更新提醒任务
    alarmManager.updateRemind(accountRemind);
}

void DAccountManageModule::notifyMsgHanding(const QString &alarmID, const qint32 operationNum)
{
    DRemindData::Ptr remindData = m_accountManagerDB->getRemindData(alarmID);
    //如果相应的日程被删除,则不做处理
    if (remindData.isNull()) {
        return;
    }
    //如果为稍后提醒操作则需要更新对应的重复次数和提醒时间
    qint64 Minute = 60 * 1000;
    qint64 Hour = Minute * 60;
    switch (operationNum) {
    case 2: { //稍后提醒
        remindData->setRemindCount(remindData->remindCount() + 1);
        remindData->updateRemindTimeByCount();
        m_accountManagerDB->updateRemindInfo(remindData);
    } break;
    case 21: { //15min后提醒
        remindData->updateRemindTimeByMesc(15 * Minute);
        m_accountManagerDB->updateRemindInfo(remindData);
    } break;
    case 22: { //一个小时后提醒
        remindData->updateRemindTimeByMesc(Hour);
        m_accountManagerDB->updateRemindInfo(remindData);
    } break;
    case 23: { //四个小时后提醒
        remindData->updateRemindTimeByMesc(4 * Hour);
        m_accountManagerDB->updateRemindInfo(remindData);
    } break;
    case 3: { //明天提醒
        remindData->updateRemindTimeByMesc(24 * Hour);
        m_accountManagerDB->updateRemindInfo(remindData);
    } break;
    case 1: //打开日历
    case 4: { //提前1天提醒
        DSchedule::Ptr schedule;
        QString scheduleStr = m_accountModuleMap[remindData->accountID()]->getScheduleByScheduleID(remindData->scheduleID());
        DSchedule::fromJsonString(schedule, scheduleStr);
        if (schedule->allDay()) {
            schedule->setAlarmType(DSchedule::Alarm_15Min_Front);
        } else {
            schedule->setAlarmType(DSchedule::Alarm_1Day_Front);
        }
        DSchedule::toJsonString(schedule, scheduleStr);
        m_accountModuleMap[remindData->accountID()]->updateSchedule(scheduleStr);
    } break;
    default:
        //删除对应的数据
        m_accountManagerDB->deleteRemindInfoByAlarmID(alarmID);
        break;
    }
    DAlarmManager alarm;
    alarm.notifyMsgHanding(remindData, operationNum);
}

void DAccountManageModule::unionIDDataMerging()
{
    m_accountList = m_accountManagerDB->getAccountList();
    DAccount::Ptr accountUnionid = m_unionIDDbus->getUserData();
    //如果已登陆unionid
    if (!accountUnionid.isNull()) {
        DAccount::Ptr unionidDB;
        auto hasUnionid = [=, &unionidDB](const DAccount::Ptr &account) {
            if (account->accountType() == DAccount::Account_UnionID) {
                unionidDB = account;
                return true;
            }
            return false;
        };
        //如果数据库中有unionid帐户
        if (std::any_of(m_accountList.begin(), m_accountList.end(), hasUnionid)) {
            if (unionidDB->avatar() == accountUnionid->avatar() && unionidDB->accountName() == accountUnionid->accountName()
                && unionidDB->displayName() == accountUnionid->displayName()) {
            } else {
                unionidDB->setAvatar(accountUnionid->avatar());
                unionidDB->setAccountName(accountUnionid->accountName());
                unionidDB->setDisplayName(accountUnionid->displayName());
                m_accountManagerDB->updateAccountInfo(unionidDB);
            }
        }
    }
}
