// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "customproperties.h"
#include "participant.h"
#include "remind.h"
#include "repeatinfo.h"

#include <QMap>
#include <QStringList>

namespace CalendarCore {

class CalEvent : public PropertyBag
{
public:
    using Ptr = QSharedPointer<CalEvent>;
    using List = QList<Ptr>;

    CalEvent();
    CalEvent(const CalEvent &event);
    CalEvent &operator=(const CalEvent &) = delete;
    virtual ~CalEvent() = default;
    virtual CalEvent *clone() const { return new CalEvent(*this); }

    QString uid() const { return m_uid; }
    void setUid(const QString &value) { m_uid = value; }
    QString schedulingID() const { return m_schedulingId.isNull() ? m_uid : m_schedulingId; }
    void setSchedulingID(const QString &id, const QString &uid);
    QStringList categories() const { return m_categories; }
    QString summary() const { return m_summary; }
    void setSummary(const QString &value) { m_summary = value; }
    QString description() const { return m_description; }
    void setDescription(const QString &value) { m_description = value; }
    QString location() const { return m_location; }
    void setLocation(const QString &value) { m_location = value; }
    QDateTime created() const { return m_created; }
    void setCreated(const QDateTime &value) { m_created = value.toUTC(); }
    QDateTime lastModified() const { return m_modified; }
    void setLastModified(const QDateTime &value) { m_modified = value.toUTC(); }
    int revision() const { return m_revision; }
    void setRevision(int value) { m_revision = value; }
    QDateTime recurrenceId() const { return m_recurrenceId; }
    bool hasRecurrenceId() const { return m_recurrenceId.isValid(); }
    bool recurrenceIdIsDateOnly() const { return m_recurrenceIdDateOnly; }
    void setRecurrenceId(const QDateTime &value) { setRecurrenceId(value, m_allDay); }
    void setRecurrenceId(const QDateTime &value, bool dateOnly) { m_recurrenceId = value; m_recurrenceIdDateOnly = dateOnly; }
    bool thisAndFuture() const { return m_thisAndFuture; }
    void setThisAndFuture(bool value) { m_thisAndFuture = value; }
    bool cancelled() const { return m_cancelled; }
    void setCancelled(bool value) { m_cancelled = value; }
    QString instanceIdentifier() const;

    bool allDay() const { return m_allDay; }
    void setAllDay(bool value);
    bool lunar() const { return m_lunar; }
    void setLunar(bool value) { m_lunar = value; }
    QDateTime dtStart() const { return m_start; }
    void setDtStart(const QDateTime &value);
    QDateTime dtEnd() const;
    void setDtEnd(const QDateTime &value);
    TimeSpan duration() const;
    void setDuration(TimeSpan value);
    bool isMultiDay(const QTimeZone &zone = {}) const;

    RepeatInfo *recurrence() { return &m_repeat; }
    const RepeatInfo *recurrence() const { return &m_repeat; }
    bool recurs() const { return m_repeat.recurs(); }
    void clearRepeat() { m_repeat.clear(); }
    Remind::List alarms() const { return m_alarms; }
    Remind::Ptr newAlarm();
    void addAlarm(const Remind::Ptr &alarm) { if (alarm) m_alarms.append(alarm); }
    void clearAlarms() { m_alarms.clear(); }
    Remind::Ptr firstEnabledAlarm() const;
    bool hasEnabledAlarms() const;
    const QList<Participant> &attendees() const { return m_attendees; }
    void addAttendee(const Participant &participant);
    Contact organizer() const { return m_organizer; }
    void setOrganizer(const Contact &value) { m_organizer = value; m_organizerProperty.clear(); }

private:
    friend class IcsCodec;
    QString m_uid, m_schedulingId, m_summary, m_description, m_location;
    QStringList m_categories;
    QDateTime m_start, m_end, m_created, m_modified, m_recurrenceId;
    int m_revision = 0;
    bool m_allDay = false;
    bool m_lunar = false;
    bool m_hasDuration = false;
    bool m_summaryRich = false;
    bool m_descriptionRich = false;
    bool m_locationRich = false;
    bool m_thisAndFuture = false;
    bool m_recurrenceIdDateOnly = false;
    bool m_cancelled = false;
    QMap<QByteArray, QByteArray> m_timeZoneComponents;
    TimeSpan m_duration;
    RepeatInfo m_repeat;
    Remind::List m_alarms;
    Contact m_organizer;
    QList<Participant> m_attendees;
    QList<QByteArray> m_attendeeProperties, m_extraProperties;
    qsizetype m_parsedAttendeeCount = 0;
    QByteArray m_organizerProperty;
};

} // namespace CalendarCore
