/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
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
#include "lunarmanager.h"

LunarManager::LunarManager(QObject *parent) : QObject(parent)
  , m_dbusRequest(new DbusHuangLiRequest)
{

}

//按月获取节假日信息
void LunarManager::getFestivalMonth(quint32 year, quint32 month, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->getFestivalMonth(year, month);
}

//按天获取黄历信息
void LunarManager::getHuangLiDay(quint32 year, quint32 month, quint32 day, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->getHuangLiDay(year, month, day);
}

//按月获取黄历信息
void LunarManager::getHuangLiMonth(quint32 year, quint32 month, bool fill, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->getHuangLiMonth(year, month, fill);
}

//获取农历信息
void LunarManager::getLunarInfoBySolar(quint32 year, quint32 month, quint32 day, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->getLunarInfoBySolar(year, month, day);
}

//获取农历月日程
void LunarManager::getLunarMonthCalendar(quint32 year, quint32 month, bool fill, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->getLunarMonthCalendar(year, month, fill);
}
