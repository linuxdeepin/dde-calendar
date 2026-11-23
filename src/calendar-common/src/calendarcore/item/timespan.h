// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QDateTime>

namespace CalendarCore {

class TimeSpan
{
public:
    explicit TimeSpan(qint64 seconds = 0) : m_value(seconds) {}
    static TimeSpan days(qint64 count)
    {
        TimeSpan span(count);
        span.m_days = true;
        return span;
    }
    qint64 asSeconds() const { return m_days ? m_value * 86400 : m_value; }
    qint64 asDays() const { return m_days ? m_value : m_value / 86400; }
    bool isDaily() const { return m_days; }
    QDateTime end(const QDateTime &start) const
    {
        return m_days ? start.addDays(m_value) : start.addSecs(m_value);
    }

private:
    qint64 m_value;
    bool m_days = false;
};

} // namespace CalendarCore
