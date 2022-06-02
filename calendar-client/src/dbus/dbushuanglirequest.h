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
#ifndef DBUSHUANGLIREQUEST_H
#define DBUSHUANGLIREQUEST_H

#include "dbusrequestbase.h"
#include "huangliData/lunardatastruct.h"
#include "huangliData/dbusdatastruct.h"

//黄历数据请求类
class DbusHuangLiRequest : public DbusRequestBase
{
    Q_OBJECT
public:
    explicit DbusHuangLiRequest(QObject *parent = nullptr);

    //按月获取节假日信息
    bool getFestivalMonth(quint32 year, quint32 month, FestivalInfo&);
    //按天获取黄历信息
    bool getHuangLiDay(quint32 year, quint32 month, quint32 day, CaHuangLiDayInfo &);
    //按月获取黄历信息
    bool getHuangLiMonth(quint32 year, quint32 month, bool fill, CaHuangLiMonthInfo &);
    //获取农历信息
    void getLunarInfoBySolar(quint32 year, quint32 month, quint32 day);
    //获取农历月日程
    void getLunarMonthCalendar(quint32 year, quint32 month, bool fill);

signals:

public slots:
    void slotCallFinished(CDBusPendingCallWatcher*) override;

};

#endif // DBUSHUANGLIREQUEST_H
