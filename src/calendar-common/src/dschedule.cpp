// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dschedule.h"
#include "pinyin/pinyinsearch.h"
#include "commondef.h"
#include "lunarandfestival/lunardateinfo.h"

#include "icscodec.h"
#include "memcalstore.h"
#include "units.h"

#include <QtDBus/QtDBus>

#include <iterator>
#include <limits>
#include <memory>
#include <utility>

#define Duration_Min 60
#define Duration_Hour 60 * 60
#define Duration_Day 24 * 60 * 60
#define Duration_Week 7 * 24 * 60 * 60

namespace {

constexpr char RecurrenceExceptionsStoredProperty[] = "X-DDE-RECURRENCE-EXCEPTIONS-STORED";

QDate displayDate(const QDateTime &dateTime, bool allDay = false)
{
    return allDay ? dateTime.date() : dateTime.toLocalTime().date();
}

bool hasUsableRecurrence(const DSchedule::Ptr &schedule)
{
    if (schedule.isNull() || !schedule->recurs()) {
        return false;
    }

    const CalendarCore::RepeatInfo *recurrence = schedule->recurrence();
    const CalendarCore::RRule *rule = recurrence->defaultRRuleConst();
    if (rule != nullptr && rule->periodType() != CalendarCore::RRule::rNone
        && !rule->rrule().isEmpty()) {
        return true;
    }

    return !recurrence->rDates().isEmpty() || !recurrence->rDateTimes().isEmpty();
}

} // namespace

DSchedule::DSchedule()
    : CalendarCore::CalEvent()
    , m_fileName("")
    , m_scheduleTypeID("")
    , m_compatibleID(0)
{
}

DSchedule::DSchedule(const DSchedule &schedule)
    : CalendarCore::CalEvent(schedule)
    , m_recurrenceExceptions(schedule.m_recurrenceExceptions)
    , m_recurrenceRanges(schedule.m_recurrenceRanges)
    , m_fileName("")
    , m_scheduleTypeID("")
    , m_compatibleID(0)
{
    this->setScheduleTypeID(schedule.scheduleTypeID());
    this->setAccountColor(schedule.accountColor());
}

DSchedule::DSchedule(const CalendarCore::CalEvent &event)
    : CalendarCore::CalEvent(event)
    , m_fileName("")
    , m_scheduleTypeID("")
    , m_compatibleID(0)
{
}

DSchedule *DSchedule::clone() const
{
    return new DSchedule(*this);
}

QString DSchedule::scheduleTypeID() const
{
    return m_scheduleTypeID;
}

void DSchedule::setScheduleTypeID(const QString &typeID)
{
    m_scheduleTypeID = typeID;
}

QString DSchedule::accountColor() const
{
    return m_accountColor;
}

void DSchedule::setAccountColor(const QString &color)
{
    m_accountColor = color;
}

bool DSchedule::isMoved()
{
    return this->m_moved;
}

void DSchedule::setMoved(bool moved)
{
    this->m_moved = moved;
}

bool DSchedule::isValid() const
{
    //TODO:添加判断日程是否有效

    return true;
}

bool DSchedule::isMultiDay() const
{
    //如果日期不一致则为跨天日程
    bool multiDay = dtStart().date() != dtEnd().date();

    return multiDay;
}

bool DSchedule::operator==(const DSchedule &schedule) const
{
    return this->instanceIdentifier() == schedule.instanceIdentifier();
}

bool operator<(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2)
{
    if (s1.isNull() || s2.isNull()) {
        return false;
    }
    return *s1.data() < *s2.data();
}

bool DSchedule::operator<(const DSchedule &schedule) const
{
    if (this->allDay() != schedule.allDay()) {
        return this->allDay() > schedule.allDay();
    }
    if (this->dtStart() != schedule.dtStart()) {
        return this->dtStart() < schedule.dtStart();
    }
    if (this->created() != schedule.created()) {
        return this->created() < schedule.created();
    }
    if (this->summary() != schedule.summary()) {
        return this->summary() < schedule.summary();
    }
    return true;
}

void DSchedule::setAlarmType(const DSchedule::AlarmType &alarmType)
{
    //如果提醒规则没有变化则退出
    if (alarmType == getAlarmType()) {
        qCDebug(CommonLogger) << "Alarm type unchanged, skipping update";
        return;
    }

    //清除提醒规则
    this->clearAlarms();
    //如果为从不则退出
    if (alarmType == AlarmType::Alarm_None || alarmType == AlarmType::Alarm_AllDay_None) {
        qCDebug(CommonLogger) << "Setting alarm type to None, clearing all alarms";
        return;
    }

    QMap<int, AlarmType> alarmMap = getAlarmMap();
    QMap<int, AlarmType>::const_iterator iter = alarmMap.constBegin();
    for (; iter != alarmMap.constEnd(); ++iter) {
        if (iter.value() == alarmType) {
            CalendarCore::Remind::Ptr alarm = CalendarCore::Remind::Ptr::create();
            alarm->setEnabled(true);
            alarm->setType(CalendarCore::Remind::Display);
            alarm->setDisplayAlarm(this->summary());
            CalendarCore::TimeSpan duration(iter.key());
            alarm->setStartOffset(duration);
            addAlarm(alarm);
            qCDebug(CommonLogger) << "Added alarm with offset:" << iter.key() << "seconds";
            break;
        }
    }
}

DSchedule::AlarmType DSchedule::getAlarmType()
{
    AlarmType alarmType = allDay() ? Alarm_AllDay_None : Alarm_None;
    if (const auto alarm = firstEnabledAlarm()) {
        const auto duration = alarm->startOffset();
        QMap<int, AlarmType> alarmMap = getAlarmMap();
        const auto seconds = duration.asSeconds();
        if (seconds >= (std::numeric_limits<int>::min)() && seconds <= (std::numeric_limits<int>::max)()) {
            alarmType = alarmMap.value(int(seconds), alarmType);
        }
    }

    return alarmType;
}

void DSchedule::setRRuleType(const DSchedule::RRuleType &rtype)
{
    if (getRRuleType() == rtype) {
        qCDebug(CommonLogger) << "Recurrence rule type unchanged, skipping update";
        return;
    }

    clearRepeat();

    QString rules;
    switch (rtype) {
    case RRule_Year:
        rules = "FREQ=YEARLY";
        break;
    case RRule_Month:
        rules = "FREQ=MONTHLY";
        break;
    case RRule_Week:
        rules = "FREQ=WEEKLY";
        break;
    case RRule_Work:
        rules = "FREQ=DAILY;BYDAY=MO,TU,WE,TH,FR";
        break;
    case RRule_Day:
        rules = "FREQ=DAILY";
        break;
    default:
        rules = "";
        break;
    }

    if (!rules.isEmpty()) {

        CalendarCore::RepeatInfo *recurrence = this->recurrence();
        auto rrule = std::unique_ptr<CalendarCore::RRule>(new CalendarCore::RRule);

        CalendarCore::IcsCodec ical;
        if (ical.fromString(rrule.get(), rules)) {
            recurrence->addRRule(rrule.release());
        } else {
            qCWarning(CommonLogger) << "Invalid recurrence rule for schedule:" << uid() << rules;
        }
    }
}

DSchedule::RRuleType DSchedule::getRRuleType()
{
    RRuleType rtype = RRule_None;
    if (this->recurs()) {
        const auto rrule = this->recurrence()->defaultRRuleConst();
        if (!rrule) {
            return rtype;
        }
        switch (rrule->periodType()) {
        case CalendarCore::RRule::rYearly: {
            rtype = RRule_Year;
        } break;
        case CalendarCore::RRule::rMonthly: {
            rtype = RRule_Month;
        } break;
        case CalendarCore::RRule::rWeekly: {
            rtype = RRule_Week;
        } break;
        case CalendarCore::RRule::rDaily: {
            if (rrule->rrule().contains("BYDAY=MO,TU,WE,TH,FR")) {
                rtype = RRule_Work;
            } else {
                rtype = RRule_Day;
            }
        } break;
        default:
            rtype = RRule_None;
            break;
        }
    }

    return rtype;
}

int DSchedule::numberOfRepetitions(const Ptr &scheudle, const QDateTime &datetime)
{
    return scheudle ? scheudle->recurrence()->durationTo(datetime) : -1;
}

bool DSchedule::fromJsonString(DSchedule::Ptr &schedule, const QString &json)
{
    if (schedule.isNull()) {

        schedule = DSchedule::Ptr(new DSchedule);
    }

    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(json.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "Failed to parse schedule JSON. Error:" << jsonError.errorString();
        return false;
    }

    if (!jsonDoc.isObject()) {
        qCWarning(CommonLogger) << "Schedule JSON root is not an object";
        return false;
    }

    bool resBool = false;
    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("schedule") && rootObj.value("schedule").isString()) {
        const QString str = rootObj.value("schedule").toString();
        if (fromIcsString(schedule, str)) {
            if (rootObj.contains("type")) {
                if (!rootObj.value("type").isString()) {
                    qCWarning(CommonLogger) << "Schedule type is not a string";
                    return false;
                }
                schedule->setScheduleTypeID(rootObj.value("type").toString());
            }
            if (rootObj.contains("compatibleID")) {
                const QJsonValue compatibleID = rootObj.value("compatibleID");
                if (!compatibleID.isDouble() || compatibleID.toDouble() != compatibleID.toInt()) {
                    qCWarning(CommonLogger) << "Schedule compatibleID is not an integer";
                    return false;
                }
                schedule->setcompatibleID(compatibleID.toInt());
            }
            resBool = true;
        }
    } else {
        qCWarning(CommonLogger) << "No schedule data found in JSON";
    }
    return resBool;
}

bool DSchedule::toJsonString(const DSchedule::Ptr &schedule, QString &json)
{
    if (schedule.isNull()) {
        qCWarning(CommonLogger) << "Cannot convert null schedule to JSON";
        return false;
    }

    QJsonObject rootObject;
    rootObject.insert("type", schedule->scheduleTypeID());
    rootObject.insert("schedule", toIcsString(schedule));
    rootObject.insert("compatibleID", schedule->compatibleID());
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    json = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));

    return true;
}

bool DSchedule::fromIcsString(Ptr &schedule, const QString &string)
{
    return parseIcsString(schedule, string, false);
}

bool DSchedule::fromStoredIcsString(Ptr &schedule, const QString &string)
{
    return parseIcsString(schedule, string, true);
}

bool DSchedule::parseIcsString(Ptr &schedule, const QString &string, bool existingData)
{
    bool resBool = false;
    CalendarCore::IcsCodec icalformat;
    QTimeZone timezone = QDateTime::currentDateTime().timeZone();
    CalendarCore::MemCalStore::Ptr _cal(new CalendarCore::MemCalStore(timezone));
    QString diagnostic;
    const auto mode = existingData ? CalendarCore::IcsCodec::ReadMode::ExistingData
                                   : CalendarCore::IcsCodec::ReadMode::Strict;
    if (icalformat.fromString(_cal, string, false, &diagnostic, mode)) {
        CalendarCore::CalEvent::List eventList = _cal->events();
        if (!eventList.isEmpty()) {
            // Select the master even when detached VEVENTs precede it.
            CalendarCore::CalEvent::Ptr selectedEvent = eventList.first();
            for (const CalendarCore::CalEvent::Ptr &event : eventList) {
                if (!event->hasRecurrenceId()) {
                    selectedEvent = event;
                    break;
                }
            }
            schedule = DSchedule::Ptr(new DSchedule(*selectedEvent.data()));
            for (const auto &event : eventList) {
                if (event != selectedEvent && event->uid() == selectedEvent->uid() && event->hasRecurrenceId()) {
                    schedule->setRecurrenceException(*event);
                }
            }
            resBool = true;
        }
    } else {
        qCWarning(CommonLogger) << "Failed to parse ICS string:" << diagnostic;
    }
    return resBool;
}

QString DSchedule::toIcsString(const DSchedule::Ptr &schedule)
{
    CalendarCore::IcsCodec icalformat;
    CalendarCore::MemCalStore::Ptr _cal(new CalendarCore::MemCalStore(nullptr));
    if (!schedule) {
        return {};
    }
    _cal->addEvent(schedule);
    for (const auto &exception : schedule->recurrenceExceptions()) {
        _cal->addEvent(exception);
    }
    return icalformat.toString(_cal.staticCast<CalendarCore::CalStore>());
}

void DSchedule::setRecurrenceException(const CalendarCore::CalEvent &event)
{
    if (event.hasRecurrenceId()) {
        const auto id = event.recurrenceId();
        const auto copy = CalendarCore::CalEvent::Ptr::create(event);
        m_recurrenceExceptions.insert(id, copy);
        if (event.thisAndFuture()) {
            m_recurrenceRanges.insert(id, copy);
        } else {
            m_recurrenceRanges.remove(id);
        }
        setRecurrenceExceptionsStored(true);
    }
}

void DSchedule::updateRecurrenceException(const CalendarCore::CalEvent &event)
{
    const auto id = event.recurrenceId();
    const auto range = m_recurrenceRanges.value(id);
    if (range && !event.thisAndFuture()) {
        if (const auto continuation = rangeContinuation(*range, id.addMSecs(1))) {
            setRecurrenceException(*continuation);
        }
    }
    setRecurrenceException(event);
}

void DSchedule::clearRecurrenceExceptions()
{
    m_recurrenceExceptions.clear();
    m_recurrenceRanges.clear();
    setRecurrenceExceptionsStored(false);
}

bool DSchedule::hasStoredRecurrenceExceptions() const
{
    return !nonKDECustomProperty(RecurrenceExceptionsStoredProperty).isEmpty();
}

void DSchedule::setRecurrenceExceptionsStored(bool stored)
{
    setNonKDECustomProperty(RecurrenceExceptionsStoredProperty, stored ? QStringLiteral("TRUE") : QString());
}

void DSchedule::pruneRecurrenceExceptionsAfter(const QDateTime &through)
{
    if (!through.isValid()) {
        return;
    }
    List continuations;
    auto entry = m_recurrenceExceptions.upperBound(through);
    while (entry != m_recurrenceExceptions.end()) {
        const auto id = entry.key();
        if (recurrence()->mayIncludeRecurrenceId(id) || (lunar() && !recurrenceStarts(id, id).isEmpty())) {
            ++entry;
            continue;
        }
        if (const auto range = m_recurrenceRanges.value(id)) {
            if (const auto continuation = rangeContinuation(*range, id.addMSecs(1))) {
                continuations.append(continuation);
            }
            m_recurrenceRanges.remove(id);
        }
        entry = m_recurrenceExceptions.erase(entry);
    }
    for (const auto &continuation : continuations) {
        setRecurrenceException(*continuation);
    }
}

void DSchedule::removeRecurrenceExceptionsFrom(const QDateTime &from)
{
    if (from.isValid()) {
        auto entry = m_recurrenceExceptions.lowerBound(from);
        while (entry != m_recurrenceExceptions.end()) {
            m_recurrenceRanges.remove(entry.key());
            entry = m_recurrenceExceptions.erase(entry);
        }
    }
}

bool DSchedule::inheritRecurrenceExceptions(const DSchedule &source, const QDateTime &from)
{
    const auto anchor = from.isValid() ? from : source.dtStart();
    auto exceptions = source.m_recurrenceExceptions;
    if (from.isValid()) {
        const auto nextRange = source.m_recurrenceRanges.lowerBound(from);
        if (nextRange != source.m_recurrenceRanges.cbegin()) {
            const auto range = *std::prev(nextRange);
            if (const auto continuation = source.rangeContinuation(*range, from)) {
                exceptions.insert(continuation->recurrenceId(), continuation);
            }
        }
    }
    const auto end = exceptions.cend();
    auto entry = from.isValid() ? std::as_const(exceptions).lowerBound(from) : exceptions.cbegin();
    if (entry != end && (!anchor.isValid() || !dtStart().isValid())) {
        return false;
    }
    const auto sourceAnchor = source.allDay() ? anchor : anchor.toTimeZone(source.dtStart().timeZone());
    const bool rebase = sourceAnchor != dtStart() || sourceAnchor.timeZone() != dtStart().timeZone() || source.allDay() != allDay();
    QMap<QDateTime, CalendarCore::CalEvent::Ptr> inherited;
    for (; entry != end; ++entry) {
        const auto oldId = entry.key();
        const auto newId = CalendarCore::RepeatInfo::rebaseRecurrenceId(oldId, sourceAnchor, dtStart(), source.allDay(), allDay());
        if (!newId.isValid() || inherited.contains(newId)) {
            return false;
        }
        const auto exception = CalendarCore::CalEvent::Ptr::create(*entry.value());
        if (rebase) {
            if (exception->thisAndFuture() && exception->dtStart().isValid() && source.allDay() == allDay()) {
                const auto shifted = source.rangeOccurrence(*exception, newId);
                exception->setDtStart(shifted->dtStart());
                exception->setDtEnd(shifted->dtEnd());
            } else if (exception->dtStart() == oldId && exception->allDay() == allDay()) {
                const auto duration = exception->dtStart().msecsTo(exception->dtEnd());
                const auto days = exception->dtStart().date().daysTo(exception->dtEnd().date());
                exception->setDtStart(newId);
                exception->setDtEnd(allDay() ? newId.addDays(days) : newId.addMSecs(duration));
            }
            exception->setRecurrenceId(newId, allDay());
        }
        inherited.insert(newId, exception);
    }
    // A split carries only the exceptions at or after the selected slot.
    clearRecurrenceExceptions();
    for (const auto &exception : inherited) {
        setRecurrenceException(*exception);
    }
    return true;
}

bool DSchedule::titleMatches(const QString &key) const
{
    return key.isEmpty() || summary().contains(key, Qt::CaseInsensitive)
        || (pinyinsearch::CanQueryByPinyin(key)
            && pinyinsearch::getPinPinSearch()->PinyinMatch(summary(), key.trimmed().toLower()));
}

bool DSchedule::hasRecurrenceOverride(const QDateTime &id) const
{
    return hasRecurrenceException(id) || hasRecurrenceRangeAt(id);
}

bool DSchedule::hasRecurrenceRangeAt(const QDateTime &id) const
{
    return id.isValid() && !m_recurrenceRanges.isEmpty() && id >= m_recurrenceRanges.firstKey();
}

bool DSchedule::hasRecurrenceException(const QDateTime &id) const
{
    return m_recurrenceExceptions.contains(id);
}

CalendarCore::CalEvent::List DSchedule::recurrenceExceptions() const
{
    CalendarCore::CalEvent::List events;
    events.reserve(m_recurrenceExceptions.size());
    for (const auto &exception : m_recurrenceExceptions) {
        auto event = CalendarCore::CalEvent::Ptr::create(*exception);
        event->setSchedulingID(schedulingID(), uid());
        events.append(event);
    }
    return events;
}

DSchedule::Ptr DSchedule::makeOccurrence(const CalendarCore::CalEvent &event) const
{
    auto occurrence = DSchedule::Ptr::create(event);
    occurrence->setSchedulingID(schedulingID(), uid());
    occurrence->setScheduleTypeID(scheduleTypeID());
    occurrence->setAccountColor(accountColor());
    return occurrence;
}

QList<QDateTime> DSchedule::recurrenceStarts(const QDateTime &from, const QDateTime &through) const
{
    if (!from.isValid() || !through.isValid() || from > through) {
        return {};
    }
    const auto rule = recurrence()->defaultRRuleConst();
    if (!lunar() || !rule) {
        return recurrence()->timesInInterval(from, through);
    }

    LunarDateInfo dates(rule, dtStart().secsTo(dtEnd()));
    QList<QDateTime> starts;
    const auto days = dates.getRRuleStartDate(from.date(), through.date(), dtStart().date());
    for (const auto &day : days) {
        const QDateTime start(day, dtStart().time());
        if (start >= from && start <= through && !recurrence()->exDates().contains(day)
            && !recurrence()->exDateTimes().contains(start)) {
            starts.append(start);
        }
    }
    return starts;
}

DSchedule::Ptr DSchedule::rangeContinuation(const CalendarCore::CalEvent &range, const QDateTime &from) const
{
    const auto nextRange = m_recurrenceRanges.upperBound(range.recurrenceId());
    const int lastYear = lunar() ? 2100 : 9999;
    const auto end = nextRange == m_recurrenceRanges.cend()
        ? QDateTime(QDate(lastYear, 12, 31), QTime(23, 59, 59), from.timeZone()) : nextRange.key().addMSecs(-1);
    for (const auto &id : recurrenceStarts(from, end)) {
        if (!hasRecurrenceException(id)) {
            auto continuation = rangeOccurrence(range, id);
            continuation->setThisAndFuture(true);
            return continuation;
        }
    }
    return {};
}

DSchedule::Ptr DSchedule::rangeOccurrence(const CalendarCore::CalEvent &range, const QDateTime &id) const
{
    auto instance = makeOccurrence(range);
    instance->setRecurrenceId(id, allDay());
    instance->setThisAndFuture(false);
    instance->clearRepeat();
    const auto anchor = range.recurrenceId();
    const auto shift = [&](const QDateTime &time) {
        if (!time.isValid()) {
            return QDateTime();
        }
        if (allDay() && range.allDay()) {
            return id.addDays(anchor.date().daysTo(time.date()));
        }
        return id.addMSecs(anchor.msecsTo(time)).toTimeZone(time.timeZone());
    };
    instance->setDtStart(shift(range.dtStart()));
    instance->setDtEnd(shift(range.dtEnd()));
    return instance;
}

DSchedule::Ptr DSchedule::recurrenceException(const QDateTime &id) const
{
    if (const auto exception = m_recurrenceExceptions.value(id)) {
        return exception->cancelled() ? Ptr() : makeOccurrence(*exception);
    }
    if (!id.isValid() || m_recurrenceRanges.isEmpty()) {
        return {};
    }
    auto entry = m_recurrenceRanges.upperBound(id);
    if (entry == m_recurrenceRanges.cbegin()) {
        return {};
    }
    const auto &range = *--entry;
    if (range->cancelled() || !range->dtStart().isValid() || !recurrenceStarts(id, id).contains(id)) {
        return {};
    }
    return rangeOccurrence(*range, id);
}

DSchedule::List DSchedule::recurrenceExceptionsInInterval(const QDateTime &from, const QDateTime &through) const
{
    List matches;
    const auto overlaps = [&](const CalendarCore::CalEvent &event) {
        auto begin = from;
        if (event.allDay()) {
            begin.setTime(QTime(0, 0));
        }
        return !event.cancelled() && event.dtStart().isValid() && event.dtStart() <= through && event.dtEnd() >= begin;
    };
    for (const auto &event : m_recurrenceExceptions) {
        if (overlaps(*event)) {
            matches.append(makeOccurrence(*event));
        }
    }
    for (auto entry = m_recurrenceRanges.cbegin(); entry != m_recurrenceRanges.cend(); ++entry) {
        const auto &range = *entry;
        if (range->cancelled() || !range->dtStart().isValid()) {
            continue;
        }
        const auto anchor = entry.key();
        const auto next = std::next(entry);
        auto begin = from;
        auto end = through;
        if (allDay() && range->allDay()) {
            begin = QDateTime(from.date(), QTime(0, 0), anchor.timeZone()).addDays(range->dtEnd().date().daysTo(anchor.date()));
            end = QDateTime(through.date(), QTime(23, 59, 59, 999), anchor.timeZone()).addDays(range->dtStart().date().daysTo(anchor.date()));
        } else {
            begin = from.addMSecs(range->dtEnd().msecsTo(anchor));
            end = through.addMSecs(range->dtStart().msecsTo(anchor));
        }
        // Expand in the original time window so moved occurrences can enter the visible window.
        begin = qMax(begin, anchor.addMSecs(1));
        if (next != m_recurrenceRanges.cend()) {
            end = qMin(end, next.key().addMSecs(-1));
        }
        for (const auto &id : recurrenceStarts(begin, end)) {
            if (!hasRecurrenceException(id)) {
                auto instance = rangeOccurrence(*range, id);
                if (overlaps(*instance)) {
                    matches.append(instance);
                }
            }
        }
    }
    return matches;
}

bool DSchedule::hasEnabledAlarms() const
{
    if (CalEvent::hasEnabledAlarms()) {
        return true;
    }
    for (const auto &exception : m_recurrenceExceptions) {
        if (!exception->cancelled() && exception->hasEnabledAlarms()) {
            return true;
        }
    }
    return false;
}

QMap<QDate, DSchedule::List> DSchedule::fromMapString(const QString &json)
{
    QMap<QDate, DSchedule::List> scheduleMap;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(json.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "Failed to parse schedule map JSON. Error:" << jsonError.errorString();
        return scheduleMap;
    }

    if (!jsonDoc.isArray()) {
        qCWarning(CommonLogger) << "Schedule map JSON root is not an array";
        return scheduleMap;
    }

    const QJsonArray rootArray = jsonDoc.array();
    for (const QJsonValue &jsonValue : rootArray) {
        if (!jsonValue.isObject()) {
            qCWarning(CommonLogger) << "Skipping non-object schedule map entry";
            continue;
        }

        const QJsonObject jsonObj = jsonValue.toObject();
        if (!jsonObj.value(QStringLiteral("Date")).isString()) {
            qCWarning(CommonLogger) << "Skipping schedule map entry without a valid Date string";
            continue;
        }
        const QDate date = dateFromString(jsonObj.value(QStringLiteral("Date")).toString());
        if (!date.isValid() || !jsonObj.value(QStringLiteral("schedule")).isArray()) {
            qCWarning(CommonLogger) << "Skipping schedule map entry with invalid date or schedule list";
            continue;
        }

        DSchedule::List &scheduleList = scheduleMap[date];
        for (const QJsonValue &scheduleValue : jsonObj.value(QStringLiteral("schedule")).toArray()) {
            if (!scheduleValue.isString()) {
                qCWarning(CommonLogger) << "Skipping non-string schedule entry";
                continue;
            }
            DSchedule::Ptr schedule;
            if (DSchedule::fromJsonString(schedule, scheduleValue.toString())) {
                scheduleList.append(schedule);
            } else {
                qCWarning(CommonLogger) << "Skipping invalid schedule entry";
            }
        }
    }

    return scheduleMap;
}

QString DSchedule::toMapString(const QMap<QDate, DSchedule::List> &scheduleMap)
{
    QJsonArray rootArray;
    QMap<QDate, DSchedule::List>::const_iterator iter = scheduleMap.constBegin();
    for (; iter != scheduleMap.constEnd(); ++iter) {
        QJsonObject jsonObj;
        jsonObj.insert("Date", dateToString(iter.key()));
        QJsonArray jsonArray;
        foreach (auto &schedule, iter.value()) {
            QString scheduleStr;
            DSchedule::toJsonString(schedule, scheduleStr);
            jsonArray.append(scheduleStr);
        }
        jsonObj.insert("schedule", jsonArray);
        rootArray.append(jsonObj);
    }
    QJsonDocument jsonDoc;
    jsonDoc.setArray(rootArray);
    return QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
}

QPair<QString, DSchedule::List> DSchedule::fromListString(const QString &json)
{
    QPair<QString, DSchedule::List> schedulePair;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(json.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "error:" << jsonError.errorString();
        return schedulePair;
    }

    if (!jsonDoc.isObject()) {
        qCWarning(CommonLogger) << "Schedule list JSON root is not an object";
        return schedulePair;
    }

    const QJsonObject jsonObj = jsonDoc.object();
    DSchedule::List scheduleList;
    if (!jsonObj.value(QStringLiteral("query")).isString()
        || !jsonObj.value(QStringLiteral("schedules")).isArray()) {
        qCWarning(CommonLogger) << "Schedule list JSON is missing query or schedules";
        return schedulePair;
    }

    schedulePair.first = jsonObj.value(QStringLiteral("query")).toString();
    for (const QJsonValue &scheduleValue : jsonObj.value(QStringLiteral("schedules")).toArray()) {
        if (!scheduleValue.isString()) {
            qCWarning(CommonLogger) << "Skipping non-string schedule entry";
            continue;
        }
        DSchedule::Ptr schedule;
        if (!DSchedule::fromJsonString(schedule, scheduleValue.toString())) {
            qCWarning(CommonLogger) << "Skipping invalid schedule entry";
            continue;
        }
        scheduleList.append(schedule);
    }
    schedulePair.second = scheduleList;

    return schedulePair;
}

QString DSchedule::toListString(const QString &query, const DSchedule::List &scheduleList)
{
    QJsonObject jsonObj;
    jsonObj.insert("query", query);
    QJsonArray jsonArray;
    foreach (auto &schedule, scheduleList) {
        QString scheduleStr;
        DSchedule::toJsonString(schedule, scheduleStr);
        jsonArray.append(scheduleStr);
    }
    jsonObj.insert("schedules", jsonArray);

    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObj);
    return QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
}

void DSchedule::expendRecurrence(DSchedule::Map &scheduleMap, const DSchedule::Ptr &schedule, const QDateTime &dtStart, const QDateTime &dtEnd)
{
    QDateTime queryDtStart = dtStart;
    //如果日程为全天日程，则查询的开始时间设置为0点，因为全天日程的开始和结束时间都是0点
    if(schedule->allDay()){
        queryDtStart.setTime(QTime(0,0,0));
    }

    if (schedule->cancelled()) {
        return;
    }
    if (hasUsableRecurrence(schedule)) {
        //获取日程的开始结束时间差
        qint64 interval = schedule->dtStart().secsTo(schedule->dtEnd());
        QList<QDateTime> dtList = schedule->recurrence()->timesInInterval(queryDtStart, dtEnd);

        foreach (auto &dt, dtList) {
            if (schedule->hasRecurrenceOverride(dt)) {
                continue;
            }
            QDateTime scheduleDtEnd = dt.addSecs(interval);
            DSchedule::Ptr newSchedule = DSchedule::Ptr(schedule->clone());
            if (dt != schedule->dtStart()) {
                newSchedule->clearRecurrenceExceptions();
            }
            newSchedule->setDtStart(dt);
            newSchedule->setDtEnd(scheduleDtEnd);

            //只有重复日程设置RecurrenceId
            if (schedule->dtStart() != dt) {
                newSchedule->setRecurrenceId(dt);
            }
            scheduleMap[displayDate(dt, schedule->allDay())].append(newSchedule);
        }
    } else {

        if (!schedule->hasRecurrenceException(schedule->dtStart())
            && !(schedule->dtStart() > dtEnd || schedule->dtEnd() < queryDtStart)) {
            scheduleMap[displayDate(schedule->dtStart(), schedule->allDay())].append(schedule);
        }
    }
}

QMap<QDate, DSchedule::List> DSchedule::convertSchedules(const DScheduleQueryPar::Ptr &queryPar, const DSchedule::List &scheduleList)
{
    QDateTime dtStart = queryPar->dtStart();
    QDateTime dtEnd = queryPar->dtEnd();
    bool extend = queryPar->queryType() == DScheduleQueryPar::Query_None;

    const auto queryType = queryPar->queryType();
    const QString searchKey = (queryType == DScheduleQueryPar::Query_None || queryType == DScheduleQueryPar::Query_Top)
        ? queryPar->key() : QString();
    QMap<QDate, DSchedule::List> scheduleMap;
    foreach (auto &schedule, scheduleList) {
        if (schedule->cancelled() && !schedule->hasRecurrenceId()) {
            continue;
        }
        for (const auto &exception : schedule->recurrenceExceptionsInInterval(dtStart, dtEnd)) {
            if (!exception->titleMatches(searchKey)) {
                continue;
            }
            const auto startDay = displayDate(exception->dtStart(), exception->allDay());
            const auto endDay = extend ? qMin(displayDate(exception->dtEnd(), exception->allDay()),
                                              displayDate(dtEnd, exception->allDay())) : startDay;
            const auto firstDay = extend ? qMax(startDay, displayDate(dtStart, exception->allDay())) : startDay;
            for (auto day = firstDay; day <= endDay && day.isValid(); day = day.addDays(1)) {
                scheduleMap[day].append(exception);
            }
        }
        if (schedule->cancelled() || (schedule->hasRecurrenceExceptions() && !schedule->titleMatches(searchKey))) {
            continue;
        }
        //获取日程的开始结束时间差
        qint64 interval = schedule->dtStart().secsTo(schedule->dtEnd());
        //如果存在重复日程
        if (hasUsableRecurrence(schedule)) {

            //如果为农历日程
            if (schedule->lunar() && schedule->recurrence()->defaultRRuleConst()) {

                //农历重复日程计算
                LunarDateInfo lunardate(schedule->recurrence()->defaultRRuleConst(), interval);

                QMap<int, QDate> ruleStartDate = lunardate.getRRuleStartDate(dtStart.date(), dtEnd.date(), schedule->dtStart().date());

                QDateTime recurDateTime;
                recurDateTime.setTime(schedule->dtStart().time());
                QDateTime copyEnd;
                QMap<int, QDate>::ConstIterator iter = ruleStartDate.constBegin();
                for (; iter != ruleStartDate.constEnd(); iter++) {
                    recurDateTime.setDate(iter.value());
                    //如果在忽略时间列表中,则忽略
                    if (schedule->hasRecurrenceOverride(recurDateTime)
                        || schedule->recurrence()->exDateTimes().contains(recurDateTime))
                        continue;
                    copyEnd = recurDateTime.addSecs(interval);
                    DSchedule::Ptr newSchedule = DSchedule::Ptr(new DSchedule(*schedule.data()));
                    if (recurDateTime != schedule->dtStart()) {
                        newSchedule->clearRecurrenceExceptions();
                    }
                    newSchedule->setDtStart(recurDateTime);
                    newSchedule->setDtEnd(copyEnd);
                    //只有重复日程设置RecurrenceId
                    if (schedule->dtStart() != recurDateTime) {
                        newSchedule->setRecurrenceId(recurDateTime);
                    }
                    scheduleMap[displayDate(recurDateTime, schedule->allDay())].append(newSchedule);
                }
            } else {

                //非农历日程
                expendRecurrence(scheduleMap, schedule, dtStart, dtEnd);
            }
        } else {

            //普通日程
            //如果在查询时间范围内
            QDateTime queryDtStart = dtStart;
            //如果日程为全天日程，则查询的开始时间设置为0点，因为全天日程的开始和结束时间都是0点
            if(schedule->allDay()){
                queryDtStart.setTime(QTime(0,0,0));
            }
            if (!schedule->hasRecurrenceException(schedule->dtStart())
                && !(schedule->dtEnd() < queryDtStart || schedule->dtStart() > dtEnd)) {
                const QDate displayStartDate = displayDate(schedule->dtStart(), schedule->allDay());
                const QDate displayEndDate = displayDate(schedule->dtEnd(), schedule->allDay());
                if (extend && displayStartDate != displayEndDate) {

                    //需要扩展的天数
                    const int extenddays = displayStartDate.daysTo(displayEndDate);
                    for (int i = 0; i <= extenddays; ++i) {
                        // 开始结束日期已经在查询时间范围内
                        scheduleMap[displayStartDate.addDays(i)].append(schedule);
                    }
                } else {
                    scheduleMap[displayStartDate].append(schedule);
                }
            }
        }
    }

    //如果为查询前N个日程，则取前N个日程
    if (queryPar->queryType() == DScheduleQueryPar::Query_Top) {

        int scheduleNum = 0;
        DSchedule::Map filterSchedule;
        DSchedule::Map::const_iterator iter = scheduleMap.constBegin();
        for (; iter != scheduleMap.constEnd(); ++iter) {
            if (iter.value().size() == 0) {
                continue;
            }
            if (scheduleNum + iter.value().size() > queryPar->queryTop()) {
                DSchedule::List scheduleList;
                int residuesNum = queryPar->queryTop() - scheduleNum;
                for (int i = 0; i < residuesNum; ++i) {
                    scheduleList.append(iter.value().at(i));
                }
                filterSchedule[iter.key()] = scheduleList;
            } else {
                filterSchedule[iter.key()] = iter.value();
            }
        }
        scheduleMap = filterSchedule;
    }

    return scheduleMap;
}

QMap<QDate, DSchedule::List> DSchedule::fromQueryResult(const QString &query)
{
    QMap<QDate, DSchedule::List> scheduleMap;
    QPair<QString, DSchedule::List> pair = fromListString(query);
    DScheduleQueryPar::Ptr queryPar = DScheduleQueryPar::fromJsonString(pair.first);
    if (queryPar.isNull()) {
        qCWarning(CommonLogger) << "Failed to parse query parameters from query result.";
        return scheduleMap;
    }

    scheduleMap = DSchedule::convertSchedules(queryPar, pair.second);
    return scheduleMap;
}

bool operator==(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2)
{
    return s1.isNull() || s2.isNull() ? s1.isNull() && s2.isNull() : s1->instanceIdentifier() == s2->instanceIdentifier();
}

QMap<int, DSchedule::AlarmType> DSchedule::getAlarmMap()
{
    static QMap<int, DSchedule::AlarmType> alarmMap {
        {0, Alarm_Begin},
        {-15 * Duration_Min, Alarm_15Min_Front},
        {-30 * Duration_Min, Alarm_30Min_Front},
        {-Duration_Hour, Alarm_1Hour_Front},
        {-Duration_Day, Alarm_1Day_Front},
        {-Duration_Day * 2, Alarm_2Day_Front},
        {-Duration_Week, Alarm_1Week_Front},
        {9 * Duration_Hour, Alarm_9Hour_After},
        {-15 * Duration_Hour, Alarm_15Hour_Front},
        {-39 * Duration_Hour, Alarm_39Hour_Front},
        {-159 * Duration_Hour, Alarm_159Hour_Front}};
    return alarmMap;
}

QString DSchedule::fileName() const
{
    return m_fileName;
}

void DSchedule::setFileName(const QString &fileName)
{
    m_fileName = fileName;
}

int DSchedule::compatibleID() const
{
    return m_compatibleID;
}

void DSchedule::setcompatibleID(int compatibleID)
{
    m_compatibleID = compatibleID;
}

QDebug operator<<(QDebug debug, const DSchedule &scheduleJsonData)
{
    QDebugStateSaver saver(debug);
    debug.noquote() << "dtStart:" << dtToString(scheduleJsonData.dtStart())
                    << " ,dtEnd:" << dtToString(scheduleJsonData.dtEnd())
                    << " ,dtCreate:" << dtToString(scheduleJsonData.created())
                    << " ,summary:" << scheduleJsonData.summary()
                    << " ,scheduleTypeID:" << scheduleJsonData.scheduleTypeID()
                    << ",Uid:" << scheduleJsonData.uid()
                    << " ,rrule:" << CalendarCore::IcsCodec().toString(scheduleJsonData.recurrence()->defaultRRuleConst());
    return debug;
}
