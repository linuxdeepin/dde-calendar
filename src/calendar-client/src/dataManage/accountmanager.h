// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include "dbus/dbusaccountmanagerrequest.h"
#include "accountitem.h"
#include "dcalendargeneralsettings.h"

#include <QHash>
#include <QSet>
#include <QString>

//所有帐户管理类
class AccountManager : public QObject
{
    Q_OBJECT
public:
    ~AccountManager() override;

    static AccountManager *getInstance();
    QList<AccountItem::Ptr> getAccountList();
    AccountItem::Ptr getLocalAccountItem();
    AccountItem::Ptr getUnionAccountItem();
    DScheduleType::Ptr getScheduleTypeByScheduleTypeId(const QString &schduleTypeId);
    AccountItem::Ptr getAccountItemByScheduleTypeId(const QString &schduleTypeId);
    AccountItem::Ptr getAccountItemByAccountId(const QString &accountId);
    AccountItem::Ptr getAccountItemByAccountName(const QString &accountName);
    DCalendarGeneralSettings::Ptr getGeneralSettings();

    //重新获取帐户信息
    void resetAccount();

    //根据帐户ID下拉数据
    void downloadByAccountID(const QString &accountID, CallbackFunc callback = nullptr);
    //更新网络帐户数据
    void uploadNetWorkAccountData(CallbackFunc callback = nullptr);
    void validateCalDavAccount(int providerType, const QString &serverUrl, const QString &username,
                                const QString &credentialRef);
    void getCalDavAccountConfig(const QString &accountID);
    void validateCalDavAccountForUpdate(const QString &accountID, int providerType,
                                        const QString &serverUrl, const QString &username,
                                        const QString &credentialRef);
    void deleteCalDavAccountWithLocalDataOption(const QString &accountID, bool deleteLocalData);
    void resolveAllCalDavConflicts(const QString &accountID, bool keepLocal);
    void createCalDavAccount(int providerType, const QString &serverUrl, const QString &username,
                             const QString &credentialRef, const QString &displayName);
    void updateCalDavAccount(const QString &accountID, int providerType,
                             const QString &serverUrl, const QString &username,
                             const QString &credentialRef, const QString &displayName);

    //设置一周首日
    void setFirstDayofWeek(int);
    //设置时间显示格式
    void setTimeFormatType(int);
    // 设置一周首日来源
    void setFirstDayofWeekSource(DCalendarGeneralSettings::GeneralSettingSource);
    // 设置时间显示格式来源
    void setTimeFormatTypeSource(DCalendarGeneralSettings::GeneralSettingSource);
    // 获取一周首日来源
    DCalendarGeneralSettings::GeneralSettingSource getFirstDayofWeekSource();
    // 获取时间显示格式来源
    DCalendarGeneralSettings::GeneralSettingSource getTimeFormatTypeSource();
    //帐户登录
    void login();
    //帐户登出
    void loginout();

    bool getIsSupportUid() const;
    DCalDavAccountStatus getCalDavAccountStatus(const QString &accountID) const;
    bool canWriteCalDavAccount(const QString &accountID) const;

    /**
     * @brief Determines whether a completed CalDAV sync was manually requested.
     * @param accountID Identifier of the account that completed synchronization.
     * @param syncStatus Current CalDAV synchronization status.
     * @return True when the completed sync was manually requested. The request
     * marker is consumed before returning true.
     */
    bool takeManualCalDavSyncRequest(const QString &accountID, int syncStatus);

signals:
    void signalDataInitFinished();
    void signalAccountUpdate();
    void signalGeneralSettingsUpdate();

    void signalAccountDataUpdate();
    void signalScheduleUpdate();
    void signalScheduleTypeUpdate();
    void signalSearchScheduleUpdate();
    void signalAccountStateChange();
    void signalCalDavAccountStatusChanged(const QString &accountID);
    void signalCalDavAccountStatusReady();
    void signalCalDavScheduleCreateFailed(const QString &accountID, int createFailure);
    void signalCalDavAccountValidationStarted(const QString &requestID);
    void signalCalDavAccountValidationForUpdateStarted(const QString &requestID);
    void signalGetCalDavAccountConfigFinish(const QString &config);
    void signalCalDavAccountValidationFinished(const QString &requestID, bool success,
                                               int validationError, const QString &errorMessage,
                                               const QString &principalDisplayName);
    void signalCreateCalDavAccountFinish(const QString &accountID);
    void signalDeleteCalDavAccountFinish(bool success);
    void signalUpdateCalDavAccountFinish(bool success);
    void signalCalDavAccountRequestFailed(const QString &method, const QString &errorMessage);

    //正在同步
    void signalSyncNum(const int num = -1);

    //帐户登出信号
    void signalLogout(DAccount::Type);

public slots:
    //获取帐户信息完成事件
    void slotGetAccountListFinish(DAccount::List accountList);
    //获取通用设置完成事件
    void slotGetGeneralSettingsFinish(DCalendarGeneralSettings::Ptr ptr);
    //获取是否支持UID完成事件
    void slotGetIsSupportUidFinish(bool supported);
    void slotGetCalDavAccountStatusListFinish(DCalDavAccountStatus::List statusList);

protected:
    explicit AccountManager(QObject *parent = nullptr);

private:
    void initConnect();

private:
    static AccountManager *m_accountManager;
    AccountItem::Ptr  m_localAccountItem;
    AccountItem::Ptr  m_unionAccountItem;
    QList<AccountItem::Ptr> m_calDavAccountItems;
    QHash<QString, DCalDavAccountStatus> m_calDavAccountStatuses;
    QSet<QString> m_manualCalDavSyncAccounts;
    bool m_calDavStatusesInitialized = false;
    QHash<QString, int> m_calDavProviderTypeOverrides;
    int m_pendingCalDavProviderType = -1;
    QString m_pendingCalDavDeleteAccountID;
    DCalendarGeneralSettings::Ptr m_settings;

    DbusAccountManagerRequest *m_dbusRequest;
    bool m_isSupportUid = false;
    bool m_isSupportUidLoaded = false;
};

#define gAccountManager AccountManager::getInstance()
#define gLocalAccountItem gAccountManager->getLocalAccountItem()
#define gUosAccountItem gAccountManager->getUnionAccountItem()

#endif // ACCOUNTMANAGER_H
