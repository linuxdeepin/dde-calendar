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
#ifndef SCHEDULESTURCTS_H
#define SCHEDULESTURCTS_H

#include <QList>
#include <QDateTime>
#include <QColor>
#include <QVector>
typedef struct _tagScheduleInfo {
    int               id;
    QDateTime         beginDateTime;
    QDateTime         endDateTime;
    QString           titleName;
    QString           description;
    int               remind;  //0 当天 1 一天前 2 三天前 3 一周前
    int               repeat;  //0 一次 1每天    2 每周  3每月  4每年
    char              allday;  //1全天
    int               infotype;    //0工作 1 生活 2其他
} ScheduleInfo;
typedef struct _tagScheduleRemindInfo {
    int               id;
    QDateTime         remindDateTime;
    int               times;
    _tagScheduleRemindInfo()
    {
        times = 1;
    }
} ScheduleRemindInfo;
typedef struct _tagScheduleRemindData {
    int               n; //全天代表天数 非全天代表分钟
    QTime             time; //全天该变量才有效
} ScheduleRemindData;
typedef struct _tagScheduleEndRepeatData {
    int               type; //0 永不 1  多少次结束  2 结束日期
    QDateTime         date; //为2时才有效
    int               tcount;//1时有效
} ScheduleEndRepeatData;
typedef struct _tagScheduleType {
    QString           typeName; //work life other
    QColor            color; //颜色
    int               ID;
} ScheduleType;
typedef struct _tagScheduleDtailInfo {
    int               id;
    QDateTime         beginDateTime;
    QDateTime         endDateTime;
    QVector<QDateTime>  ignore;
    QString           titleName;
    QString           description;
    bool              allday;  //1全天
    ScheduleType      type;    //1工作 2 生活 3其他
    int               RecurID;  //0 代表原始  大于0 代表克隆
    bool              remind;  //0无 1 提醒
    ScheduleRemindData remindData;
    int               rpeat; //0 无  1 每天 2 每个工作日 3 每周 4每月 5每年
    ScheduleEndRepeatData enddata;

    bool                    IsMoveInfo = false;      //只在拖拽使用
    explicit _tagScheduleDtailInfo()
    {
        type.ID = -1;
    }
    bool operator ==(const _tagScheduleDtailInfo &info)const
    {
        if (info.type.ID ==4) {
            return this->id == info.id &&this->RecurID == info.RecurID &&
                   titleName == info.titleName &&
                   beginDateTime ==info.beginDateTime;
        } else {
            return this->id == info.id &&this->RecurID == info.RecurID &&
                   titleName == info.titleName;
        }
    }
    bool operator <(const _tagScheduleDtailInfo &info)const
    {
        if (beginDateTime.date() != endDateTime.date() &&
                info.beginDateTime.date() == info.endDateTime.date()) {
            return true;
        } else if (beginDateTime.date() == endDateTime.date() &&
                   info.beginDateTime.date() != info.endDateTime.date()) {
            return false;
        } else if (beginDateTime.date() != endDateTime.date() &&
                   info.beginDateTime.date() != info.endDateTime.date()) {
            if (beginDateTime.date() == info.beginDateTime.date()) {
                return beginDateTime.daysTo(endDateTime) > info.beginDateTime.daysTo(info.endDateTime);
            }
            return beginDateTime.date() < info.beginDateTime.date();
        } else {
            if (type.ID == 4) return true;
            if (info.type.ID == 4) return false;
            if (beginDateTime == info.beginDateTime) {
                if (titleName == info.titleName) {
                    return id<info.id;
                }
                return titleName < info.titleName;
            } else {
                return beginDateTime < info.beginDateTime;
            }
        }
    }
} ScheduleDtailInfo;

typedef struct _tagScheduleDateRangeInfo {
    QDate date;
    QVector<ScheduleDtailInfo> vData;
} ScheduleDateRangeInfo;
typedef struct _tagMScheduleDateRangeInfo {
    QDate bdate;
    QDate edate;
    bool state;
    int  num;
    ScheduleDtailInfo tData;
    bool operator <(const _tagMScheduleDateRangeInfo &info)const
    {
        if (bdate == info.bdate) {
            if (bdate.daysTo(edate)==info.bdate.daysTo(info.edate)) {
                return tData<info.tData;
            } else {
                return bdate.daysTo(edate)>info.bdate.daysTo(info.edate);
            }
        } else {
            return bdate<info.bdate;
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
#endif
