// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulemessage.h"

#include <QString>

using namespace CalendarKernel;

//@cond PRIVATE
class Q_DECL_HIDDEN CalendarKernel::ScheduleMessage::Private
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
    : d(new CalendarKernel::ScheduleMessage::Private)
{
    d->mIncidence = incidence;
    d->mMethod = method;
    d->mStatus = status;
}

ScheduleMessage::~ScheduleMessage()
{
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
    switch (method) {
    case iTIPPublish:
        return QStringLiteral("Publish");
    case iTIPRequest:
        return QStringLiteral("Request");
    case iTIPRefresh:
        return QStringLiteral("Refresh");
    case iTIPCancel:
        return QStringLiteral("Cancel");
    case iTIPAdd:
        return QStringLiteral("Add");
    case iTIPReply:
        return QStringLiteral("Reply");
    case iTIPCounter:
        return QStringLiteral("Counter");
    case iTIPDeclineCounter:
        return QStringLiteral("Decline Counter");
    default:
        return QStringLiteral("Unknown");
    }
}

ScheduleMessage::Status ScheduleMessage::status() const
{
    return d->mStatus;
}

QString ScheduleMessage::error() const
{
    return d->mError;
}
