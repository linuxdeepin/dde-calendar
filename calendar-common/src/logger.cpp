// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logger.h"
#include "commondef.h"

#include <QLoggingCategory>
#include <QDebug>
#include <DConfig>
#include <DLog>

DCORE_USE_NAMESPACE

CalendarLogger::CalendarLogger(QObject *parent)
    : QObject(parent), m_rules(""), m_config(nullptr)
{
    QByteArray logRules = qgetenv("QT_LOGGING_RULES");
    // qunsetenv 之前一定不要有任何日志打印，否则取消环境变量设置不会生效
    qunsetenv("QT_LOGGING_RULES");

    // set env rules first
    m_rules = logRules;

    // read dconfig (which contains default configuration)
    m_config = DConfig::create("org.deepin.dde-calendar", "org.deepin.dde-calendar");
    if (m_config) {
        // DConfig contains default values, just read and append them
        logRules = m_config->value("log_rules").toByteArray();
        appendRules(logRules);
        setRules(m_rules);

        // watch dconfig changes
        connect(m_config, &DConfig::valueChanged, this, [this](const QString &key) {
            qCInfo(CommonLogger) << "DConfig value changed:" << key;
            if (key == "log_rules") {
                setRules(m_config->value(key).toByteArray());
            }
        });
    } else {
        qCWarning(CommonLogger) << "Failed to create DConfig for org.deepin.dde-calendar";
        setRules(m_rules);
    }
}

CalendarLogger::~CalendarLogger()
{
    if (m_config) {
        m_config->deleteLater();
    }
}

void CalendarLogger::initLogger()
{
    // set log format and register console and file appenders
    const QString logFormat = "%{time}{yy-MM-ddTHH:mm:ss.zzz} [%{type:-7}] [%{category}] <%{function}:%{line}> %{message}";
    DLogManager::setLogFormat(logFormat);

// 为了兼容性
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
    DLogManager::registerJournalAppender();
#endif

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
}

void CalendarLogger::setRules(const QString &rules)
{
    auto tmpRules = rules;
    m_rules = tmpRules.replace(";", "\n");
    QLoggingCategory::setFilterRules(m_rules);
}

void CalendarLogger::appendRules(const QString &rules)
{
    QString tmpRules = rules;
    tmpRules = tmpRules.replace(";", "\n");
    auto tmplist = tmpRules.split('\n');
    for (int i = 0; i < tmplist.count(); i++)
        if (m_rules.contains(tmplist.at(i))) {
            tmplist.removeAt(i);
            i--;
        }
    if (tmplist.isEmpty())
        return;
    m_rules.isEmpty() ? m_rules = tmplist.join("\n")
                      : m_rules += "\n" + tmplist.join("\n");
}
