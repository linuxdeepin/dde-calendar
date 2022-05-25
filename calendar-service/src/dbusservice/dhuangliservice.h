/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
#ifndef HUANGLISERVICE_H
#define HUANGLISERVICE_H

#include "dservicebase.h"
#include "huangliData/dbusdatastruct.h"
#include "calendarhuangli.h"

/**
 * @brief The DHuangliService class      黄历服务
 */
class DHuangliService : public DServiceBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.HuangLi")
public:
    explicit DHuangliService(QObject *parent = nullptr);
public slots:
    Q_SCRIPTABLE QString getFestivalMonth(quint32 year, quint32 month);
    Q_SCRIPTABLE QString getHuangLiDay(quint32 year, quint32 month, quint32 day);
    Q_SCRIPTABLE QString getHuangLiMonth(quint32 year, quint32 month, bool fill);
    Q_SCRIPTABLE CaLunarDayInfo getLunarInfoBySolar(quint32 year, quint32 month, quint32 day);
    Q_SCRIPTABLE CaLunarMonthInfo getLunarMonthCalendar(quint32 year, quint32 month, bool fill);

private:
    CalendarHuangLi *m_huangli;
};

#endif // HUANGLISERVICE_H
