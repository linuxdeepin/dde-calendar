// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LUNARDATASTRUCT_H
#define LUNARDATASTRUCT_H
#include <QString>
#include <QVector>
#include <QDate>

typedef struct HuangLi {
    qint64 ID = 0; //  `json:"id"` // format: ("%s%02s%02s", year, month, day)
    QString Avoid {}; // `json:"avoid"`
    QString Suit {}; //`json:"suit"`
} stHuangLi;

typedef struct _tagHolidayInfo {
    QDate date;
    char status {};// 1: 休 2: 补班
} HolidayInfo;

typedef struct _tagFestivalInfo {
    QString ID {};
    QString FestivalName {};
    QString description {};
    QString Rest {};
    int month = 0;
    int year = 0;
    QVector<HolidayInfo> listHoliday {};//除次参数外，其他参数都为空，已废弃
} FestivalInfo;

#endif // LUNARDATASTRUCT_H
