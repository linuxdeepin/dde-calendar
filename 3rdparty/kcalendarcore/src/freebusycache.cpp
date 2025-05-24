/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the FreeBusyCache abstract base class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "freebusycache.h"
#include <QDebug>

using namespace KCalendarCore;

Q_LOGGING_CATEGORY(freeBusyCacheLog, "calendar.freebusy.cache")

FreeBusyCache::~FreeBusyCache()
{
    qCDebug(freeBusyCacheLog) << "FreeBusyCache destroyed";
}

void FreeBusyCache::virtual_hook(int id, void *data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
    qCWarning(freeBusyCacheLog) << "Unhandled virtual hook called with id:" << id;
    Q_ASSERT(false);
}
