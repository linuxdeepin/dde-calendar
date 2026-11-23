// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "customproperties.h"
#include "timespan.h"

#include <QSharedPointer>
#include <QVector>

namespace CalendarCore {

class IcsCodec;

class Remind : public PropertyBag
{
public:
    enum Type : int { Invalid, Display, Procedure, Email, Audio };
    using Ptr = QSharedPointer<Remind>;
    using List = QVector<Ptr>;

    Type type() const { return m_type; }
    void setType(Type type) { m_type = type; }
    bool enabled() const { return m_enabled; }
    void setEnabled(bool isEnabled) { m_enabled = isEnabled; }
    QString text() const { return m_text; }
    void setText(const QString &text) { m_text = text; }
    void setDisplayAlarm(const QString &text) { setType(Display); setText(text); }
    TimeSpan startOffset() const { return m_offsetFromEnd ? TimeSpan() : m_offset; }
    void setStartOffset(TimeSpan offset) { setRelativeOffset(offset, false); }
    void setTime(const QDateTime &absolute) { m_absolute = absolute; m_offset = TimeSpan(); m_offsetFromEnd = false; }
    QDateTime absoluteTime() const { return m_absolute; }
    bool hasSameTrigger(const Remind &other) const
    {
        return m_enabled == other.m_enabled && m_type == other.m_type && m_absolute == other.m_absolute
            && m_offsetFromEnd == other.m_offsetFromEnd && m_offset.isDaily() == other.m_offset.isDaily()
            && m_offset.asSeconds() == other.m_offset.asSeconds();
    }
    QDateTime triggerTime(const QDateTime &start, const QDateTime &end) const
    {
        return m_absolute.isValid() ? m_absolute : m_offset.end(m_offsetFromEnd ? end : start);
    }

private:
    friend class IcsCodec;
    void setRelativeOffset(TimeSpan offset, bool fromEnd)
    {
        m_offset = offset;
        m_offsetFromEnd = fromEnd;
        m_absolute = {};
    }
    Type m_type = Invalid;
    bool m_enabled = false;
    QString m_text;
    QDateTime m_absolute;
    TimeSpan m_offset;
    bool m_offsetFromEnd = false;
    QList<QByteArray> m_extraProperties;
};

} // namespace CalendarCore
