// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULECOLOURMANAGE_H
#define SCHEDULECOLOURMANAGE_H

#include <QColor>
#include <QObject>

struct CSchedulesColor {
    int type;
    QColor gradientFromC;
    QColor gradientToC;
    QColor dotColor;
    QColor Purecolor;
    QColor shadowcolor;
    QColor textColor;
    QColor timeColor;
    QColor hightlightgradientFromC;
    QColor hightlightgradientToC;
    QColor hightlightPurecolor;
    QColor hovergradientFromC;
    QColor hovergradientToC;
    QColor hoverPurecolor;
    QColor splitColor;
    QColor pressgradientFromC;
    QColor pressgradientToC;
    QColor pressPurecolor;
};

class ScheduleColourManage
{
public:
    ScheduleColourManage();
    ~ScheduleColourManage();

public:
    void setTheMe(int type = 0);
    CSchedulesColor getColorByTypeId(const int &id);
private:
    CSchedulesColor LifeScheduleColour() const;

    CSchedulesColor WorkScheduleColour() const;

    CSchedulesColor OtherScheduleColour() const;

    CSchedulesColor SolarScheduleColour() const;

private:
    void setLifeScheduleTheme(int type = 0);
    void setWorkScheduleTheme(int type = 0);
    void setOtherScheduleTheme(int type = 0);
    void setSolarScheduleTheme(int type = 0);

private:
    CSchedulesColor *m_LifeScheduleColour {nullptr};
    CSchedulesColor *m_WorkScheduleColour {nullptr};
    CSchedulesColor *m_OtherScheduleColour {nullptr};
    CSchedulesColor *m_SolarScheduleColour {nullptr};
};

#endif // SCHEDULECOLOURMANAGE_H
