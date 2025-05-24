// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduledatamanage.h"
#include "accountmanager.h"
#include "cscheduleoperation.h"
#include "accountmanager.h"

#include <QJsonArray>
#include <QJsonDocument>

Q_LOGGING_CATEGORY(scheduleDataLog, "calendar.schedule.data")

CScheduleDataManage *CScheduleDataManage::m_vscheduleDataManage = nullptr;

//
CSchedulesColor CScheduleDataManage::getScheduleColorByType(const QString &typeId)
{
    qCDebug(scheduleDataLog) << "Getting schedule color for type ID:" << typeId;
    CSchedulesColor color;
    DScheduleType::Ptr type = gAccountManager->getScheduleTypeByScheduleTypeId(typeId);
    QColor typeColor;
    if (nullptr != type) {
        typeColor = type->typeColor().colorCode();
    } else if (typeId =="other") {
        qCInfo(scheduleDataLog) << "Using default color for 'other' type";
        //如果类型不存在则设置一个默认颜色
        typeColor = QColor("#BA60FA");
    } else {
        qCWarning(scheduleDataLog) << "Schedule type not found:" << typeId;
    }

    color.orginalColor = typeColor;
    color.normalColor = color.orginalColor;
    color.normalColor.setAlphaF(0.2);
    color.pressColor = color.orginalColor;
    color.pressColor.setAlphaF(0.35);

    color.hoverColor = color.orginalColor;
    color.hoverColor.setAlphaF(0.3);

    color.hightColor = color.orginalColor;
    color.hightColor.setAlphaF(0.35);

    return color;
}

QColor CScheduleDataManage::getSystemActiveColor()
{
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
    qCDebug(scheduleDataLog) << "System active color:" << color;
    return color;
}

QColor CScheduleDataManage::getTextColor()
{
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().text().color();
    qCDebug(scheduleDataLog) << "Text color:" << color;
    return color;
}

void CScheduleDataManage::setTheMe(int type)
{
    qCInfo(scheduleDataLog) << "Setting theme to:" << type;
    m_theme = type;
}

CScheduleDataManage *CScheduleDataManage::getScheduleDataManage()
{
    if (nullptr == m_vscheduleDataManage) {
        qCDebug(scheduleDataLog) << "Creating schedule data manager instance";
        m_vscheduleDataManage = new CScheduleDataManage();
    }
    return m_vscheduleDataManage;
}

CScheduleDataManage::CScheduleDataManage(QObject *parent)
    : QObject(parent)
{
    qCDebug(scheduleDataLog) << "Initializing schedule data manager";
}

CScheduleDataManage::~CScheduleDataManage()
{
}
