// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountmodule.h"
#include "units.h"
#include "dscheduletype.h"
#include "dschedulequerypar.h"
#include "memorycalendar.h"
#include "lunarandfestival/lunardateinfo.h"
#include "lunarandfestival/lunarmanager.h"
#include "dbus/dbusuiopenschedule.h"
#include "dalarmmanager.h"
#include "syncfilemanage.h"
#include "csystemdtimercontrol.h"
#include "dsyncdatafactory.h"
#include "unionIDDav/dunioniddav.h"
#include "ddatasyncbase.h"
#include "dbusnotify.h"

#include <QDir>
#include <QFile>

#define UPDATEREMINDJOBTIMEINTERVAL 1000 * 60 * 10 //提醒任务更新时间间隔毫秒数（10分钟）

DAccountModule::DAccountModule(const DAccount::Ptr &account, QObject *parent)
    : QObject(parent)
    , m_account(account)
    , m_accountDB(new DAccountDataBase(account))
    , m_alarm(new DAlarmManager)
    , m_dataSync(DSyncDataFactory::createDataSync(m_account))
{
    qCDebug(ServiceLogger) << "DAccountModule constructor called for account:" << account->accountID();
    QString newDbPath = getDBPath();
    m_accountDB->setDBPath(newDbPath + "/" + account->dbName());
    m_accountDB->initDBData();
    m_accountDB->getAccountInfo(m_account);

    //关联打开日历界面
    connect(m_alarm.data(), &DAlarmManager::signalCallOpenCalendarUI, this, &DAccountModule::slotOpenCalendar);
    if (m_dataSync != nullptr) {
        connect(m_dataSync, &DDataSyncBase::signalSyncState, this, &DAccountModule::slotSyncState);
        connect(m_dataSync, &DDataSyncBase::signalUpdate, this, &DAccountModule::slotDateUpdate);
    }
    //关联关闭提醒弹窗
    connect(this, &DAccountModule::signalCloseNotification, m_alarm->getdbusnotify(), &DBusNotify::closeNotification);
    qCDebug(ServiceLogger) << "DAccountModule constructed for account:" << account->accountID();
}

DAccountModule::~DAccountModule()
{
    qCDebug(ServiceLogger) << "DAccountModule destructor called for account:" << (m_account ? m_account->accountID() : "unknown");
    if (m_dataSync != nullptr) {
        delete m_dataSync;
        m_dataSync = nullptr;
    }
}

QString DAccountModule::getAccountInfo()
{
    // qCDebug(ServiceLogger) << "Getting account info for:" << m_account->accountID();
    QString accountInfo;
    DAccount::toJsonString(m_account, accountInfo);
    return accountInfo;
}

bool DAccountModule::getExpand()
{
    // qCDebug(ServiceLogger) << "Getting expand state for account:" << m_account->accountID();
    return m_account->isExpandDisplay();
}

void DAccountModule::setExpand(const bool &isExpand)
{
    qCDebug(ServiceLogger) << "Setting expand state to" << isExpand << "for account:" << m_account->accountID();
    if (m_account->isExpandDisplay() != isExpand) {
        m_account->setIsExpandDisplay(isExpand);
        m_accountDB->updateAccountInfo();
    }
}

int DAccountModule::getAccountState()
{
    // qCDebug(ServiceLogger) << "Getting account state for:" << m_account->accountID();
    return int(m_account->accountState());
}

void DAccountModule::setAccountState(const int accountState)
{
    qCDebug(ServiceLogger) << "Setting account state to" << accountState << "for account:" << m_account->accountID();
    if (int(m_account->accountState()) != accountState) {
        m_account->setAccountState(static_cast<DAccount::AccountState>(accountState));
        m_accountDB->updateAccountInfo();
    }
}

int DAccountModule::getSyncState()
{
    // qCDebug(ServiceLogger) << "Getting sync state for account:" << m_account->accountID();
    return m_account->syncState();
}

QString DAccountModule::getSyncFreq()
{
    // qCDebug(ServiceLogger) << "Getting sync frequency for account:" << m_account->accountID();
    return DAccount::syncFreqToJsonString(m_account);
}

void DAccountModule::setSyncFreq(const QString &freq)
{
    qCDebug(ServiceLogger) << "Setting sync frequency for account:" << m_account->accountID() << "freq:" << freq;
    DAccount::SyncFreqType syncType = m_account->syncFreq();
    DAccount::syncFreqFromJsonString(m_account, freq);
    if (syncType == m_account->syncFreq()) {
        qCDebug(ServiceLogger) << "Sync frequency unchanged for account:" << m_account->accountID();
        return;
    }
    m_accountDB->updateAccountInfo();
    downloadTaskhanding(1);
}

QString DAccountModule::getScheduleTypeList()
{
    qCDebug(ServiceLogger) << "Getting schedule type list for account:" << m_account->accountID();
    DScheduleType::List typeList = m_accountDB->getScheduleTypeList();
    //排序
    std::sort(typeList.begin(), typeList.end());
    QString typeListStr;
    DScheduleType::toJsonListString(typeList, typeListStr);
    return typeListStr;
}

QString DAccountModule::getScheduleTypeByID(const QString &typeID)
{
    qCDebug(ServiceLogger) << "Getting schedule type by ID:" << typeID << "for account:" << m_account->accountID();
    DScheduleType::Ptr scheduleType = m_accountDB->getScheduleTypeByID(typeID);
    QString typeStr;
    DScheduleType::toJsonString(scheduleType, typeStr);
    return typeStr;
}

QString DAccountModule::createScheduleType(const QString &typeInfo)
{
    qCDebug(ServiceLogger) << "Creating schedule type for account:" << m_account->accountID() << "with info:" << typeInfo;
    DScheduleType::Ptr scheduleType;
    DScheduleType::fromJsonString(scheduleType, typeInfo);
    //如果颜色为用户自定义则需要在数据库中记录
    if (scheduleType->typeColor().colorID() == "") {
        scheduleType->setColorID(DDataBase::createUuid());
        DTypeColor::Ptr typeColor(new DTypeColor(scheduleType->typeColor()));
        typeColor->setPrivilege(DTypeColor::PriUser);
        m_accountDB->addTypeColor(typeColor);
        qCDebug(ServiceLogger) << "Added custom color for schedule type:" << typeColor->colorID();
        
        //添加创建颜色任务
        if (m_account->isNetWorkAccount()) {
            DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
            uploadTask->setTaskType(DUploadTaskData::TaskType::Create);
            uploadTask->setTaskObject(DUploadTaskData::Task_Color);
            uploadTask->setObjectId(typeColor->colorID());
            m_accountDB->addUploadTask(uploadTask);
            qCDebug(ServiceLogger) << "Added color upload task for network account";
        }
    }
    //设置创建时间
    scheduleType->setDtCreate(QDateTime::currentDateTime());
    QString scheduleTypeID = m_accountDB->createScheduleType(scheduleType);
    //如果为网络日程则需要上传任务
    if (m_account->isNetWorkAccount()) {
        DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
        uploadTask->setTaskType(DUploadTaskData::TaskType::Create);
        uploadTask->setTaskObject(DUploadTaskData::Task_ScheduleType);
        uploadTask->setObjectId(scheduleTypeID);
        m_accountDB->addUploadTask(uploadTask);
        qCDebug(ServiceLogger) << "Added schedule type upload task for network account";
        //开启上传任务
        uploadNetWorkAccountData();
    }
    emit signalScheduleTypeUpdate();
    return scheduleTypeID;
}

bool DAccountModule::deleteScheduleTypeByID(const QString &typeID)
{
    qCDebug(ServiceLogger) << "Deleting schedule type by ID:" << typeID << "for account:" << m_account->accountID();
    //如果日程类型被使用需要删除对应到日程信息
    if (m_accountDB->scheduleTypeByUsed(typeID)) {
        qCDebug(ServiceLogger) << "Schedule type" << typeID << "is in use, deleting associated schedules.";
        QStringList scheduleIDList = m_accountDB->getScheduleIDListByTypeID(typeID);
        foreach (auto scheduleID, scheduleIDList) {
            closeNotification(scheduleID);
            //添加删除日程任务
            if (m_account->isNetWorkAccount()) {
                DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
                uploadTask->setTaskType(DUploadTaskData::TaskType::Delete);
                uploadTask->setTaskObject(DUploadTaskData::Task_Schedule);
                uploadTask->setObjectId(scheduleID);
                m_accountDB->addUploadTask(uploadTask);
            }
        }
        //更新提醒任务
        updateRemindSchedules(false);
        m_accountDB->deleteSchedulesByScheduleTypeID(typeID, !m_account->isNetWorkAccount());
        emit signalScheduleUpdate();
    }
    DScheduleType::Ptr scheduleType = m_accountDB->getScheduleTypeByID(typeID);
    //根据帐户是否为网络帐户需要添加任务列表中,并设置弱删除
    if (m_account->isNetWorkAccount()) {
        QStringList scheduleIDList = m_accountDB->getScheduleIDListByTypeID(typeID);
        //弱删除
        m_accountDB->deleteScheduleTypeByID(typeID);

        //发送操作内容给任务列表
        DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
        uploadTask->setTaskType(DUploadTaskData::TaskType::Delete);
        uploadTask->setTaskObject(DUploadTaskData::Task_ScheduleType);
        uploadTask->setObjectId(typeID);
        m_accountDB->addUploadTask(uploadTask);

        //如果颜色不为系统类型则删除
        if(scheduleType->typeColor().privilege() != DTypeColor::PriSystem){
            m_accountDB->deleteTypeColor(scheduleType->typeColor().colorID());
            DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
            uploadTask->setTaskType(DUploadTaskData::TaskType::Delete);
            uploadTask->setTaskObject(DUploadTaskData::Task_Color);
            uploadTask->setObjectId(scheduleType->typeColor().colorID());
            m_accountDB->addUploadTask(uploadTask);
        }

        //开启上传任务
        uploadNetWorkAccountData();
    } else {
        m_accountDB->deleteScheduleTypeByID(typeID, 1);
        if(scheduleType->typeColor().privilege() != DTypeColor::PriSystem){
            m_accountDB->deleteTypeColor(scheduleType->typeColor().colorID());
        }
    }
    if (scheduleType.isNull()) {
        qCWarning(ServiceLogger) << "scheduleType isNull, typeID:" << typeID;
        return false;
    }

    //如果为用户颜色则删除颜色
    if (scheduleType->typeColor().privilege() > 1) {
        m_accountDB->deleteTypeColor(scheduleType->typeColor().colorID());
    }
    emit signalScheduleTypeUpdate();
    return true;
}

bool DAccountModule::scheduleTypeByUsed(const QString &typeID)
{
    // qCDebug(ServiceLogger) << "Checking if schedule type" << typeID << "is used for account:" << m_account->accountID();
    return m_accountDB->scheduleTypeByUsed(typeID);
}

bool DAccountModule::updateScheduleType(const QString &typeInfo)
{
    qCDebug(ServiceLogger) << "Updating schedule type for account:" << m_account->accountID() << "with info:" << typeInfo;
    DScheduleType::Ptr scheduleType;
    DScheduleType::fromJsonString(scheduleType, typeInfo);
    DScheduleType::Ptr oldScheduleType = m_accountDB->getScheduleTypeByID(scheduleType->typeID());
    //如果颜色有改动
    if (oldScheduleType.isNull()) {
        qCWarning(ServiceLogger) << "get oldScheduleType error,typeID:" << scheduleType->typeID();
    } else {
        qCDebug(ServiceLogger) << "oldScheduleType:" << oldScheduleType->typeID();
        if (oldScheduleType->typeColor() != scheduleType->typeColor()) {
            if (!oldScheduleType->typeColor().isSysColorInfo()) {
                m_accountDB->deleteTypeColor(oldScheduleType->typeColor().colorID());
                //添加删除颜色任务
                if (m_account->isNetWorkAccount()) {
                    DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
                    uploadTask->setTaskType(DUploadTaskData::TaskType::Delete);
                    uploadTask->setTaskObject(DUploadTaskData::Task_Color);
                    uploadTask->setObjectId(oldScheduleType->typeColor().colorID());
                    m_accountDB->addUploadTask(uploadTask);
                }
            }
            if (!scheduleType->typeColor().isSysColorInfo()) {
                DTypeColor::Ptr typeColor(new DTypeColor(scheduleType->typeColor()));
                typeColor->setPrivilege(DTypeColor::PriUser);
                m_accountDB->addTypeColor(typeColor);
                scheduleType->setColorID(typeColor->colorID());
                //添加创建颜色任务
                if (m_account->isNetWorkAccount()) {
                    DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
//                    uploadTask->setTaskType(DUploadTaskData::TaskType::Delete);
                    uploadTask->setTaskType(DUploadTaskData::TaskType::Create);
                    uploadTask->setTaskObject(DUploadTaskData::Task_Color);
                    uploadTask->setObjectId(typeColor->colorID());
                    m_accountDB->addUploadTask(uploadTask);
                }
            }
        }
    }
    scheduleType->setDtUpdate(QDateTime::currentDateTime());
    bool isSucc = m_accountDB->updateScheduleType(scheduleType);

    if (isSucc) {
        qCDebug(ServiceLogger) << "Schedule type updated successfully";
        if (m_account->isNetWorkAccount()) {
            DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
            uploadTask->setTaskType(DUploadTaskData::TaskType::Modify);
            uploadTask->setTaskObject(DUploadTaskData::Task_ScheduleType);
            uploadTask->setObjectId(scheduleType->typeID());
            m_accountDB->addUploadTask(uploadTask);
            //开启上传任务
            uploadNetWorkAccountData();
        }
        //如果不是修改显示状态则发送日程类型改变信号
        if (oldScheduleType->showState() == scheduleType->showState()) {
            qCDebug(ServiceLogger) << "Schedule type show state unchanged";
            emit signalScheduleTypeUpdate();
        } else {
            qCDebug(ServiceLogger) << "Schedule type show state changed";
            //日程改变信号
            emit signalScheduleUpdate();
        }
    }
    qCDebug(ServiceLogger) << "Schedule type update result:" << isSucc;
    return isSucc;
}

QString DAccountModule::createSchedule(const QString &scheduleInfo)
{
    qCDebug(ServiceLogger) << "Creating schedule for account:" << m_account->accountID() << "with info:" << scheduleInfo;
    DSchedule::Ptr schedule;
    DSchedule::fromJsonString(schedule, scheduleInfo);
    schedule->setCreated(QDateTime::currentDateTime());

    QString scheduleID = m_accountDB->createSchedule(schedule);
    //根据是否为网络帐户判断是否需要更新任务列表
    if (m_account->isNetWorkAccount()) {
        qCDebug(ServiceLogger) << "Schedule type is network account";
        DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
        uploadTask->setTaskType(DUploadTaskData::TaskType::Create);
        uploadTask->setTaskObject(DUploadTaskData::Task_Schedule);
        uploadTask->setObjectId(scheduleID);
        m_accountDB->addUploadTask(uploadTask);
        qCDebug(ServiceLogger) << "Added schedule upload task for network account";
        //开启上传任务
        uploadNetWorkAccountData();
    }
    //根据是否为提醒日程更新提醒任务
    if (schedule->alarms().size() > 0) {
        qCDebug(ServiceLogger) << "Updating reminders for new schedule:" << scheduleID;
        updateRemindSchedules(false);
    }
    //发送日程更新信号
    emit signalScheduleUpdate();
    return scheduleID;
}

bool DAccountModule::updateSchedule(const QString &scheduleInfo)
{
    qCDebug(ServiceLogger) << "Updating schedule for account:" << m_account->accountID() << "with info:" << scheduleInfo;
    //根据是否为提醒日程更新提醒任务
    DSchedule::Ptr schedule;
    DSchedule::fromJsonString(schedule, scheduleInfo);
    DSchedule::Ptr oldSchedule = m_accountDB->getScheduleByScheduleID(schedule->uid());
    schedule->setLastModified(QDateTime::currentDateTime());
    schedule->setRevision(schedule->revision() + 1);

    //如果旧日程为提醒日程
    if (oldSchedule->alarms().size() > 0) {
        qCDebug(ServiceLogger) << "Old schedule had alarms, checking for reminders to update/delete.";
        //根据日程ID获取提醒日程信息
        DRemindData::List remindList = m_accountDB->getRemindByScheduleID(schedule->schedulingID());

        DRemindData::List deleteRemind;

        //如果是重复日程且重复规则不一样
        if ((schedule->recurs() || oldSchedule->recurs()) && *schedule->recurrence() != *oldSchedule->recurrence()) {
            QDateTime dtEnd = schedule->dtStart();
            for (int i = remindList.size() - 1; i >= 0; --i) {
                dtEnd = dtEnd > remindList.at(i)->dtStart() ? dtEnd : remindList.at(i)->dtStart();
            }

            if (remindList.size() > 0) {
                //获取新日程的重复时间
                QList<QDateTime> dtList = schedule->recurrence()->timesInInterval(schedule->dtStart(), dtEnd);
                foreach (auto remind, remindList) {
                    //如果生成的开始时间列表内不包含提醒日程的开始时间，则表示该重复日程被删除
                    if (!dtList.contains(remind->dtStart())) {
                        //如果改日程已经提醒，且通知弹框未操作
                        if (remind->notifyid() > 0) {
                            emit signalCloseNotification(static_cast<quint64>(remind->notifyid()));
                            deleteRemind.append(remind);
                        } else if (remind->dtRemind() > QDateTime::currentDateTime()) {
                            //删除没有触发的提醒日程
                            deleteRemind.append(remind);
                        }
                    }
                }
            }
        } else {
            qCDebug(ServiceLogger) << "Not a repeat schedule";
            //不是重复日程
            if (remindList.size() > 0 && remindList.at(0)->notifyid() > 0) {
                deleteRemind.append(remindList.at(0));
            }
        }
        for (int i = 0; i < deleteRemind.size(); ++i) {
            m_accountDB->deleteRemindInfoByAlarmID(deleteRemind.at(i)->alarmID());
        }
    }

    bool ok = m_accountDB->updateSchedule(schedule);

    //如果存在提醒
    if (oldSchedule->alarms().size() > 0 || schedule->alarms().size() > 0) {
        updateRemindSchedules(false);
    }

    emit signalScheduleUpdate();
    //根据是否为网络帐户判断是否需要更新任务列表
    if (m_account->isNetWorkAccount()) {
        qCDebug(ServiceLogger) << "Schedule is network account";
        DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
        uploadTask->setTaskType(DUploadTaskData::TaskType::Modify);
        uploadTask->setTaskObject(DUploadTaskData::Task_Schedule);
        uploadTask->setObjectId(schedule->uid());
        m_accountDB->addUploadTask(uploadTask);
        //开启上传任务
        uploadNetWorkAccountData();
    }
    return ok;
}

QString DAccountModule::getScheduleByScheduleID(const QString &scheduleID)
{
    qCDebug(ServiceLogger) << "Getting schedule by ID:" << scheduleID << "for account:" << m_account->accountID();
    DSchedule::Ptr schedule = m_accountDB->getScheduleByScheduleID(scheduleID);
    QString scheduleStr;
    DSchedule::toJsonString(schedule, scheduleStr);
    return scheduleStr;
}

bool DAccountModule::deleteScheduleByScheduleID(const QString &scheduleID)
{
    qCDebug(ServiceLogger) << "Deleting schedule by ID:" << scheduleID << "for account:" << m_account->accountID();
    //根据是否为网络判断是否需要弱删除
    bool isOK;
    DSchedule::Ptr schedule = m_accountDB->getScheduleByScheduleID(scheduleID);
    if (m_account->isNetWorkAccount()) {
        qCDebug(ServiceLogger) << "Schedule is network account";
        isOK = m_accountDB->deleteScheduleByScheduleID(scheduleID);
        //更新上传任务表
        DUploadTaskData::Ptr uploadTask(new DUploadTaskData);
        uploadTask->setTaskType(DUploadTaskData::TaskType::Delete);
        uploadTask->setTaskObject(DUploadTaskData::Task_Schedule);
        uploadTask->setObjectId(scheduleID);
        m_accountDB->addUploadTask(uploadTask);
        //开启任务
        uploadNetWorkAccountData();
    } else {
        qCDebug(ServiceLogger) << "Schedule is not network account";
        isOK = m_accountDB->deleteScheduleByScheduleID(scheduleID, 1);
    }
    //如果删除的是提醒日程
    if (schedule->alarms().size() > 0) {
        qCDebug(ServiceLogger) << "Schedule has alarms, closing notification and updating remind schedules";
        //关闭提醒消息和对应的通知弹框
        closeNotification(scheduleID);
        updateRemindSchedules(false);
    }
    emit signalScheduleUpdate();
    return isOK;
}

QString DAccountModule::querySchedulesWithParameter(const QString &params)
{
    qCDebug(ServiceLogger) << "Querying schedules for account:" << m_account->accountID() << "with params:" << params;
    DScheduleQueryPar::Ptr queryPar = DScheduleQueryPar::fromJsonString(params);
    if (queryPar.isNull()) {
        qCWarning(ServiceLogger) << "Failed to parse query parameters.";
        return QString();
    }
    DSchedule::List scheduleList;
    if (queryPar->queryType() == DScheduleQueryPar::Query_RRule) {
        qCDebug(ServiceLogger) << "Querying schedules by RRule";
        scheduleList = m_accountDB->querySchedulesByRRule(queryPar->key(), queryPar->rruleType());
    } else if (queryPar->queryType() == DScheduleQueryPar::Query_ScheduleID) {
        qCDebug(ServiceLogger) << "Querying schedule by ScheduleID";
        DSchedule::Ptr schedule = m_accountDB->getScheduleByScheduleID(queryPar->key());
        if (schedule.isNull()) {
            return QString();
        }
        scheduleList.append(schedule);
    } else {
        qCDebug(ServiceLogger) << "Querying schedules by key";
        scheduleList = m_accountDB->querySchedulesByKey(queryPar->key());
    }

    bool extend = queryPar->queryType() == DScheduleQueryPar::Query_None;
    //根据条件判断是否需要添加节假日日程
    if (isChineseEnv() && extend && m_account->accountType() == DAccount::Account_Local) {
        qCDebug(ServiceLogger) << "Querying festival schedules";
        scheduleList.append(getFestivalSchedule(queryPar->dtStart(), queryPar->dtEnd(), queryPar->key()));
    }

    qCDebug(ServiceLogger) << "Querying schedules result:" << scheduleList.size();
    return DSchedule::toListString(params, scheduleList);
}

DSchedule::List DAccountModule::getRemindScheduleList(const QDateTime &dtStart, const QDateTime &dtEnd)
{
    qCDebug(ServiceLogger) << "Getting remind schedule list for account:" << m_account->accountID() << "from" << dtStart << "to" << dtEnd;
    //获取范围内需要提醒的日程信息
    DSchedule::List scheduleList;
    //当前最多提前一周提醒。所以结束时间+8天
    QMap<QDate, DSchedule::List> scheduleMap = getScheduleTimesOn(dtStart, dtEnd.addDays(8), m_accountDB->getRemindSchedule(), false);
    QMap<QDate, DSchedule::List>::const_iterator iter = scheduleMap.constBegin();
    for (; iter != scheduleMap.constEnd(); ++iter) {
        foreach (auto schedule, iter.value()) {
            if (schedule->alarms().size() > 0
                    && schedule->alarms()[0]->time() >= dtStart && schedule->alarms()[0]->time() <= dtEnd) {
                scheduleList.append(schedule);
            }
        }
    }
    qCDebug(ServiceLogger) << "Remind schedule list size:" << scheduleList.size();
    return scheduleList;
}

QString DAccountModule::getSysColors()
{
    qCDebug(ServiceLogger) << "Getting system colors for account:" << m_account->accountID();
    DTypeColor::List colorList = m_accountDB->getSysColor();
    std::sort(colorList.begin(), colorList.end());
    return DTypeColor::toJsonString(colorList);
}

DAccount::Ptr DAccountModule::account() const
{
    // qCDebug(ServiceLogger) << "Getting account object.";
    return m_account;
}

void DAccountModule::updateRemindSchedules(bool isClear)
{
    qCDebug(ServiceLogger) << "Updating remind schedules for account:" << m_account->accountID() << "isClear:" << isClear;
    //因为全天的当前提醒日程会在开始时间延后9小时提醒
    QDateTime dtCurrent = QDateTime::currentDateTime();
    QDateTime dtStart = dtCurrent.addSecs(-9*60*60);
    QDateTime dtEnd = dtCurrent.addMSecs(UPDATEREMINDJOBTIMEINTERVAL);

    //获取未提醒的日程相关信息
    DRemindData::List noRemindList = m_accountDB->getValidRemindJob();

    //获取每个账户下需要提醒的日程信息
    DRemindData::List accountRemind;

    DSchedule::List scheduleList = getRemindScheduleList(dtStart, dtEnd);
    foreach (auto &remind, scheduleList) {
        DRemindData::Ptr remindData = DRemindData::Ptr(new DRemindData);
        remindData->setScheduleID(remind->uid());
        remindData->setAccountID(m_account->accountID());
        remindData->setDtStart(remind->dtStart());
        remindData->setDtEnd(remind->dtEnd());
        remindData->setRecurrenceId(remind->recurrenceId());
        //由于获取的都是需要提醒的日程,所以日程的提醒列表size必然是大于0的
        remindData->setDtRemind(remind->alarms()[0]->time());
        remindData->setNotifyid(0);
        remindData->setRemindCount(0);
        accountRemind.append(remindData);
    }

    //获取未提醒的稍后日程信息,由于15分钟后，1xxx后等不会修改提醒次数
    //所以需要根据提醒时间，日程id，日程重复id来判断是否是没有被触发点提醒日程
    for (int i = 0; i < noRemindList.size(); i++) {
        for (int j = accountRemind.size() - 1; j >= 0; j--) {
            if (accountRemind.at(j)->scheduleID() == noRemindList.at(i)->scheduleID()
                    && accountRemind.at(j)->recurrenceId() == noRemindList.at(i)->recurrenceId()
                    && accountRemind.at(j)->dtRemind() == noRemindList.at(i)->dtRemind())
                //如果该日程没有被触发提醒过(创建后没有被提醒，而不是提醒后点了15分钟后等不改变提醒次数的日程)
                //则移除
                accountRemind.removeAt(j);
        }
    }

    if (isClear) {
        qCDebug(ServiceLogger) << "Clearing remind job database";
        //清空数据库
        m_accountDB->clearRemindJobDatabase();

        foreach (auto remind, noRemindList) {
            m_accountDB->createRemindInfo(remind);
        }
    }
    //添加从账户中获取到的需要提醒的日程信息
    foreach (auto remind, accountRemind) {
        m_accountDB->createRemindInfo(remind);
    }
    accountRemind.append(noRemindList);
    //更新提醒任务
    m_alarm->updateRemind(accountRemind);
}

void DAccountModule::notifyMsgHanding(const QString &alarmID, const qint32 operationNum)
{
    qCDebug(ServiceLogger) << "Handling notification message for alarm:" << alarmID << "operation:" << operationNum;
    DRemindData::Ptr remindData = m_accountDB->getRemindData(alarmID);
    remindData->setAccountID(m_account->accountID());
    //如果相应的日程被删除,则不做处理
    if (remindData.isNull()) {
        qCDebug(ServiceLogger) << "Remind data is null, skipping notification handling";
        return;
    }
    //如果为稍后提醒操作则需要更新对应的重复次数和提醒时间
    qint64 Minute = 60 * 1000;
    qint64 Hour = Minute * 60;
    switch (operationNum) {
    case 2: { //稍后提醒
        remindData->setRemindCount(remindData->remindCount() + 1);
        remindData->updateRemindTimeByCount();
        m_accountDB->updateRemindInfo(remindData);
    } break;
    case 21: { //15min后提醒
        remindData->updateRemindTimeByMesc(15 * Minute);
        m_accountDB->updateRemindInfo(remindData);
    } break;
    case 22: { //一个小时后提醒
        remindData->updateRemindTimeByMesc(Hour);
        m_accountDB->updateRemindInfo(remindData);
    } break;
    case 23: { //四个小时后提醒
        remindData->updateRemindTimeByMesc(4 * Hour);
        m_accountDB->updateRemindInfo(remindData);
    } break;
    case 3: { //明天提醒
        remindData->updateRemindTimeByMesc(24 * Hour);
        m_accountDB->updateRemindInfo(remindData);
    } break;
    case 1: { //打开日历
    } break;
    case 4: { //提前1天提醒
        DSchedule::Ptr schedule = getScheduleByRemind(remindData);
        //TODO:如果是重复日程是否需要修改所有日程的提醒？还是只修改此日程？
        if (schedule->allDay()) {
            schedule->setAlarmType(DSchedule::Alarm_15Min_Front);
        } else {
            schedule->setAlarmType(DSchedule::Alarm_1Day_Front);
        }
        m_accountDB->updateSchedule(schedule);
        //删除对应提醒任务数据
        m_accountDB->deleteRemindInfoByAlarmID(alarmID);
        emit signalScheduleUpdate();
    } break;
    default:
        //删除对应提醒任务数据
        m_accountDB->deleteRemindInfoByAlarmID(alarmID);
        break;
    }

    m_alarm->notifyMsgHanding(remindData, operationNum);
}

void DAccountModule::remindJob(const QString &alarmID)
{
    qCDebug(ServiceLogger) << "Executing remind job for alarm:" << alarmID;
    DRemindData::Ptr remindData = m_accountDB->getRemindData(alarmID);
    remindData->setAccountID(m_account->accountID());
    DSchedule::Ptr schedule = getScheduleByRemind(remindData);

    int notifyid = m_alarm->remindJob(remindData, schedule);
    remindData->setNotifyid(notifyid);
    m_accountDB->updateRemindInfo(remindData);
}

void DAccountModule::accountDownload()
{
    qCDebug(ServiceLogger) << "Account download triggered for account:" << m_account->accountID();
    if (m_dataSync != nullptr) {
        qCInfo(ServiceLogger) << "Starting data download for account:" << m_account->accountID();
        m_dataSync->syncData(this->account()->accountID(), this->account()->accountName(), 
                            (int)this->account()->accountState(), 
                            DDataSyncBase::Sync_Upload | DDataSyncBase::Sync_Download);
    } else {
        qCWarning(ServiceLogger) << "Cannot download data - sync not available for account:" 
                                << m_account->accountID();
    }
}

void DAccountModule::uploadNetWorkAccountData()
{
    qCDebug(ServiceLogger) << "Upload network account data triggered for account:" << m_account->accountID();
    if (m_dataSync != nullptr) {
        qCInfo(ServiceLogger) << "Starting data upload for account:" << m_account->accountID();
        m_dataSync->syncData(this->account()->accountID(), this->account()->accountName(), 
                            (int)this->account()->accountState(), 
                            DDataSyncBase::Sync_Upload);
    } else {
        qCWarning(ServiceLogger) << "Cannot upload data - sync not available for account:" 
                                << m_account->accountID();
    }
}

QString DAccountModule::getDtLastUpdate()
{
    // qCDebug(ServiceLogger) << "Getting last update time for account:" << m_account->accountID();
    return dtToString(m_account->dtLastSync());
}

void DAccountModule::removeDB()
{
    qCDebug(ServiceLogger) << "Removing database for account:" << m_account->accountID();
    m_accountDB->removeDB();
    //如果为uid帐户退出则清空目录下所有关于uid的数据库文件
    //解决在某些条件下数据库没有被移除的问题（自测未发现）
    if(account()->accountType() == DAccount::Type::Account_UnionID){
        QString dbPatch = getHomeConfigPath().append(QString("/deepin/dde-calendar-service/"));
        QDir dir(dbPatch);
        if (dir.exists()) {
            qCDebug(ServiceLogger) << "Removing all UID account DB files from:" << dbPatch;
            QStringList filters;
            filters << QString("account_uid_*");
            dir.setFilter(QDir::Files | QDir::NoSymLinks);
            dir.setNameFilters(filters);
            for (uint i = 0; i < dir.count(); ++i) {
                QFile::remove(dbPatch + dir[i]);
            }
        }
    }
}

QMap<QDate, DSchedule::List> DAccountModule::getScheduleTimesOn(const QDateTime &dtStart, const QDateTime &dtEnd, const DSchedule::List &scheduleList, bool extend)
{
    qCDebug(ServiceLogger) << "Getting scheduled times on for" << scheduleList.size() << "schedules, from" << dtStart << "to" << dtEnd << "extend:" << extend;
    QMap<QDate, DSchedule::List> m_scheduleMap;
    //相差多少天
    int days = static_cast<int>(dtStart.daysTo(dtEnd));
    for (int i = 0; i <= days; ++i) {
        DSchedule::List scheduleList;
        m_scheduleMap[dtStart.addDays(i).date()] = scheduleList;
    }

    foreach (auto &schedule, scheduleList) {
        //获取日程的开始结束时间差
        qint64 interval = schedule->dtStart().secsTo(schedule->dtEnd());
        //如果存在重复日程
        if (schedule->recurs()) {
            //如果为农历日程
            if (schedule->lunnar()) {
                //农历重复日程计算
                LunarDateInfo lunardate(schedule->recurrence()->defaultRRuleConst(), interval);

                QMap<int, QDate> ruleStartDate = lunardate.getRRuleStartDate(dtStart.date(), dtEnd.date(), schedule->dtStart().date());

                QDateTime recurDateTime;
                recurDateTime.setTime(schedule->dtStart().time());
                QDateTime copyEnd;
                QMap<int, QDate>::ConstIterator iter = ruleStartDate.constBegin();
                for (; iter != ruleStartDate.constEnd(); iter++) {
                    recurDateTime.setDate(iter.value());
                    //如果在忽略时间列表中,则忽略
                    if (schedule->recurrence()->exDateTimes().contains(recurDateTime))
                        continue;
                    copyEnd = recurDateTime.addSecs(interval);
                    DSchedule::Ptr newSchedule = DSchedule::Ptr(new DSchedule(*schedule.data()));
                    newSchedule->setDtStart(recurDateTime);
                    newSchedule->setDtEnd(copyEnd);
                    //只有重复日程设置RecurrenceId
                    if (schedule->dtStart() != recurDateTime) {
                        newSchedule->setRecurrenceId(recurDateTime);
                    }

                    if (extend) {
                        //需要扩展的天数
                        int extenddays = static_cast<int>(recurDateTime.daysTo(copyEnd));
                        for (int i = 0; i <= extenddays; ++i) {
                            m_scheduleMap[recurDateTime.date().addDays(i)].append(newSchedule);
                        }

                    } else {
                        m_scheduleMap[recurDateTime.date()].append(newSchedule);
                    }
                }
            } else {
                //非农历日程
                extendRecurrence(m_scheduleMap, schedule, dtStart, dtEnd, extend);
            }
        } else {
            //普通日程
            //如果在查询时间范围内
            QDateTime queryDtStart = dtStart;
            //如果日程为全天日程，则查询的开始时间设置为0点，因为全天日程的开始和结束时间都是0点
            if(schedule->allDay()){
                queryDtStart.setTime(QTime(0,0,0));
            }
            if (!(schedule->dtEnd() < queryDtStart || schedule->dtStart() > dtEnd)) {
                if (extend && schedule->isMultiDay()) {
                    //需要扩展的天数
                    int extenddays = static_cast<int>(schedule->dtStart().daysTo(schedule->dtEnd()));
                    for (int i = 0; i <= extenddays; ++i) {
                        //如果扩展的日期在查询范围内则添加，否则退出
                        if(m_scheduleMap.contains(schedule->dtStart().date().addDays(i))){
                            m_scheduleMap[schedule->dtStart().date().addDays(i)].append(schedule);
                        } else {
                            break;
                        }
                    }
                } else {
                    m_scheduleMap[schedule->dtStart().date()].append(schedule);
                }
            }
        }
    }
    qCDebug(ServiceLogger) << "Getting scheduled times on result:" << m_scheduleMap.size();
    return m_scheduleMap;
}

DSchedule::List DAccountModule::getFestivalSchedule(const QDateTime &dtStart, const QDateTime &dtEnd, const QString &key)
{
    qCDebug(ServiceLogger) << "Getting festival schedules from" << dtStart << "to" << dtEnd << "with key:" << key;
    QList<stDayFestival> festivaldays = GetFestivalsInRange(dtStart, dtEnd);
    if (!key.isEmpty()) {
        festivaldays = FilterDayFestival(festivaldays, key);
    }
    DSchedule::List scheduleList;
    foreach (stDayFestival festivalDay, festivaldays) {
        foreach (QString festival, festivalDay.Festivals) {
            if (!festival.isEmpty()) {
                DSchedule::Ptr schedule = DSchedule::Ptr(new DSchedule);
                schedule->setSummary(festival);
                schedule->setAllDay(true);
                schedule->setDtStart(QDateTime(QDate(festivalDay.date.date()), QTime(0, 0)));
                schedule->setDtEnd(QDateTime(QDate(festivalDay.date.date()), QTime(23, 59)));
                //设置UID为开始时间+内容
                schedule->setUid(dtToString(schedule->dtStart()) + schedule->summary());
                schedule->setScheduleTypeID(m_accountDB->getFestivalTypeID());
                scheduleList.append(schedule);
            }
        }
    }
    qCDebug(ServiceLogger) << "Getting festival schedules result:" << scheduleList.size();
    return scheduleList;
}

void DAccountModule::extendRecurrence(DSchedule::Map &scheduleMap, const DSchedule::Ptr &schedule, const QDateTime &dtStart, const QDateTime &dtEnd, bool extend)
{
    // qCDebug(ServiceLogger) << "Extending recurrence for schedule:" << schedule->summary() << "from" << dtStart << "to" << dtEnd << "extend:" << extend;
    QDateTime queryDtStart = dtStart;
    //如果日程为全天日程，则查询的开始时间设置为0点，因为全天日程的开始和结束时间都是0点
    if(schedule->allDay()){
        qCDebug(ServiceLogger) << "Schedule is all day";
        queryDtStart.setTime(QTime(0,0,0));
    }
    if (schedule->recurs()) {
        qCDebug(ServiceLogger) << "Schedule is recursive";
        //获取日程的开始结束时间差
        qint64 interval = schedule->dtStart().secsTo(schedule->dtEnd());
        QList<QDateTime> dtList = schedule->recurrence()->timesInInterval(queryDtStart, dtEnd);
        foreach (auto &dt, dtList) {
            QDateTime scheduleDtEnd = dt.addSecs(interval);
            DSchedule::Ptr newSchedule = DSchedule::Ptr(schedule->clone());
            newSchedule->setDtStart(dt);
            newSchedule->setDtEnd(scheduleDtEnd);

            //只有重复日程设置RecurrenceId
            if (schedule->dtStart() != dt) {
                newSchedule->setRecurrenceId(dt);
            }
            if (extend) {
                //需要扩展的天数
                int extenddays = static_cast<int>(dt.daysTo(scheduleDtEnd));
                for (int i = 0; i <= extenddays; ++i) {
                    scheduleMap[dt.date().addDays(i)].append(newSchedule);
                }
            } else {
                scheduleMap[dt.date()].append(newSchedule);
            }
        }
    } else {
        qCDebug(ServiceLogger) << "Schedule is not recursive";
        if (!(schedule->dtStart() > dtEnd || schedule->dtEnd() < queryDtStart)) {
            scheduleMap[schedule->dtStart().date()].append(schedule);
        }
    }
}

void DAccountModule::closeNotification(const QString &scheduleId)
{
    qCDebug(ServiceLogger) << "Closing notifications for schedule ID:" << scheduleId;
    //根据日程ID获取提醒日程信息
    DRemindData::List remindList = m_accountDB->getRemindByScheduleID(scheduleId);
    foreach (auto remind, remindList) {
        m_accountDB->deleteRemindInfoByAlarmID(remind->alarmID());
        emit signalCloseNotification(static_cast<quint32>(remind->notifyid()));
    }
}

DSchedule::Ptr DAccountModule::getScheduleByRemind(const DRemindData::Ptr &remindData)
{
    // qCDebug(ServiceLogger) << "Getting schedule by reminder data for schedule ID:" << remindData->scheduleID();
    DSchedule::Ptr schedule = m_accountDB->getScheduleByScheduleID(remindData->scheduleID());
    if (!schedule.isNull() && schedule->dtStart() != remindData->dtStart()) {
        schedule->setDtStart(remindData->dtStart());
        schedule->setDtEnd(remindData->dtEnd());
        schedule->setRecurrenceId(remindData->recurrenceId());
    }
    return schedule;
}

void DAccountModule::downloadTaskhanding(int index)
{
    qCDebug(ServiceLogger) << "Handling download task for account:" << m_account->accountID() << "index:" << index;
    //index: 0:帐户登录 1：修改同步频率 2：帐户登出
    CSystemdTimerControl sysControl;
    if (index > 0) {
        qCDebug(ServiceLogger) << "Stopping download task for account:" << m_account->accountID();
        //修改和停止都需要停止定时任务
        sysControl.stopDownloadTask(m_account->accountID());
    }
    if (index != 2) {
        qCDebug(ServiceLogger) << "Starting download task for account:" << m_account->accountID();
        //如果帐户刚刚登录开启定时任务
        //设置同步频率
        if (m_account->isNetWorkAccount()) {
            int sync = -1;
            switch (m_account->syncFreq()) {
            case DAccount::SyncFreq_15Mins:
                sync = 15;
                break;
            case DAccount::SyncFreq_30Mins:
                sync = 30;
                break;
            case DAccount::SyncFreq_1hour:
                sync = 60;
                break;
            case DAccount::SyncFreq_24hour:
                sync = 24 * 60;
                break;
            default:
                break;
            }
            if (sync > 0) {
                sysControl.startDownloadTask(m_account->accountID(), sync);
            }
        }
    }
}

void DAccountModule::uploadTaskHanding(int open)
{
    qCDebug(ServiceLogger) << "Handling upload task for account:" << m_account->accountID() << "open:" << open;
    CSystemdTimerControl sysControl;
    if (1 == open) {
        qCDebug(ServiceLogger) << "Starting upload task for account:" << m_account->accountID();
        sysControl.startUploadTask(15);
        return;
    }
    if (0 == open) {
        qCDebug(ServiceLogger) << "Stopping upload task for account:" << m_account->accountID();
        //TODO:需要考虑多个帐户情况
        sysControl.stopUploadTask();
        return;
    }
}

void DAccountModule::slotOpenCalendar(const QString &alarmID)
{
    qCDebug(ServiceLogger) << "Slot: Open calendar requested for alarm ID:" << alarmID;
    DbusUIOpenSchedule openCalendar("com.deepin.Calendar",
                                    "/com/deepin/Calendar",
                                    QDBusConnection::sessionBus(),
                                    this);
    DRemindData::Ptr remindData = m_accountDB->getRemindData(alarmID);
    if (remindData.isNull()) {
        qCWarning(ServiceLogger) << "No corresponding reminder ID found";
        return;
    }
    DSchedule::Ptr schedule = getScheduleByRemind(remindData);
    QString scheduleStr;
    DSchedule::toJsonString(schedule, scheduleStr);
    openCalendar.OpenSchedule(scheduleStr);
    //删除对应提醒任务数据
    m_accountDB->deleteRemindInfoByAlarmID(alarmID);
}

void DAccountModule::slotSyncState(const int syncState)
{
    qCDebug(ServiceLogger) << "Slot: Sync state changed for account:" << m_account->accountID() << "New state:" << syncState;
    m_account->setDtLastSync(QDateTime::currentDateTime());
    switch (syncState) {
    case 0:
        //执行正常
        m_account->setSyncState(DAccount::Sync_Normal);
        m_accountDB->updateAccountInfo();
        qCDebug(ServiceLogger) << "Sync completed successfully for account:" << m_account->accountID();
        //同步成功后更新提醒任务
        updateRemindSchedules(false);
        break;
    case 7506:
        //网络异常
        m_account->setSyncState(DAccount::Sync_NetworkAnomaly);
        qCWarning(ServiceLogger) << "Network anomaly during sync for account:" << m_account->accountID();
        break;
    case 7508:
        //存储已满
        m_account->setSyncState(DAccount::Sync_StorageFull);
        qCWarning(ServiceLogger) << "Storage full during sync for account:" << m_account->accountID();
        break;
    default:
        qCWarning(ServiceLogger) << "Server exception during sync for account:" << m_account->accountID() 
                                << "Error code:" << syncState;
        //默认服务器异常
        m_account->setSyncState(DAccount::Sync_ServerException);
        break;
    }

    emit signalDtLastUpdate();
    //错误处理
    emit signalSyncState();
}

void DAccountModule::slotDateUpdate(const DDataSyncBase::UpdateTypes updateType)
{
    qCDebug(ServiceLogger) << "Slot: Data update notification received. Type:" << updateType;
    if (updateType.testFlag(DDataSyncBase::Update_Setting)) {
        qCDebug(ServiceLogger) << "Updating setting change";
        emit signalSettingChange();
    }
    if (updateType.testFlag(DDataSyncBase::Update_Schedule)) {
        qCDebug(ServiceLogger) << "Updating schedule change";
        emit signalScheduleUpdate();
    }
    if (updateType.testFlag(DDataSyncBase::Update_ScheduleType)) {
        qCDebug(ServiceLogger) << "Updating schedule type change";
        emit signalScheduleTypeUpdate();
    }
}

// 导入日程
bool DAccountModule::importSchedule(const QString &icsFilePath, const QString &typeID, const bool cleanExists)
{
    qCDebug(ServiceLogger) << "Importing schedules from" << icsFilePath << "into type" << typeID << "Clean exists:" << cleanExists;
    KCalendarCore::ICalFormat icalformat;
    QTimeZone timezone = QDateTime::currentDateTime().timeZone();
    KCalendarCore::MemoryCalendar::Ptr cal(new KCalendarCore::MemoryCalendar(timezone));
    auto ok = icalformat.load(cal, icsFilePath);
    if (!ok) {
        qCWarning(ServiceLogger) << "Failed to load ICS file:" << icsFilePath;
        return false;
    }
    auto events = cal->events();
    if (cleanExists) {
        qCDebug(ServiceLogger) << "Cleaning existing schedules for type:" << typeID;
        ok = m_accountDB->deleteSchedulesByScheduleTypeID(typeID, true);
        if (!ok) {
            qCWarning(ServiceLogger) << "Failed to clean existing schedules for type:" << typeID;
            return false;
        }
    };
    int importedCount = 0;
    foreach (auto event, events) {
        auto data = event.data();
        auto sch = DSchedule::Ptr(new DSchedule(*data));
        sch->setScheduleTypeID(typeID);
        QString scheduleID = m_accountDB->createSchedule(sch);
        if (!scheduleID.isEmpty()) {
            importedCount++;
            qCDebug(ServiceLogger) << "Successfully imported schedule:" << data->summary()
                                   << "Start:" << data->dtStart().toString()
                                   << "ID:" << scheduleID;
        } else {
            qCWarning(ServiceLogger) << "Failed to import schedule:" << data->summary()
                                     << "Start:" << data->dtStart().toString();
        }
    };
    qCInfo(ServiceLogger) << "Import completed: " << importedCount << " out of " << events.size() << " events successfully imported for type " << typeID;
    if (importedCount == 0 && !events.isEmpty()) {
        qCWarning(ServiceLogger) << "WARNING: No schedules were successfully imported, despite having " << events.size() << " events in ICS file: " << icsFilePath;
    }
    // 发送日程更新信号
    emit signalScheduleUpdate();
    return importedCount > 0 || events.isEmpty(); // 如果没有事件或者至少导入了一个事件，则认为成功
}

// 导出日程
bool DAccountModule::exportSchedule(const QString &icsFilePath, const QString &typeID)
{
    qCDebug(ServiceLogger) << "Exporting schedules to" << icsFilePath << "from type" << typeID;
    auto typeInfo = m_accountDB->getScheduleTypeByID(typeID);
    KCalendarCore::MemoryCalendar::Ptr cal(new KCalendarCore::MemoryCalendar(nullptr));
    // 附加扩展信息
    cal->setNonKDECustomProperty("X-DDE-CALENDAR-TYPE-ID", typeID);
    cal->setNonKDECustomProperty("X-DDE-CALENDAR-TYPE-NAME", typeInfo->displayName());
    cal->setNonKDECustomProperty("X-DDE-CALENDAR-TYPE-COLOR", typeInfo->getColorCode());
    cal->setNonKDECustomProperty("X-WR-CALNAME", typeInfo->displayName());

    auto ids = m_accountDB->getScheduleIDListByTypeID(typeID);
    foreach (auto id, ids) {
        auto schedule = m_accountDB->getScheduleByScheduleID(id);
        cal->addEvent(schedule);
    }
    qCDebug(ServiceLogger) << "Exporting" << cal->events().count() << "schedules.";
    KCalendarCore::ICalFormat icalformat;
    return icalformat.save(cal, icsFilePath);
}
