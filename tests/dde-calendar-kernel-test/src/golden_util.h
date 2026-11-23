// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>
#include <QFile>
#include <QSaveFile>
#include <QString>

// golden 基准读写工具：
// - 设置环境变量 KERNEL_GOLDEN_GEN=1 运行时，为每个用例生成 golden 文件（作为基准提交）
// - 默认运行时，将实际输出与 golden 文件逐字节对比
// 对比前做归一化：PRODID 行与解析期自动生成的时间戳属性（DTSTAMP/CREATED/LAST-MODIFIED，
// 其值为当前时间、逐次运行不同）不参与对比。VTIMEZONE 只比较 TZID，规则由 libical 时区库提供。

inline QString normalizeIcs(const QString &ics)
{
    static const char *volatileProps[] = {"PRODID:", "DTSTAMP:", "CREATED:", "LAST-MODIFIED:"};
    QStringList result;
    bool inTimeZone = false;
    const auto lines = ics.split(QLatin1Char('\n'));
    for (QString line : lines) {
        if (line.trimmed() == QLatin1String("BEGIN:VTIMEZONE")) {
            inTimeZone = true;
        } else if (line.trimmed() == QLatin1String("END:VTIMEZONE")) {
            inTimeZone = false;
        } else if (inTimeZone && !line.startsWith(QLatin1String("TZID:"))) {
            continue;
        }
        for (const char *prop : volatileProps) {
            if (line.startsWith(QLatin1String(prop), Qt::CaseInsensitive)) {
                line = QLatin1String(prop) + "EXEMPT";
                break;
            }
        }
        result.append(line);
    }
    return result.join(QLatin1Char('\n'));
}

// 与 golden 文件对比；gen 模式下生成 golden 文件
inline void expectMatchesGolden(const QString &name, const QString &actual)
{
    const bool generate = qgetenv("KERNEL_GOLDEN_GEN") == "1";
    const auto directory = generate ? qgetenv("KERNEL_GOLDEN_DIR") : QByteArray(GOLDEN_DIR);
    ASSERT_FALSE(directory.isEmpty()) << "Set KERNEL_GOLDEN_DIR to a writable output directory";
    const QString path = QString::fromLocal8Bit(directory) + "/" + name;
    if (generate) {
        QSaveFile f(path);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text))
            << "cannot write golden: " << path.toStdString();
        const QByteArray data = actual.toUtf8();
        ASSERT_EQ(f.write(data), data.size());
        ASSERT_TRUE(f.commit());
        GTEST_SUCCEED() << "golden generated: " << name.toStdString();
        return;
    }
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly)) << "golden file missing: " << path.toStdString();
    const QString expected = QString::fromUtf8(f.readAll());
    f.close();
    ASSERT_EQ(normalizeIcs(expected), normalizeIcs(actual))
        << "golden mismatch: " << name.toStdString();
}
