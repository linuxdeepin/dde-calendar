// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULESTURCTS_H
#define SCHEDULESTURCTS_H

#include <QList>
#include <QDateTime>
#include <QColor>
#include <QVector>

typedef struct _tagScheduleRemindInfo {
    int id = 0;
    QDateTime remindDateTime;
    int times = 1;
} ScheduleRemindInfo;
typedef struct _tagScheduleRemindData {
    int n = 0; //全天代表天数 非全天代表分钟
    QTime time; //全天该变量才有效
} ScheduleRemindData;
typedef struct _tagScheduleEndRepeatData {
    int type = 0; //0 永不 1  多少次结束  2 结束日期
    QDateTime date; //为2时才有效
    int tcount = 0; //1时有效
} ScheduleEndRepeatData;
typedef struct _tagScheduleType {
    QString typeName; //work life other
    QColor color; //颜色
    int ID = -1;
} ScheduleType;
typedef struct _tagScheduleDtailInfo {
    int id = 0;
    QDateTime beginDateTime;
    QDateTime endDateTime;
    QVector<QDateTime> ignore;
    QString titleName;
    QString description;
    bool allday = true; //1全天
    ScheduleType type; //1工作 2 生活 3其他
    int RecurID = 0; //0 代表原始  大于0 代表克隆
    bool remind = true; //0无 1 提醒
    ScheduleRemindData remindData;
    int rpeat = 0; //0 无  1 每天 2 每个工作日 3 每周 4每月 5每年
    ScheduleEndRepeatData enddata;
    explicit _tagScheduleDtailInfo()
    {
        type.ID = -1;
    }
    bool operator==(const _tagScheduleDtailInfo &info) const
    {
        if (info.type.ID == 4) {
            return this->id == info.id && this->RecurID == info.RecurID && titleName == info.titleName && beginDateTime == info.beginDateTime;
        } else {
            return this->id == info.id && this->RecurID == info.RecurID && titleName == info.titleName;
        }
    }
    bool operator<(const _tagScheduleDtailInfo &info) const
    {
        if (beginDateTime.date() != endDateTime.date() && info.beginDateTime.date() == info.endDateTime.date()) {
            return true;
        } else if (beginDateTime.date() == endDateTime.date() && info.beginDateTime.date() != info.endDateTime.date()) {
            return false;
        } else if (beginDateTime.date() != endDateTime.date() && info.beginDateTime.date() != info.endDateTime.date()) {
            if (beginDateTime.date() == info.beginDateTime.date()) {
                return beginDateTime.daysTo(endDateTime) > info.beginDateTime.daysTo(info.endDateTime);
            }
            return beginDateTime.date() < info.beginDateTime.date();
        } else {
            if (type.ID == 4)
                return true;
            if (info.type.ID == 4)
                return false;
            if (beginDateTime == info.beginDateTime) {
                if (titleName == info.titleName) {
                    return id < info.id;
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
    bool state = true;
    int num = 0;
    ScheduleDtailInfo tData;
    bool operator<(const _tagMScheduleDateRangeInfo &info) const
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
    bool operator==(const _tagMScheduleDateRangeInfo &info) const
    {
        return bdate == info.bdate && edate == info.edate && tData == info.tData && state == info.state && num == info.num;
    }
} MScheduleDateRangeInfo;
#endif
