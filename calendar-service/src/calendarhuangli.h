// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALENDARHUANGLI_H
#define CALENDARHUANGLI_H

#include "dbmanager/huanglidatabase.h"

#include <QObject>

class HuangLiDataBase;

class CalendarHuangLi : public QObject
{
    Q_OBJECT
public:
    explicit CalendarHuangLi(QObject *parent = nullptr);

    QString GetFestivalMonth(quint32 year, quint32 month);
    QString GetHuangLiDay(quint32 year, quint32 month, quint32 day);
    QString GetHuangLiMonth(quint32 year, quint32 month, bool fill);
    CaLunarDayInfo GetLunarInfoBySolar(quint32 year, quint32 month, quint32 day);
    CaLunarMonthInfo GetLunarCalendarMonth(quint32 year, quint32 month, bool fill);

private:
signals:

public slots:

private:
    HuangLiDataBase *m_database;
};

#endif // CALENDARHUANGLI_H
