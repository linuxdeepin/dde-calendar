// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QTimeZone>

#include "calendar.h"
#include "event.h"
#include "memorycalendar.h"
#include "icalformat.h"

namespace CK = CalendarKernel;

static QString loadFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QTextStream(&f).readAll();
}

static QStringList listFixtureFiles()
{
    QDir dir(FIXTURES_DIR);
    QStringList filters;
    filters << "*.ics";
    return dir.entryList(filters, QDir::Files);
}

static QString formatExpansion(CK::MemoryCalendar::Ptr cal, const QString &fixtureName)
{
    QString fixturePath = QString(FIXTURES_DIR) + "/" + fixtureName;
    QString icsContent = loadFile(fixturePath);
    if (icsContent.isEmpty()) return QString();

    cal->clear();

    CK::ICalFormat format;
    if (!format.fromString(cal.data(), icsContent)) return QString();

    QDate startDate(2024, 1, 1);
    QDate endDate(2028, 12, 31);

    CK::Event::List events = cal->events();
    QString result;
    QTextStream stream(&result);

    for (const CK::Event::Ptr &event : events) {
        stream << "Event:" << event->uid() << "\n";
        stream << "Summary:" << event->summary() << "\n";

        QDateTime periodStart(startDate, QTime(0, 0), QTimeZone("Asia/Shanghai"));
        QDateTime periodEnd(endDate, QTime(23, 59, 59), QTimeZone("Asia/Shanghai"));

        CK::Event::List instances = cal->events(periodStart, periodEnd);
        for (const CK::Event::Ptr &inst : instances) {
            if (inst->uid() != event->uid()) continue;
            stream << "  " << inst->dtStart().toString(Qt::ISODate)
                   << "|" << (inst->allDay() ? "allday" : "timed")
                   << "|" << inst->instanceIdentifier() << "\n";
        }
        stream << "\n";
    }

    stream.flush();
    return result;
}

class RecurrenceExpansionTest : public ::testing::TestWithParam<QString> {};

TEST_P(RecurrenceExpansionTest, FiveYearExpansionTable)
{
    CK::MemoryCalendar::Ptr cal(new CK::MemoryCalendar(QTimeZone("Asia/Shanghai")));
    QString result = formatExpansion(cal, GetParam());
    ASSERT_FALSE(result.isEmpty()) << "Failed to process fixture: " << GetParam().toStdString();

    QString goldenKey = GetParam() + ".expansion.golden";

}

INSTANTIATE_TEST_SUITE_P(Fixtures, RecurrenceExpansionTest,
    ::testing::ValuesIn(listFixtureFiles()));
