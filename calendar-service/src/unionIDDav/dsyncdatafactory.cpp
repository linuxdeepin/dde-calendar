// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsyncdatafactory.h"

#include "dunioniddav.h"
#include "commondef.h"

#include <QDebug>

DSyncDataFactory::DSyncDataFactory()
{
    qCDebug(ServiceLogger) << "Creating DSyncDataFactory";
}

DDataSyncBase *DSyncDataFactory::createDataSync(const DAccount::Ptr &account)
{
    qCDebug(ServiceLogger) << "Creating data sync for account type:" << account->accountType();
    DDataSyncBase *syncBase = nullptr;
    switch (account->accountType()) {
    case DAccount::Account_UnionID:
        qCDebug(ServiceLogger) << "Creating UnionID data sync";
        syncBase = new DUnionIDDav();
        qCInfo(ServiceLogger) << "创建同步任务";
        break;
    default:
        qCDebug(ServiceLogger) << "Creating default data sync";
        syncBase = nullptr;
        break;
    }
    qCDebug(ServiceLogger) << "Data sync created";
    return syncBase;
}
