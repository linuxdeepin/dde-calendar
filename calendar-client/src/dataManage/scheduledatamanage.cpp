// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduledatamanage.h"
#include "accountmanager.h"
#include "cscheduleoperation.h"
#include "accountmanager.h"
#include "commondef.h"

#include <QJsonArray>
#include <QJsonDocument>

CScheduleDataManage *CScheduleDataManage::m_vscheduleDataManage = nullptr;

//
CSchedulesColor CScheduleDataManage::getScheduleColorByType(const QString &typeId)
{
    qCDebug(ClientLogger) << "Getting schedule color for type ID:" << typeId;
    CSchedulesColor color;
    DScheduleType::Ptr type = gAccountManager->getScheduleTypeByScheduleTypeId(typeId);
    QColor typeColor;
    if (nullptr != type) {
        typeColor = type->typeColor().colorCode();
        qCDebug(ClientLogger) << "Found type color:" << typeColor.name();
    } else if (typeId =="other"){
            //如果类型不存在则设置一个默认颜色
            typeColor = QColor("#BA60FA");
            qCDebug(ClientLogger) << "Using default color for 'other' type:" << typeColor.name();
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

    qCDebug(ClientLogger) << "Returning schedule colors with original color:" << color.orginalColor.name();
    return color;
}

QColor CScheduleDataManage::getSystemActiveColor()
{
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
    // qCDebug(ClientLogger) << "Getting system active color:" << color.name();
    return color;
}

QColor CScheduleDataManage::getTextColor()
{
    QColor color = DGuiApplicationHelper::instance()->applicationPalette().text().color();
    // qCDebug(ClientLogger) << "Getting text color:" << color.name();
    return color;
}

void CScheduleDataManage::setTheMe(int type)
{
    // qCDebug(ClientLogger) << "Setting theme to:" << type;
    m_theme = type;
}

CScheduleDataManage *CScheduleDataManage::getScheduleDataManage()
{
    // qCDebug(ClientLogger) << "Getting schedule data manage instance";
    if (nullptr == m_vscheduleDataManage) {
        qCDebug(ClientLogger) << "Creating new schedule data manage instance";
        m_vscheduleDataManage = new CScheduleDataManage();
    }
    return m_vscheduleDataManage;
}

CScheduleDataManage::CScheduleDataManage(QObject *parent)
    : QObject(parent)
{
    qCDebug(ClientLogger) << "CScheduleDataManage constructor";
}

CScheduleDataManage::~CScheduleDataManage()
{
    qCDebug(ClientLogger) << "CScheduleDataManage destructor";
}
