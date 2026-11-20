// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Period class.

  @brief
  Represents a period of time.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#ifndef CALENDARKERNEL_FREEBUSYPERIOD_H
#define CALENDARKERNEL_FREEBUSYPERIOD_H

#include "period.h"

#include <QMetaType>

namespace CalendarKernel {
/**
  The period can be defined by either a start time and an end time or
  by a start time and a duration.
*/
class Q_CORE_EXPORT FreeBusyPeriod : public Period
{
public:
    enum FreeBusyType {
        Free,
        Busy,
        BusyUnavailable,
        BusyTentative,
        Unknown,
    };

    /**
       List of periods.
     */
    typedef QVector<FreeBusyPeriod> List;

    /**
      Constructs a period without a duration.
    */
    FreeBusyPeriod();

    /**
      Constructs a period from @p start to @p end.

      @param start the time the period begins.
      @param end the time the period ends.
    */
    FreeBusyPeriod(const QDateTime &start, const QDateTime &end);

    /**
      Constructs a period from @p start and lasting @p duration.

      @param start the time when the period starts.
      @param duration how long the period lasts.
    */
    FreeBusyPeriod(const QDateTime &start, const Duration &duration);

    /**
      Constructs a period by copying another period object

      @param period the period to copy
     */

    FreeBusyPeriod(const FreeBusyPeriod &period);

    /**
      Constructs a period by copying another period object

      @param period the period to copy
     */

    FreeBusyPeriod(const Period &period); // krazy:exclude=explicit

    /**
      Destroys a period.
    */
    ~FreeBusyPeriod();

    /**
      Sets this period equal to the @p other one.

      @param other is the other period to compare.
    */
    FreeBusyPeriod &operator=(const FreeBusyPeriod &other);

    /**
      Sets the period summary.
      @param summary is the period summary string.
      @see summary().
    */
    void setSummary(const QString &summary);

    /**
      Returns the period summary.
      @see setSummary()
    */
    Q_REQUIRED_RESULT QString summary() const;

    /**
      Sets the period location.
      @param location is the period location string.
      @see location().
    */
    void setLocation(const QString &location);

    /**
      Returns the period location.
      @see setLocation()
    */
    Q_REQUIRED_RESULT QString location() const;

    /**
      Sets the free/busy type.
      @param type is the type of free/busy period
      @see type().
      @since 5.0
    */
    void setType(FreeBusyType type);

    /**
      Returns free/busy type
      @see setType().
      @since 5.0
    */
    Q_REQUIRED_RESULT FreeBusyType type() const;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const CalendarKernel::FreeBusyPeriod &period);
    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, CalendarKernel::FreeBusyPeriod &period);
};

/** Write @p period to the datastream @p stream, in binary format. */
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const CalendarKernel::FreeBusyPeriod &period);

/** Read a Period object into @p period from @p stream, in binary format. */
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, CalendarKernel::FreeBusyPeriod &period);
} // namespace CalendarKernel

//@cond PRIVATE
Q_DECLARE_METATYPE(CalendarKernel::FreeBusyPeriod)
//@endcond

#endif
