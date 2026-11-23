// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "calstore.h"

using namespace CalendarCore;

bool CalStore::addEvent(const CalEvent::Ptr &event)
{
    if (!event || event->uid().isEmpty()) {
        return false;
    }
    auto &indices = m_eventIndices[event->uid()];
    const auto entry = indices.constFind(event->recurrenceId());
    if (entry != indices.cend()) {
        const auto index = entry.value();
        if (m_events.at(index)->revision() >= event->revision()) {
            return false;
        }
        m_events[index] = event;
        return true;
    }
    indices.insert(event->recurrenceId(), m_events.size());
    m_events.append(event);
    return true;
}

void CalStore::removeEvent(const QString &uid, const QDateTime &recurrenceId)
{
    auto series = m_eventIndices.find(uid);
    if (series == m_eventIndices.end()) {
        return;
    }
    auto entry = series->find(recurrenceId);
    if (entry == series->end()) {
        return;
    }
    const auto index = entry.value();
    series->erase(entry);
    if (series->isEmpty()) {
        m_eventIndices.erase(series);
    }
    m_events.removeAt(index);
    for (qsizetype i = index; i < m_events.size(); ++i) {
        const auto &event = m_events.at(i);
        m_eventIndices[event->uid()][event->recurrenceId()] = i;
    }
}

CalEvent::List CalStore::events(const QDate &from, const QDate &through, const QTimeZone &zone) const
{
    const auto effectiveZone = zone.isValid() ? zone : m_zone;
    auto dateInZone = [&](const QDateTime &dt, bool allDay) {
        return allDay ? dt.date() : dt.toTimeZone(effectiveZone).date();
    };
    CalEvent::List visible;
    for (const auto &event : m_events) {
        if (through.isValid() && dateInZone(event->dtStart(), event->allDay()) > through) {
            continue;
        }
        const QDate end = event->recurs() ? event->recurrence()->endDate()
                                         : dateInZone(event->dtEnd(), event->allDay());
        if (!from.isValid() || !end.isValid() || end >= from) {
            visible.append(event);
        }
    }
    return visible;
}
