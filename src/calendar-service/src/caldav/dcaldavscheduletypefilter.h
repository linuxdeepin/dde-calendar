// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVSCHEDULETYPEFILTER_H
#define DCALDAVSCHEDULETYPEFILTER_H

#include "dcaldavprofile.h"
#include "dscheduletype.h"

class DAccountManagerDataBase;

class DCalDavScheduleTypeFilter
{
public:
    static void filter(DCalDavProviderProfile::ProviderType providerType,
                       const QString &accountID,
                       DScheduleType::List &types,
                       DAccountManagerDataBase *accountManagerDatabase);
};

#endif // DCALDAVSCHEDULETYPEFILTER_H
