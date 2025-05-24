// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicemanager.h"

#include "dhuangliservice.h"
#include "daccountmanagerservice.h"
#include "units.h"
#include "commondef.h"

#include "dbuscloudsync.h"
#include <QDBusConnection>
#include <QDBusError>

Q_LOGGING_CATEGORY(serviceManager, "calendar.service.manager")

DServiceManager::DServiceManager(QObject *parent)
    : QObject(parent)
{
    qCDebug(serviceManager) << "Initializing ServiceManager";
    //注册服务
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService(serviceBaseName)) {
        qCCritical(serviceManager) << "Failed to register service:" << sessionBus.lastError();
        exit(0x0001);
    }
    qCInfo(serviceManager) << "Successfully registered DBus service:" << serviceBaseName;

    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    //创建黄历服务
    DServiceBase *huangliService = new class DHuangliService(this);
    if (!sessionBus.registerObject(huangliService->getPath(), huangliService->getInterface(), huangliService, options)) {
        qCCritical(serviceManager) << "Failed to register huangliService object:" << sessionBus.lastError();
        exit(0x0002);
    }
    qCInfo(serviceManager) << "Successfully registered huangliService object";

    //创建帐户管理服务
    m_accountManagerService = new class DAccountManagerService(this);
    if (!sessionBus.registerObject(m_accountManagerService->getPath(), m_accountManagerService->getInterface(), m_accountManagerService, options)) {
        qCCritical(serviceManager) << "Failed to register accountManagerService object:" << sessionBus.lastError();
        exit(0x0003);
    }
    qCInfo(serviceManager) << "Successfully registered accountManagerService object";

    //创建云同步回调服务
    DServiceBase *cloudsyncService = new class Dbuscloudsync(this);
    if (!sessionBus.registerObject(cloudsyncService->getPath(), cloudsyncService->getInterface(), cloudsyncService, options)) {
        qCCritical(serviceManager) << "Failed to register cloudsyncService object:" << sessionBus.lastError();
        exit(0x0004);
    }
    qCInfo(serviceManager) << "Successfully registered cloudsyncService object";
    qCInfo(serviceManager) << "ServiceManager initialization completed";
}

void DServiceManager::updateRemindJob()
{
    qCDebug(serviceManager) << "Updating remind job";
    if(nullptr != m_accountManagerService){
        m_accountManagerService->updateRemindJob(false);
    } else {
        qCWarning(serviceManager) << "AccountManagerService is null, cannot update remind job";
    }
}
