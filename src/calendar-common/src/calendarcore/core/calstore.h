// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "calevent.h"

#include <QHash>

namespace CalendarCore {

class CalStore : public PropertyBag
{
public:
    using Ptr = QSharedPointer<CalStore>;
    explicit CalStore(const QTimeZone &zone) : m_zone(zone) {}
    explicit CalStore(std::nullptr_t) : CalStore(QTimeZone::systemTimeZone()) {}

    QTimeZone timeZone() const { return m_zone; }
    CalEvent::List events() const { return m_events; }
    CalEvent::List events(const QDate &from, const QDate &through, const QTimeZone &zone = {}) const;
    // Keep UID and recurrence ID stable while the event is stored. A duplicate replaces it only at a higher revision.
    bool addEvent(const CalEvent::Ptr &event);
    void removeEvent(const QString &uid, const QDateTime &recurrenceId);

private:
    QTimeZone m_zone;
    CalEvent::List m_events;
    QHash<QString, QHash<QDateTime, qsizetype>> m_eventIndices;
};

} // namespace CalendarCore
