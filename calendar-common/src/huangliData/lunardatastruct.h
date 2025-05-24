// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LUNARDATASTRUCT_H
#define LUNARDATASTRUCT_H
#include <QString>
#include <QVector>
#include <QDate>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lunarDataStruct)

typedef struct HuangLi {
    qint64 ID = 0; //  `json:"id"` // format: ("%s%02s%02s", year, month, day)
    QString Avoid {}; // `json:"avoid"`
    QString Suit {}; //`json:"suit"`
} stHuangLi;

typedef struct _tagHolidayInfo {
    QDate date;
    uint status; // 1: 休 2: 补班
} HolidayInfo;

typedef struct _tagFestivalInfo {
    QVector<HolidayInfo> listHoliday {};
} FestivalInfo;

#endif // LUNARDATASTRUCT_H
