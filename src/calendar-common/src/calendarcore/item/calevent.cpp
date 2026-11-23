// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "calevent.h"

#include <QUuid>
#include <algorithm>

using namespace CalendarCore;

CalEvent::CalEvent()
    : m_uid(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      m_created(QDateTime::currentDateTimeUtc()), m_modified(m_created)
{
}

CalEvent::CalEvent(const CalEvent &event)
    : PropertyBag(event), m_uid(event.m_uid), m_schedulingId(event.m_schedulingId),
      m_summary(event.m_summary), m_description(event.m_description), m_location(event.m_location),
      m_categories(event.m_categories),
      m_start(event.m_start), m_end(event.m_end), m_created(event.m_created),
      m_modified(event.m_modified), m_recurrenceId(event.m_recurrenceId),
      m_revision(event.m_revision), m_allDay(event.m_allDay), m_lunar(event.m_lunar),
      m_hasDuration(event.m_hasDuration), m_summaryRich(event.m_summaryRich),
      m_descriptionRich(event.m_descriptionRich), m_locationRich(event.m_locationRich),
      m_thisAndFuture(event.m_thisAndFuture), m_recurrenceIdDateOnly(event.m_recurrenceIdDateOnly),
      m_cancelled(event.m_cancelled), m_timeZoneComponents(event.m_timeZoneComponents),
      m_duration(event.m_duration), m_repeat(event.m_repeat),
      m_organizer(event.m_organizer), m_attendees(event.m_attendees),
      m_attendeeProperties(event.m_attendeeProperties), m_extraProperties(event.m_extraProperties),
      m_parsedAttendeeCount(event.m_parsedAttendeeCount), m_organizerProperty(event.m_organizerProperty)
{
    for (const auto &alarm : event.m_alarms) {
        m_alarms.append(Remind::Ptr::create(*alarm));
    }
}

void CalEvent::setSchedulingID(const QString &id, const QString &uid)
{
    m_schedulingId = id;
    if (!uid.isEmpty()) {
        m_uid = uid;
    }
}

QString CalEvent::instanceIdentifier() const
{
    return m_uid + m_recurrenceId.toString(Qt::ISODate);
}

void CalEvent::setDtStart(const QDateTime &value)
{
    if (m_start.isValid() && m_start.timeZone().id() != value.timeZone().id()) {
        m_repeat.setCustomTimeZone({}, {});
    }
    m_start = value;
    m_repeat.setAnchor(value, m_allDay);
}

void CalEvent::setAllDay(bool value)
{
    m_allDay = value;
    m_repeat.setAnchor(m_start, value);
}

QDateTime CalEvent::dtEnd() const
{
    if (m_end.isValid()) {
        return m_end;
    }
    if (m_hasDuration) {
        const auto end = m_duration.end(m_start);
        return m_allDay ? qMax(m_start, end.addDays(-1)) : end;
    }
    return m_start;
}

void CalEvent::setDtEnd(const QDateTime &value)
{
    m_end = value;
    m_hasDuration = false;
}

TimeSpan CalEvent::duration() const
{
    return m_hasDuration ? m_duration : TimeSpan();
}

void CalEvent::setDuration(TimeSpan value)
{
    m_duration = value;
    m_hasDuration = true;
    m_end = {};
}

bool CalEvent::isMultiDay(const QTimeZone &zone) const
{
    if (m_allDay || !zone.isValid()) {
        return dtEnd().date() > m_start.date();
    }
    return dtEnd().toTimeZone(zone).date() > m_start.toTimeZone(zone).date();
}

Remind::Ptr CalEvent::firstEnabledAlarm() const
{
    const auto alarm = std::find_if(m_alarms.cbegin(), m_alarms.cend(), [](const auto &entry) { return entry->enabled(); });
    return alarm == m_alarms.cend() ? Remind::Ptr() : *alarm;
}

bool CalEvent::hasEnabledAlarms() const
{
    return bool(firstEnabledAlarm());
}

Remind::Ptr CalEvent::newAlarm()
{
    const auto alarm = Remind::Ptr::create();
    m_alarms.append(alarm);
    return alarm;
}

void CalEvent::addAttendee(const Participant &participant)
{
    const auto sameEmail = [&](const Participant &existing) {
        return existing.email().compare(participant.email(), Qt::CaseInsensitive) == 0;
    };
    if (std::none_of(m_attendees.cbegin(), m_attendees.cend(), sameEmail)) {
        m_attendees.append(participant);
    }
}
