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

    int syncState() const;
    void setSyncState(int syncState);

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

    int syncFreq() const;
    void setSyncFreq(int syncFreq);

    int intervalTime() const;
    void setIntervalTime(int intervalTime);

    QString dbusInterface() const;
    void setDbusInterface(const QString &dbusInterface);

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
    int m_syncState; //同步状态
    QDateTime m_dtCreate;
    QDateTime m_dtDelete;
    QDateTime m_dtUpdate;
    QString m_cloudPath;
    int m_syncFreq; //同步频率
    int m_intervalTime; //当同步频率为自定义时，才有效，单位（min）

    bool m_isExpandDisplay; //左侧帐户列表信息是否展开显示
};

#endif // DACCOUNT_H
