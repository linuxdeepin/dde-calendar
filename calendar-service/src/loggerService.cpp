// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "loggerService.h"
#include "dtkcore_global.h"
#include "qglobal.h"

#include <QLoggingCategory>
#include <QObject>

#include <DConfig>

DCORE_USE_NAMESPACE

MLogger::MLogger(QObject *parent)
    : QObject(parent), m_rules(""), m_config(nullptr) {
  QByteArray logRules = qgetenv("QT_LOGGING_RULES");
  // qunsetenv 之前一定不要有任何日志打印，否则取消环境变量设置不会生效
  qunsetenv("QT_LOGGING_RULES");

  // set env
  m_rules = logRules;

  // set dconfig
  m_config = DConfig::create("org.deepin.dde.calendar", "org.deepin.dde.calendar.service");
  logRules = m_config->value("log_rules").toByteArray();
  appendRules(logRules);
  setRules(m_rules);

  // watch dconfig
  connect(m_config, &DConfig::valueChanged, this, [this](const QString &key) {
    qCritical() << "value changed:" << key;
    if (key == "log_rules") {
      setRules(m_config->value(key).toByteArray());
    }
  });
}

MLogger::~MLogger() { m_config->deleteLater(); }

void MLogger::setRules(const QString &rules) {
  auto tmpRules = rules;
  m_rules = tmpRules.replace(";", "\n");
  const QStringList ruleList = m_rules.split('\n');
  const bool debugEnabled = ruleList.contains("*.debug=true");
  const bool infoEnabled = ruleList.contains("*.info=true");

  // 显式覆盖：追加的 *.warning=true/*.info=true 会覆盖用户在配置中设置的 *.warning=false/*.info=false。
  // 因为 QLoggingCategory::setFilterRules 采用后匹配优先策略（last-match-wins），
  // 追加的规则位于末尾，会覆盖之前的所有同名 category 设置。这是预期行为。
  // System developer rules take precedence and use cumulative log levels.
  if (debugEnabled) {
    m_rules += "\n*.debug=true\n*.info=true\n*.warning=true";
  } else if (infoEnabled) {
    m_rules += "\n*.info=true\n*.warning=true";
  }
  QLoggingCategory::setFilterRules(m_rules);
}

void MLogger::appendRules(const QString &rules) {
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
