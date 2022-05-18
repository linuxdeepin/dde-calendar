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

//帐户信息
class DAccount
{
public:
    enum Type {
        Local_Account, //本地帐户
        UnionID_Account, //union id 帐户
        CalDav_Account //caldav通用协议帐户
    };
    typedef QSharedPointer<DAccount> Ptr;

    explicit DAccount(Type type);
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

private:
    QString m_displayName; //显示名称
    QString m_accountID; //帐户id
    QString m_accountName; //帐户名称
    QString m_dbusPath; //dbus路径
    Type m_accountType; //帐户类型
    QString m_avatar;
    QString m_description;

    bool m_isExpandDisplay; //左侧帐户列表信息是否展开显示
};

#endif // DACCOUNT_H
