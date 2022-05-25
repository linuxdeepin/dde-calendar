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
#include "dservicemanager.h"

#include "dhuangliservice.h"
#include "daccountmanagerservice.h"
#include "units.h"

#include <QDBusConnection>
#include <QDBusError>


DServiceManager::DServiceManager(QObject *parent)
    : QObject(parent)
{
    //注册服务
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService(serviceBaseName)) {
        qCritical() << "registerService failed:" << sessionBus.lastError();
        exit(0x0001);
    }

    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    //创建黄历服务
    DServiceBase *huangliService = new class DHuangliService(this);
    if (!sessionBus.registerObject(huangliService->getPath(), huangliService->getInterface(), huangliService, options)) {
        qCritical() << "registerObject huangliService failed:" << sessionBus.lastError();
        exit(0x0002);
    }

    //创建帐户管理服务
    DServiceBase *accountManagerService = new class DAccountManagerService(this);
    if (!sessionBus.registerObject(accountManagerService->getPath(), accountManagerService->getInterface(), accountManagerService, options)) {
        qCritical() << "registerObject  accountManagerService failed:" << sessionBus.lastError();
        exit(0x0003);
    }
    //测试
    //    //创建帐户
    //    DServiceBase *accountService = new DAccountService(serviceBasePath + "/Account_Loalend", serviceBaseName + "./Account_Loalend", this);
    //    if (!sessionBus.registerObject(accountService->getPath(), accountService, options)) {
    //        qCritical() << "registerObject failed:" << sessionBus.lastError();
    //        exit(0x0002);
    //    }
}
