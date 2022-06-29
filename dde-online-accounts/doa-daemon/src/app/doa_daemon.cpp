/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#include "dbus/dbus_consts.h"
#include "dbus/doaaccountmanager.h"

#include <DLog>

#include <QCoreApplication>
#include <QDBusMessage>
#include <QtDBus>
#include <QDBusConnection>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-online-account-service");

    qDebug() << Dtk::Core::DLogManager::getlogFilePath();

    Dtk::Core::DLogManager::registerFileAppender();
    Dtk::Core::DLogManager::registerConsoleAppender();

    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if (!sessionBus.registerService(kAccountsService)) {
        qCritical() << "registerService failed:" << sessionBus.lastError();
        exit(0x0001);
    }

    //创建帐户管理dbus服务对象
    DOAAccountManager m_doaaccountManager;
    //注册服务和对象
    if (sessionBus.registerService(kAccountsService)) {
        sessionBus.registerObject(kAccountsServiceManagerPath, &m_doaaccountManager, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportAllProperties);
    }

    //从数据库中查询帐户信息生成dbus服务
    m_doaaccountManager.creatAllAccountDbusFromDB();

    return app.exec();
}