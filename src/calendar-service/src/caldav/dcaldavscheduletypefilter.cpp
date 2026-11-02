// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavscheduletypefilter.h"

#include "daccountmanagerdatabase.h"
#include "dcaldavcalendarinfo.h"
#include "dcaldavxmlreader.h"

void DCalDavScheduleTypeFilter::filter(DCalDavProviderProfile::ProviderType providerType,
                                       const QString &accountID,
                                       DScheduleType::List &types,
                                       DAccountManagerDataBase *accountManagerDatabase)
{
    if (providerType != DCalDavProviderProfile::Provider_WeCom
        || accountID.isEmpty() || accountManagerDatabase == nullptr) {
        return;
    }

    for (auto type = types.begin(); type != types.end();) {
        if (type->isNull()) {
            ++type;
            continue;
        }

        const DScheduleType::Ptr &scheduleType = *type;
        const DCalDavCalendarInfo calendar =
            accountManagerDatabase->getCalDavCalendarByScheduleTypeID(accountID, scheduleType->typeID());
        const bool isWritableEventCalendar =
            scheduleType->description() == QStringLiteral("CalDAV calendar")
            && !calendar.calendarId.isEmpty()
            && calendar.supportedComponents.contains(QStringLiteral("VEVENT"))
            && (calendar.privileges & DCalDavXmlReader::WritePrivilege);
        if (!isWritableEventCalendar) {
            type = types.erase(type);
        } else {
            ++type;
        }
    }
}
