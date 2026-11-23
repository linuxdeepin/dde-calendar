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

#include <memory>

#include "calstore.h"
#include "calevent.h"
#include "memcalstore.h"
#include "icscodec.h"

#include "repeatinfo.h"

#include "golden_util.h"

namespace CK = CalendarCore;

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

    CK::MemCalStore::Ptr cal(new CK::MemCalStore(QTimeZone("Asia/Shanghai")));
    CK::IcsCodec format;
    if (!format.fromString(cal, icsContent)) return QString();

    CK::CalEvent::List events = cal->events();
    QString result;
    QTextStream stream(&result);

    for (const CK::CalEvent::Ptr &event : events) {
        stream << "Event:" << event->uid() << "\n";

        CK::RepeatInfo *rec = event->recurrence();
        if (rec && rec->defaultRRuleConst()) {
            stream << "  rrule:" << rec->defaultRRuleConst()->periodType() << "\n";
            stream << "  rruleStr:" << format.toString(rec->defaultRRuleConst()) << "\n";
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

    expectMatchesGolden(GetParam() + ".rrule.golden", result);
}

TEST(RruleSummary, CountBeyondTenThousandRemainsQueryable)
{
    CK::RRule rule;
    ASSERT_TRUE(rule.parse(QStringLiteral("FREQ=DAILY;COUNT=10005")));
    rule.setStartDt(QDateTime(QDate(2000, 1, 1), QTime(9, 0), Qt::UTC));
    const QDateTime from(QDate(2027, 5, 20), QTime(0, 0), Qt::UTC);
    const auto dates = rule.timesInInterval(from, from.addDays(3));
    ASSERT_EQ(dates.size(), 3);
    EXPECT_EQ(dates.first().date(), from.date());
    EXPECT_TRUE(rule.endDt().isValid());
    EXPECT_EQ(rule.endDt().date(), QDate(2027, 5, 23));
}


TEST(RruleSummary, PreservesLegacyRecurrenceTypeClassification)
{
    const struct {
        const char *rrule;
        int type;
    } cases[] = {
        { "FREQ=DAILY", 3 },
        { "FREQ=DAILY;BYHOUR=9,17", 10 },
        { "FREQ=DAILY;BYDAY=MO,TU,WE,TH,FR", 10 },
        { "FREQ=WEEKLY;BYDAY=MO,WE", 4 },
        { "FREQ=MONTHLY;BYDAY=1MO", 5 },
        { "FREQ=MONTHLY;BYMONTHDAY=1", 6 },
        { "FREQ=MONTHLY;BYDAY=MO;BYMONTHDAY=1", 10 },
        { "FREQ=YEARLY;BYYEARDAY=100", 8 },
        { "FREQ=YEARLY;BYMONTH=4;BYYEARDAY=100", 10 },
        { "FREQ=YEARLY;BYMONTH=4;BYDAY=2MO", 9 },
    };
    for (const auto &test : cases) {
        SCOPED_TRACE(test.rrule);
        auto rule = std::make_unique<CK::RRule>();
        ASSERT_TRUE(rule->parse(QString::fromLatin1(test.rrule)));
        CK::RepeatInfo recurrence;
        recurrence.addRRule(rule.release());
        EXPECT_EQ(recurrence.periodType(), test.type);
    }
}

INSTANTIATE_TEST_SUITE_P(Fixtures, RruleSummaryTest,
    ::testing::ValuesIn(listFixtureFiles()));
