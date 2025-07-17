// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddatasyncbase.h"
#include "commondef.h"
#include "dunioniddav.h"

DDataSyncBase::DDataSyncBase()
{
    qCDebug(ServiceLogger) << "Initializing DataSyncBase";
    qRegisterMetaType<DDataSyncBase::UpdateTypes>("DDataSyncBase::UpdateTypes");
    qRegisterMetaType<SyncTypes>("SyncTypes");
    qCDebug(ServiceLogger) << "DataSyncBase initialization completed";
}

DDataSyncBase::~DDataSyncBase()
{
    qCDebug(ServiceLogger) << "Destroying DataSyncBase";
}


