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
#ifndef DACCOUNT_H
#define DACCOUNT_H

#include <QString>
#include <QSharedPointer>
#include <QDateTime>

//帐户信息
class DAccount
{
public:
    enum Type {
        Account_Local, //本地帐户
        Account_UnionID, //union id 帐户
        Account_CalDav //caldav通用协议帐户
    };

    enum AccountState {
        Account_Close = 0x1, //日历同步总开关被关闭(控制中心)
        Account_Open = 0x2, //日历同步总开关开启
        Account_Calendar = 0x4, //同步日程
        Account_Setting = 0x8, //同步通用设置
    };
    Q_DECLARE_FLAGS(AccountStates, AccountState)

    enum AccountSyncState {
        Sync_Normal, //正常
        Sync_NetworkAnomaly, //网络异常
        Sync_ServerException, //服务器异常
        Sync_StorageFull, //存储已满
    };

    enum SyncFreqType{
        SyncFreq_Maunal, //手动同步
        SyncFreq_15Mins,
        SyncFreq_30Mins,
        SyncFreq_1hour,
        SyncFreq_24hour,
    };

    typedef QSharedPointer<DAccount> Ptr;
    typedef QVector<DAccount::Ptr> List;

    explicit DAccount(Type type = Account_Local);
    QString displayName() const;
    void setDisplayName(const QString &displayName);

    QString accountID() const;
    void setAccountID(const QString &accountID);

    QString accountName() const;
    void setAccountName(const QString &accountName);

    QString dbusPath() const;
    void setDbusPath(const QString &dbusPath);

    Type accountType() const;
    void setAccountType(const Type &accountTyep);

    bool isExpandDisplay() const;
    void setIsExpandDisplay(bool isExpandDisplay);

    bool isNetWorkAccount();

    int syncTag() const;
    void setSyncTag(int syncTag);

    AccountSyncState syncState() const;
    void setSyncState(AccountSyncState syncState);

    QString avatar() const;
    void setAvatar(const QString &avatar);

    QString description() const;
    void setDescription(const QString &description);

    QDateTime dtCreate() const;
    void setDtCreate(const QDateTime &dtCreate);

    QDateTime dtDelete() const;
    void setDtDelete(const QDateTime &dtDelete);

    QDateTime dtUpdate() const;
    void setDtUpdate(const QDateTime &dtUpdate);

    static bool toJsonString(const DAccount::Ptr &account, QString &jsonStr);
    static bool fromJsonString(DAccount::Ptr &account, const QString &jsonStr);
    static bool toJsonListString(const DAccount::List &accountList, QString &jsonStr);
    static bool fromJsonListString(DAccount::List &accountList, const QString &jsonStr);

    QString dbName() const;
    void setDbName(const QString &dbName);

    QString cloudPath() const;
    void setCloudPath(const QString &cloudPath);

    SyncFreqType syncFreq() const;
    void setSyncFreq(SyncFreqType syncFreq);

    int intervalTime() const;
    void setIntervalTime(int intervalTime);

    QString dbusInterface() const;
    void setDbusInterface(const QString &dbusInterface);

    AccountStates accountState() const;
    void setAccountState(const AccountStates &accountState);

    QDateTime dtLastSync() const;
    void setDtLastSync(const QDateTime &dtLastSync);

    static QString syncFreqToJsonString(const DAccount::Ptr &account);
    static void syncFreqFromJsonString(const DAccount::Ptr &account, const QString &syncFreqStr);

private:
    QString m_displayName; //显示名称
    QString m_accountID; //帐户id
    QString m_accountName; //帐户名称
    QString m_dbName; //对应的数据库名称
    QString m_dbusPath; //dbus路径
    QString m_dbusInterface; //dbus接口
    Type m_accountType; //帐户类型
    QString m_avatar; //头像
    QString m_description; //描述
    int m_syncTag; //同步标识,用来与云端标识比对
    AccountStates m_accountState; //帐户状态
    AccountSyncState m_syncState; //同步状态
    QDateTime m_dtCreate;
    QDateTime m_dtDelete;
    QDateTime m_dtUpdate;
    QDateTime m_dtLastSync; //最后一次同步时间
    QString m_cloudPath;
    SyncFreqType m_syncFreq; //同步频率
    int m_intervalTime; //当同步频率为自定义时，才有效，单位（min）

    bool m_isExpandDisplay; //左侧帐户列表信息是否展开显示
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DAccount::AccountStates)

#endif // DACCOUNT_H
