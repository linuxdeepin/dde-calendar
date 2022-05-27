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
#ifndef SCHEDULEDATERANGEINFO_H
#define SCHEDULEDATERANGEINFO_H
#include "dschedule.h"

typedef struct _tagMScheduleDateRangeInfo {
    QDate bdate;
    QDate edate;
    bool state;
    int  num;
    DSchedule::Ptr tData;

    bool operator <(const _tagMScheduleDateRangeInfo &info)const
    {
        if (bdate == info.bdate) {
            if (bdate.daysTo(edate) == info.bdate.daysTo(info.edate)) {
                return tData < info.tData;
            } else {
                return bdate.daysTo(edate) > info.bdate.daysTo(info.edate);
            }
        } else {
            return bdate < info.bdate;
        }
    }

    bool operator ==(const _tagMScheduleDateRangeInfo &info)const
    {
        return bdate == info.bdate &&
               edate == info.edate &&
               tData == info.tData &&
               state == info.state &&
               num == info.num;
    }
} MScheduleDateRangeInfo;
#endif // SCHEDULEDATERANGEINFO_H
