/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "scheduledatamanage.h"
#include "accountmanager.h"
#include "cscheduleoperation.h"
#include "accountmanager.h"

#include <QJsonArray>
#include <QJsonDocument>

CScheduleDataManage *CScheduleDataManage::m_vscheduleDataManage = nullptr;

//
CSchedulesColor CScheduleDataManage::getScheduleColorByType(const QString &typeId)
{
    CSchedulesColor color;
    DScheduleType::Ptr type = gAccountManager->getScheduleTypeByScheduleTypeId(typeId);
    QColor typeColor;
    if (nullptr != type) {
        typeColor = type->typeColor().colorCode();
    } else if (typeId =="other"){
            //如果类型不存在则设置一个默认颜色
            typeColor = QColor("#BA60FA");
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
    return DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
}

QColor CScheduleDataManage::getTextColor()
{
    return DGuiApplicationHelper::instance()->applicationPalette().text().color();
}

void CScheduleDataManage::setTheMe(int type)
{
    m_theme = type;
}

CScheduleDataManage *CScheduleDataManage::getScheduleDataManage()
{
    if (nullptr == m_vscheduleDataManage) {
        m_vscheduleDataManage = new CScheduleDataManage();
    }
    return m_vscheduleDataManage;
}

CScheduleDataManage::CScheduleDataManage(QObject *parent)
    : QObject(parent)
{
}

CScheduleDataManage::~CScheduleDataManage()
{
}
