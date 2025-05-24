// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendarglobalenv.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(calendarEnv, "calendar.environment")

CalendarGlobalEnv *CalendarGlobalEnv::getGlobalEnv()
{
    static CalendarGlobalEnv globalEnv;
    qCDebug(calendarEnv) << "Getting global environment instance";
    return &globalEnv;
}

CalendarGlobalEnv *CalendarGlobalEnv::operator->() const
{
    return getGlobalEnv();
}

//注册关键字
bool CalendarGlobalEnv::registerKey(const QString &key, const QVariant &variant)
{
    //如果不包含则添加
    if (!m_GlobalEnv.contains(key)) {
        m_GlobalEnv[key] = variant;
        qCDebug(calendarEnv) << "Registered new key:" << key << "with value:" << variant;
        return true;
    }
    qCDebug(calendarEnv) << "Failed to register key:" << key << "(already exists)";
    return false;
}

//修改值
bool CalendarGlobalEnv::reviseValue(const QString &key, const QVariant &variant)
{
    //如果包含，则修改
    if (m_GlobalEnv.contains(key)) {
        QVariant oldValue = m_GlobalEnv[key];
        m_GlobalEnv[key] = variant;
        qCDebug(calendarEnv) << "Updated key:" << key << "from:" << oldValue << "to:" << variant;
        return true;
    }
    qCWarning(calendarEnv) << "Failed to revise value for key:" << key << "(key not found)";
    return false;
}

//移除注册的关键字
bool CalendarGlobalEnv::removeKey(const QString &key)
{
    //如果包含则移除
    if (m_GlobalEnv.contains(key)) {
        m_GlobalEnv.remove(key);
        qCDebug(calendarEnv) << "Removed key:" << key;
        return true;
    }
    qCDebug(calendarEnv) << "Failed to remove key:" << key << "(key not found)";
    return false;
}

//根据关键字获取对应的值
bool CalendarGlobalEnv::getValueByKey(const QString &key, QVariant &variant)
{
    if (m_GlobalEnv.contains(key)) {
        variant = m_GlobalEnv.value(key);
        qCDebug(calendarEnv) << "Retrieved value for key:" << key << "value:" << variant;
        return true;
    }
    qCDebug(calendarEnv) << "Failed to get value for key:" << key << "(key not found)";
    return false;
}

CalendarGlobalEnv::CalendarGlobalEnv()
    : m_GlobalEnv{}
{
    qCDebug(calendarEnv) << "Initializing CalendarGlobalEnv";
}
