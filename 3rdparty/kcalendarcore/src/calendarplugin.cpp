/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarplugin.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(calendarPluginLog, "calendar.plugin")

using namespace KCalendarCore;

CalendarPlugin::CalendarPlugin(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    qCDebug(calendarPluginLog) << "Creating calendar plugin instance";
    Q_UNUSED(args)
}
