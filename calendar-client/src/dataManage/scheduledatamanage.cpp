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

CScheduleDataManage *CScheduleDataManage::m_vscheduleDataManage = new CScheduleDataManage;

//
CSchedulesColor CScheduleDataManage::getScheduleColorByType(const QString &typeId)
{
    CSchedulesColor color;
    DTypeColor colorinfo;
    DScheduleType::Ptr type = gAccounManager->getScheduleTypeByScheduleTypeId(typeId);
    QColor typeColor;
    if (nullptr != type) {
        typeColor = type->typeColor().colorCode();
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
    return m_vscheduleDataManage;
}

CScheduleDataManage::CScheduleDataManage(QObject *parent)
    : QObject(parent)
{
}

CScheduleDataManage::~CScheduleDataManage()
{
}

void JobTypeInfoManager::updateInfo()
{
//    CScheduleOperation so;
//    so.getJobTypeList(this->m_lstJobType);
//    so.getColorTypeList(this->m_lstJobTypeColor);

    for (auto k = noticeObjBill.begin(); k != noticeObjBill.end(); k++) {
        QMetaObject::invokeMethod(k.key(), k.value());
    }
    return;
}
JobTypeInfoManager::JobTypeInfoManager(QObject *parent)
    : QObject(parent) //私有静态构造函数
{
    updateInfo();
}
JobTypeInfoManager *JobTypeInfoManager::instance()
{
    static JobTypeInfoManager instance;             //局部静态变量
    return &instance;
}
/**
 * @brief isSysJobTypeColor   是否是默认颜色
 * @param colorTypeNo         颜色类型编号
 * @return                    是否是默认颜色
 */
bool JobTypeInfoManager::isSysJobTypeColor(int colorTypeNo)
{
    for (auto typecolor : m_lstJobTypeColor) {
        if ((typecolor->colorID() == colorTypeNo) && (typecolor->privilege() == 1)) { //设定1为展示权限
            return true;
        }
    }
    return  false;
}
/**
 * @brief getSysJobTypeColor  获取指定编号的默认颜色
 * @param colorTypeNo         颜色类型编号
 * @param jobTypeColorInfo    颜色信息
 * @return                    操作结果
 */
bool JobTypeInfoManager::getSysJobTypeColor(int colorTypeNo, DTypeColor &jobTypeColorInfo)
{
    for (auto typecolor : m_lstJobTypeColor) {
        if ((typecolor->colorID() == colorTypeNo) && (typecolor->privilege() == 1)) { //设定1为展示权限
            jobTypeColorInfo = *typecolor;
            return true;
        }
    }
    return  false;
}

//查询日程类型
bool JobTypeInfoManager::getJobTypeByNo(QString iNo, DScheduleType::Ptr jobType)
{
    for (auto jb : m_lstJobType) {
        if (jb->typeID() == iNo) {
            jobType = jb;
            return true;
        }
    }
    return false;
}
//查询日程类型颜色
bool JobTypeInfoManager::getJobTypeColorByNo(int iNo, DTypeColor &jobType)
{
    for (auto &color : m_lstJobTypeColor) {
        if (color->colorID() == iNo) {
            jobType = *color;
            return true;
        }
    }
    return false;
}

//查询日程类型名称是否重复
bool JobTypeInfoManager::isJobTypeNameUsed(QString strName)
{
    for (auto jb : m_lstJobType) {
        if (jb->displayName() == strName) {
            return true;
        }
    }
    return false;
}

bool JobTypeInfoManager::isJobTypeNameUsed(const DScheduleType &info)
{
    for (auto jb : m_lstJobType) {
        //
        if (jb->displayName() == info.displayName() && jb->typeID() != info.typeID()) {
            return true;
        }
    }
    return false;
}
//获取自定义日程类型下一个编号
//TODO:代办
//int JobTypeInfoManager::getNextTypeNo()
//{
//    int typeNo = 1;

//    for (DScheduleType &job : m_lstJobType) {
//        if (job.getJobTypeNo() >= typeNo) {
//            typeNo = job.getJobTypeNo() + 1;
//        }
//    }
//    if (4 == typeNo) { //节日类型占用
//        typeNo += 1;
//    }
//    return typeNo;
//}
//获取自定义日程类型颜色下一个编号
int JobTypeInfoManager::getNextColorTypeNo()
{
    int colorTypeNo = 1;
    for (auto &jobColor : m_lstJobTypeColor) {
        if (jobColor->colorID() >= colorTypeNo) {
            colorTypeNo = jobColor->colorID() + 1;
        }
    }
    return colorTypeNo;
}

void JobTypeInfoManager::addToNoticeBill(QObject *obj, const QByteArray &method)
{
    noticeObjBill.insert(obj, method);
}

void JobTypeInfoManager::removeFromNoticeBill(QObject *obj)
{
    noticeObjBill.remove(obj);
}
