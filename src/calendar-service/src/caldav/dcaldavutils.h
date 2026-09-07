// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVUTILS_H
#define DCALDAVUTILS_H

#include "dcaldavcalendarinfo.h"
#include "dcaldavtransport.h"

#include <QString>
#include <QUrl>

namespace DCalDavUtils {

/**
 * @brief Converts a transport response into a user-facing CalDAV error.
 * @param response The failed transport response.
 * @return A stable English error description for the caller to localize.
 */
QString transportErrorText(const DCalDavTransport::Response &response);

/**
 * @brief Builds the deterministic resource URL for a new CalDAV event.
 * @param calendar The target CalDAV calendar collection.
 * @param uid The event UID.
 * @return The URL of the event's .ics resource.
 */
QUrl eventUrl(const DCalDavCalendarInfo &calendar, const QString &uid);

} // namespace DCalDavUtils

#endif // DCALDAVUTILS_H
