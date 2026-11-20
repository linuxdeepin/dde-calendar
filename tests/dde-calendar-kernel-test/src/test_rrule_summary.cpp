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

#include "recurrence.h"

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

static QString formatRruleSummary(const QString &fixtureName)
{
    QString fixturePath = QString(FIXTURES_DIR) + "/" + fixtureName;
    QString icsContent = loadFile(fixturePath);
    if (icsContent.isEmpty()) return QString();

    CK::MemoryCalendar::Ptr cal(new CK::MemoryCalendar(QTimeZone("Asia/Shanghai")));
    CK::ICalFormat format;
    if (!format.fromString(cal.data(), icsContent)) return QString();

    CK::Event::List events = cal->events();
    QString result;
    QTextStream stream(&result);

    for (const CK::Event::Ptr &event : events) {
        stream << "Event:" << event->uid() << "\n";

        CK::Recurrence *rec = event->recurrence();
        if (rec && rec->defaultRRuleConst()) {
            stream << "  rrule:" << rec->defaultRRuleConst()->recurrenceType() << "\n";
            stream << "  rruleStr:" << CK::ICalFormat::toString(rec->defaultRRuleConst()) << "\n";
        } else {
            stream << "  rrule:none\n";
        }

        stream << "  duration:" << event->duration().asSeconds() << "\n";
        stream << "  endDt:" << event->dtEnd().toString(Qt::ISODate) << "\n";
        stream << "\n";
    }

    stream.flush();
    return result;
}

class RruleSummaryTest : public ::testing::TestWithParam<QString> {};

TEST_P(RruleSummaryTest, RruleDurationEndDt)
{
    QString result = formatRruleSummary(GetParam());
    ASSERT_FALSE(result.isEmpty()) << "Failed to process fixture: " << GetParam().toStdString();

    QString goldenKey = GetParam() + ".rrule.golden";

}

INSTANTIATE_TEST_SUITE_P(Fixtures, RruleSummaryTest,
    ::testing::ValuesIn(listFixtureFiles()));
