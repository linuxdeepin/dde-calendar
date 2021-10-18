/*
* Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     wangyou <wangyou@uniontech.com>
*
* Maintainer: wangyou <wangyou@uniontech.com>
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
#ifndef DOAACCOUNTSPASSWORDADAPTER_H
#define DOAACCOUNTSPASSWORDADAPTER_H
#include "doaprovider.h"

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

#include <QObject>

class DOAAccountsPassWordadapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.dde.onlineaccount.password")
public:
    explicit DOAAccountsPassWordadapter(QObject *parent = nullptr);

signals:
    void sign_changeProperty(const QString &propertyName, DOAProvider *doaProvider);
public Q_SLOTS: // METHODS
    //修改密码
    bool ChangePassword(const QString &password);
    //获取密码使用传输密钥加密
    QString getPassword();
};

#endif // DOAACCOUNTSPASSWORDADAPTER_H