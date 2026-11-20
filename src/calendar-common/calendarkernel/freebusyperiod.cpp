// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
/**
  @file
  This file is part of the API for handling calendar data and
  defines the FreeBusyPeriod class.

  @brief
  Represents a period of time.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "freebusyperiod.h"

using namespace CalendarKernel;

//@cond PRIVATE
class Q_DECL_HIDDEN CalendarKernel::FreeBusyPeriod::Private
{
public:
    Private()
        : mType(Unknown)
    {
    }

    QString mSummary;
    QString mLocation;
    FreeBusyType mType;
};
//@endcond

FreeBusyPeriod::FreeBusyPeriod()
    : Period()
    , d(new CalendarKernel::FreeBusyPeriod::Private())
{
}

FreeBusyPeriod::FreeBusyPeriod(const QDateTime &start, const QDateTime &end)
    : Period(start, end)
    , d(new CalendarKernel::FreeBusyPeriod::Private())
{
}

FreeBusyPeriod::FreeBusyPeriod(const QDateTime &start, const Duration &duration)
    : Period(start, duration)
    , d(new CalendarKernel::FreeBusyPeriod::Private())
{
}

FreeBusyPeriod::FreeBusyPeriod(const FreeBusyPeriod &period)
    : Period(period)
    , d(new CalendarKernel::FreeBusyPeriod::Private(*period.d))
{
}

FreeBusyPeriod::FreeBusyPeriod(const Period &period)
    : Period(period)
    , d(new CalendarKernel::FreeBusyPeriod::Private())
{
}

FreeBusyPeriod::~FreeBusyPeriod()
{
    delete d;
}

FreeBusyPeriod &FreeBusyPeriod::operator=(const FreeBusyPeriod &other)
{
    // check for self assignment
    if (&other == this) {
        return *this;
    }

    Period::operator=(other);
    *d = *other.d;
    return *this;
}

QString FreeBusyPeriod::summary() const
{
    return d->mSummary;
}

void FreeBusyPeriod::setSummary(const QString &summary)
{
    d->mSummary = summary;
}

QString FreeBusyPeriod::location() const
{
    return d->mLocation;
}

void FreeBusyPeriod::setLocation(const QString &location)
{
    d->mLocation = location;
}

FreeBusyPeriod::FreeBusyType FreeBusyPeriod::type() const
{
    return d->mType;
}

void FreeBusyPeriod::setType(FreeBusyPeriod::FreeBusyType type)
{
    d->mType = type;
}

QDataStream &CalendarKernel::operator<<(QDataStream &stream, const CalendarKernel::FreeBusyPeriod &period)
{
    CalendarKernel::Period periodParent = static_cast<CalendarKernel::Period>(period);
    stream << periodParent;
    stream << period.summary() << period.location() << static_cast<int>(period.type());
    return stream;
}

QDataStream &CalendarKernel::operator>>(QDataStream &stream, FreeBusyPeriod &period)
{
    CalendarKernel::Period periodParent;
    QString summary, location;
    int type;

    stream >> periodParent >> summary >> location >> type;

    period = periodParent;
    period.setLocation(location);
    period.setSummary(summary);
    period.setType(static_cast<FreeBusyPeriod::FreeBusyType>(type));
    return stream;
}
