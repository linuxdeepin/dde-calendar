// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendarglobalenv.h"
#include "commondef.h"

CalendarGlobalEnv *CalendarGlobalEnv::getGlobalEnv()
{
    static CalendarGlobalEnv globalEnv;
    return &globalEnv;
}

CalendarGlobalEnv *CalendarGlobalEnv::operator->() const
{
    return getGlobalEnv();
}

//注册关键字
bool CalendarGlobalEnv::registerKey(const QString &key, const QVariant &variant)
{
    qCDebug(ClientLogger) << "CalendarGlobalEnv::registerKey - Registering key:" << key;
    //如果不包含则添加
    if (!m_GlobalEnv.contains(key)) {
        m_GlobalEnv[key] = variant;
        qCDebug(ClientLogger) << "Key registered successfully";
        return true;
    }
    qCDebug(ClientLogger) << "Key already exists, registration failed";
    return false;
}

//修改值
bool CalendarGlobalEnv::reviseValue(const QString &key, const QVariant &variant)
{
    qCDebug(ClientLogger) << "CalendarGlobalEnv::reviseValue - Revising value for key:" << key;
    //如果包含，则修改
    if (m_GlobalEnv.contains(key)) {
        m_GlobalEnv[key] = variant;
        qCDebug(ClientLogger) << "Value revised successfully";
        return true;
    }
    qCDebug(ClientLogger) << "Key not found, revision failed";
    return false;
}

//移除注册的关键字
bool CalendarGlobalEnv::removeKey(const QString &key)
{
    qCDebug(ClientLogger) << "CalendarGlobalEnv::removeKey - Removing key:" << key;
    //如果包含则移除
    if (m_GlobalEnv.contains(key)) {
        m_GlobalEnv.remove(key);
        qCDebug(ClientLogger) << "Key removed successfully";
        return true;
    }
    qCDebug(ClientLogger) << "Key not found, removal failed";
    return false;
}

//根据关键字获取对应的值
bool CalendarGlobalEnv::getValueByKey(const QString &key, QVariant &variant)
{
    qCDebug(ClientLogger) << "CalendarGlobalEnv::getValueByKey - Getting value for key:" << key;
    if (m_GlobalEnv.contains(key)) {
        variant = m_GlobalEnv.value(key);
        qCDebug(ClientLogger) << "Value retrieved successfully";
        return true;
    }
    qCDebug(ClientLogger) << "Key not found, retrieval failed";
    return false;
}

CalendarGlobalEnv::CalendarGlobalEnv()
    : m_GlobalEnv{}
{
    qCDebug(ClientLogger) << "CalendarGlobalEnv constructor initialized";
}
