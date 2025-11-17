// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CALENDARLOGGER_H
#define CALENDARLOGGER_H

#include <QObject>
#include <dtkcore_global.h>

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

class CalendarLogger : public QObject
{
    Q_OBJECT
public:
    explicit CalendarLogger(const QString &configId, QObject *parent = nullptr);
    ~CalendarLogger();

    static void initLogger();

    inline QString rules() const { return m_rules; }
    void setRules(const QString &rules);

private:
    void appendRules(const QString &rules);

private:
    QString m_rules;
    Dtk::Core::DConfig *m_config;
};

#endif // CALENDARLOGGER_H
