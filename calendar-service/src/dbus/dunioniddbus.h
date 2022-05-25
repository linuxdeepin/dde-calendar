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
#ifndef DUNIONIDDBUS_H
#define DUNIONIDDBUS_H

#include "daccount.h"

#include <QtDBus/QtDBus>
#include <QSharedPointer>

const QString unionid_service = "com.deepin.sync.Daemon";
const QString unionid_path = "/com/deepin/utcloud/Daemon";

class DUnionIDDbus : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    typedef QSharedPointer<DUnionIDDbus> Ptr;

    explicit DUnionIDDbus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    static inline const char *staticInterfaceName()
    {
        return "com.deepin.utcloud.Daemon";
    }

    //获取帐户信息
    DAccount::Ptr getUserData();

    //TODO:
    void download(const QString &key, const QString &path);
    void login();
    void logout();
    void upload(const QString &path);
    void deleteFile(const QString &key);
    void metadata(const QString &meta);
    void onAuthorized();

private:
    DAccount::Ptr accountChangeHandle(const QString &accountInfo);
    /**
     * @brief getPropertyByName     根据属性名称获取属性值
     * @param porpertyName          属性名称
     * @return
     */
    QVariant getPropertyByName(const QString &porpertyName);

signals:

public slots:
    /**
     * @brief propertyChanged  关联的dbus服务属性改变
     * @param msg
     */
    void propertyChanged(const QDBusMessage &msg);
    /**
     * @brief slotDBusError     服务对象错误信息处理
     * @param error             错误信息
     */
    void slotDBusError(const QDBusError &error);

private:
    DAccount::Ptr m_account;
};

#endif // DUNIONIDDBUS_H
