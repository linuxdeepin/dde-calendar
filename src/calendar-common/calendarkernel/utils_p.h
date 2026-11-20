// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALENDARKERNEL_UTILS_H
#define CALENDARKERNEL_UTILS_H

#include <QDateTime>

class QDataStream;

namespace CalendarKernel {
/**
 * Helpers to retain backwards compatibility of binary serialization.
 */
void serializeQDateTimeAsKDateTime(QDataStream &out, const QDateTime &dt);
Q_CORE_EXPORT void deserializeKDateTimeAsQDateTime(QDataStream &in, QDateTime &dt);

void serializeQDateTimeList(QDataStream &out, const QList<QDateTime> &list);
void deserializeQDateTimeList(QDataStream &in, QList<QDateTime> &list);

void serializeQTimeZoneAsSpec(QDataStream &out, const QTimeZone &tz);
void deserializeSpecAsQTimeZone(QDataStream &in, QTimeZone &tz);

} // namespace CalendarKernel

#endif
