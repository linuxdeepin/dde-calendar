/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001, 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "schedulemessage.h"

#include <QString>
#include <QDebug>

Q_LOGGING_CATEGORY(scheduleMessageLog, "calendar.schedulemessage")

using namespace KCalendarCore;

//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::ScheduleMessage::Private
{
public:
    Private()
    {
    }

    IncidenceBase::Ptr mIncidence;
    iTIPMethod mMethod;
    Status mStatus;
    QString mError;

    ~Private()
    {
    }
};
//@endcond

ScheduleMessage::ScheduleMessage(const IncidenceBase::Ptr &incidence, iTIPMethod method, ScheduleMessage::Status status)
    : d(new KCalendarCore::ScheduleMessage::Private)
{
    qCDebug(scheduleMessageLog) << "Creating schedule message with method" << methodName(method) << "status" << status;
    d->mIncidence = incidence;
    d->mMethod = method;
    d->mStatus = status;
}

ScheduleMessage::~ScheduleMessage()
{
    qCDebug(scheduleMessageLog) << "Destroying schedule message";
    delete d;
}

IncidenceBase::Ptr ScheduleMessage::event() const
{
    return d->mIncidence;
}

iTIPMethod ScheduleMessage::method() const
{
    return d->mMethod;
}

QString ScheduleMessage::methodName(iTIPMethod method)
{
    QString name;
    switch (method) {
    case iTIPPublish:
        name = QStringLiteral("Publish");
        break;
    case iTIPRequest:
        name = QStringLiteral("Request");
        break;
    case iTIPRefresh:
        name = QStringLiteral("Refresh");
        break;
    case iTIPCancel:
        name = QStringLiteral("Cancel");
        break;
    case iTIPAdd:
        name = QStringLiteral("Add");
        break;
    case iTIPReply:
        name = QStringLiteral("Reply");
        break;
    case iTIPCounter:
        name = QStringLiteral("Counter");
        break;
    case iTIPDeclineCounter:
        name = QStringLiteral("Decline Counter");
        break;
    default:
        name = QStringLiteral("Unknown");
        qCWarning(scheduleMessageLog) << "Unknown iTIP method:" << method;
        break;
    }
    qCDebug(scheduleMessageLog) << "iTIP method" << method << "mapped to name" << name;
    return name;
}

ScheduleMessage::Status ScheduleMessage::status() const
{
    return d->mStatus;
}

QString ScheduleMessage::error() const
{
    return d->mError;
}
