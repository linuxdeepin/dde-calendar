// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddatasyncbase.h"
#include "dunioniddav.h"

Q_LOGGING_CATEGORY(syncBaseLog, "calendar.sync.base")

DDataSyncBase::DDataSyncBase()
{
    qCDebug(syncBaseLog) << "Initializing DDataSyncBase";
    qRegisterMetaType<DDataSyncBase::UpdateTypes>("DDataSyncBase::UpdateTypes");
    qRegisterMetaType<SyncTypes>("SyncTypes");
    qCDebug(syncBaseLog) << "DDataSyncBase initialization completed";
}

DDataSyncBase::~DDataSyncBase()
{
    qCDebug(syncBaseLog) << "DDataSyncBase destroyed";
}


