// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACCOUNTMANAGERDATABASE_H
#define DACCOUNTMANAGERDATABASE_H

#include "daccount.h"
#include "dschedule.h"
#include "ddatabase.h"
#include "dcalendargeneralsettings.h"
#include "dcaldavaccountinfo.h"
#include "dcaldaverrorcode.h"
#include "dcaldavcalendarinfo.h"
#include "dcaldavcategoryinfo.h"
#include "dcaldaveventmappinginfo.h"
#include "dcaldavoutboxitem.h"

#include <QObject>
#include <QSharedPointer>
#include <QMap>

class DAccountManagerDataBase : public DDataBase
{
    Q_OBJECT
public:
    typedef QSharedPointer<DAccountManagerDataBase> Ptr;

    explicit DAccountManagerDataBase(QObject *parent = nullptr);

    //初始化数据库数据，只有在创建表的时候才需要
    void initDBData() override;
    //确保已存在账户库补齐最新表结构，不初始化账户数据。
    void ensureSchema();

    ///////////////帐户
    //获取所有帐户信息
    DAccount::List getAccountList();

    //根据帐户id获取帐户信息
    DAccount::Ptr getAccountByID(const QString &accountID);
    //添加帐户信息
    QString addAccountInfo(const DAccount::Ptr &accountInfo);
    //更新帐户信息
    bool updateAccountInfo(const DAccount::Ptr &accountInfo);
    //根据帐户id删除对应帐户
    bool deleteAccountInfo(const QString &accountID);

    ///////////////CalDAV 帐户
    QString getCalDavAccountStatusList();
    bool getCalDavAccountInfo(const QString &accountID, DCalDavAccountInfo &accountInfo);
    bool upsertCalDavAccountInfo(const DCalDavAccountInfo &accountInfo);
    bool deleteCalDavAccountInfo(const QString &accountID);
    bool deleteCalDavAccountData(const QString &accountID);
    bool upsertCalDavAccountDeletionCleanup(const QString &accountID, const QString &sourceDbName);
    QMap<QString, QString> getCalDavAccountDeletionCleanups() const;
    bool deleteCalDavAccountDeletionCleanup(const QString &accountID);
    DCalDavCategoryInfo getCalDavCategoryMapping(const QString &accountID,
                                                  const QString &calendarID,
                                                  const QString &categoryKey) const;
    DCalDavCategoryInfo::List getCalDavCategoryMappings(const QString &accountID,
                                                         const QString &calendarID) const;
    DCalDavCategoryInfo getCalDavCategoryMappingByScheduleTypeID(
        const QString &accountID, const QString &scheduleTypeID) const;
    bool upsertCalDavCategoryMapping(const DCalDavCategoryInfo &mapping);
    bool deleteCalDavCategoryMappingByScheduleTypeID(const QString &accountID,
                                                      const QString &scheduleTypeID);
    bool updateCalDavCredentialReference(const QString &accountID, const QString &credentialRef);
    DCalDavCalendarInfo::List getCalDavCalendarList(const QString &accountID);
    DCalDavCalendarInfo getCalDavCalendarByScheduleTypeID(const QString &accountID,
                                                           const QString &scheduleTypeID);
    bool upsertCalDavCalendar(const DCalDavCalendarInfo &calendar);
    bool updateCalDavCalendarSyncToken(const QString &calendarID, const QString &syncToken);
    bool updateCalDavCalendarInitialSyncCompleted(const QString &calendarID, bool completed);
    bool updateCalDavSyncStatus(const QString &accountID, int syncStatus,
                                const QDateTime &lastSuccessfulSync, const QString &failureReason,
                                DCalDavErrorCode failureCode = DCalDavErrorCode::NoError);
    bool updateCalDavRetryState(const QString &accountID, int retryCount,
                                 const QDateTime &nextRetryAt, const QString &failureReason,
                                 DCalDavErrorCode failureCode = DCalDavErrorCode::NoError);
    bool clearCalDavRetryState(const QString &accountID);
    QStringList dueCalDavAccountRetryIDs(const QDateTime &now) const;
    QStringList dueCalDavOutboxAccountIDs(const QDateTime &now) const;
    QDateTime earliestCalDavRetryAt() const;
    DCalDavEventMappingInfo getCalDavEventMapping(const QString &accountID, const QString &href);
    DCalDavEventMappingInfo getCalDavEventMappingByLocalScheduleID(const QString &accountID,
                                                                     const QString &localScheduleID);
    DCalDavEventMappingInfo::List getCalDavEventMappingList(const QString &accountID,
                                                            const QString &calendarID);
    bool upsertCalDavEventMapping(const DCalDavEventMappingInfo &mapping);
    bool deleteCalDavEventMapping(const QString &accountID, const QString &href);
    DCalDavOutboxItem getCalDavOutboxItem(const QString &accountID,
                                           const QString &localScheduleID);
    bool hasCalDavOutboxItems(const QString &accountID) const;
    DCalDavOutboxItem::List getCalDavConflictItems(const QString &accountID);
    DCalDavOutboxItem::List getCalDavBlockedOutboxItems(const QString &accountID);
    DCalDavOutboxItem::List getCalDavRetryScheduledOutboxItems(
        const QString &accountID, const QDateTime &now) const;
    DCalDavOutboxItem::List getDueCalDavOutboxItems(const QString &accountID,
                                                     const QDateTime &now,
                                                     bool ignoreRetryAt = false);
    QDateTime earliestCalDavOutboxRetryAt() const;
    bool upsertCalDavOutboxItem(const DCalDavOutboxItem &item);
    bool deleteCalDavOutboxItem(const QString &accountID, const QString &localScheduleID);
    bool deleteCalDavOutboxItemIfCurrent(const DCalDavOutboxItem &item);

    ///////////////通用设置

    DCalendarGeneralSettings::Ptr getCalendarGeneralSettings();
    void setCalendarGeneralSettings(const DCalendarGeneralSettings::Ptr &cgSet);


    void setLoaclDB(const QString &loaclDB);

protected:
    //创建数据库
    void createDB() override;
    void initAccountManagerDB();

private:
    QString m_loaclDB;
};

#endif // DACCOUNTMANAGERDATABASE_H
