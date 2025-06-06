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
#include <QLoggingCategory>


DServiceManager::DServiceManager(QObject *parent)
    : QObject(parent)
{
    //注册服务
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    qCDebug(ServiceLogger) << "Registering service:" << serviceBaseName;
    if (!sessionBus.registerService(serviceBaseName)) {
        QDBusError error = sessionBus.lastError();
        qCritical(ServiceLogger) << "Failed to register service"
                               << "\n  Service name:" << serviceBaseName
                               << "\n  Error name:" << error.name()
                               << "\n  Error message:" << error.message()
                               << "\n  Error type:" << error.type();
        exit(0x0001);
    }
    qCInfo(ServiceLogger) << "Successfully registered service:" << serviceBaseName;

    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    
    //创建黄历服务
    qCDebug(ServiceLogger) << "Creating Huangli service";
    DServiceBase *huangliService = new class DHuangliService(this);
    if (!sessionBus.registerObject(huangliService->getPath(), huangliService->getInterface(), huangliService, options)) {
        QDBusError error = sessionBus.lastError();
        qCritical(ServiceLogger) << "Failed to register Huangli service object"
                               << "\n  Path:" << huangliService->getPath()
                               << "\n  Interface:" << huangliService->getInterface()
                               << "\n  Error name:" << error.name()
                               << "\n  Error message:" << error.message()
                               << "\n  Error type:" << error.type();
        exit(0x0002);
    }
    qCInfo(ServiceLogger) << "Successfully registered Huangli service"
                         << "\n  Path:" << huangliService->getPath()
                         << "\n  Interface:" << huangliService->getInterface();

    //创建帐户管理服务
    qCDebug(ServiceLogger) << "Creating account manager service";
    m_accountManagerService = new class DAccountManagerService(this);
    if (!sessionBus.registerObject(m_accountManagerService->getPath(), m_accountManagerService->getInterface(), m_accountManagerService, options)) {
        QDBusError error = sessionBus.lastError();
        qCritical(ServiceLogger) << "Failed to register account manager service object"
                               << "\n  Path:" << m_accountManagerService->getPath()
                               << "\n  Interface:" << m_accountManagerService->getInterface()
                               << "\n  Error name:" << error.name()
                               << "\n  Error message:" << error.message()
                               << "\n  Error type:" << error.type();
        exit(0x0003);
    }
    qCInfo(ServiceLogger) << "Successfully registered account manager service"
                         << "\n  Path:" << m_accountManagerService->getPath()
                         << "\n  Interface:" << m_accountManagerService->getInterface();

    //创建云同步回调服务
    qCDebug(ServiceLogger) << "Creating cloud sync service";
    DServiceBase *cloudsyncService = new class Dbuscloudsync(this);
    if (!sessionBus.registerObject(cloudsyncService->getPath(), cloudsyncService->getInterface(), cloudsyncService, options)) {
        QDBusError error = sessionBus.lastError();
        qCritical(ServiceLogger) << "Failed to register cloud sync service object"
                               << "\n  Path:" << cloudsyncService->getPath()
                               << "\n  Interface:" << cloudsyncService->getInterface()
                               << "\n  Error name:" << error.name()
                               << "\n  Error message:" << error.message()
                               << "\n  Error type:" << error.type();
        exit(0x0004);
    }
    qCInfo(ServiceLogger) << "Successfully registered cloud sync service"
                         << "\n  Path:" << cloudsyncService->getPath()
                         << "\n  Interface:" << cloudsyncService->getInterface();

    qCInfo(ServiceLogger) << "Service manager initialization completed successfully";
}

void DServiceManager::updateRemindJob()
{
    if(nullptr != m_accountManagerService){
        m_accountManagerService->updateRemindJob(false);
    }
}
