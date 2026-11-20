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

static QString prodidExempt(const QString &ics)
{
    QStringList lines = ics.split('\n');
    for (QString &line : lines) {
        if (line.startsWith("PRODID:", Qt::CaseInsensitive)) {
            line = "PRODID:EXEMPT";
        }
    }
    return lines.join('\n');
}

static QStringList listFixtureFiles()
{
    QDir dir(FIXTURES_DIR);
    QStringList filters;
    filters << "*.ics";
    return dir.entryList(filters, QDir::Files);
}

static QString loadFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QTextStream(&f).readAll();
}

class IcsRoundtripTest : public ::testing::TestWithParam<QString> {};

TEST_P(IcsRoundtripTest, ByteLevelRoundtrip)
{
    QString fixturePath = QString(FIXTURES_DIR) + "/" + GetParam();
    QString icsContent = loadFile(fixturePath);
    ASSERT_FALSE(icsContent.isEmpty()) << "Failed to load fixture: " << GetParam().toStdString();

    CK::MemoryCalendar::Ptr cal(new CK::MemoryCalendar(QTimeZone("Asia/Shanghai")));
    CK::ICalFormat format;
    ASSERT_TRUE(format.fromString(cal.data(), icsContent)) << "Parse failed: " << GetParam().toStdString();

    QString output = format.toString(cal.data());
    ASSERT_FALSE(output.isEmpty()) << "toString returned empty for: " << GetParam().toStdString();

    QString exemptedOutput = prodidExempt(output);

}

INSTANTIATE_TEST_SUITE_P(Fixtures, IcsRoundtripTest,
    ::testing::ValuesIn(listFixtureFiles()));
