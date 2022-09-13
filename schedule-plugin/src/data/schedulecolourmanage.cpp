// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulecolourmanage.h"

ScheduleColourManage::ScheduleColourManage()
    : m_LifeScheduleColour(new CSchedulesColor())
    , m_WorkScheduleColour(new CSchedulesColor())
    , m_OtherScheduleColour(new CSchedulesColor())
    , m_SolarScheduleColour(new CSchedulesColor())
{
}

ScheduleColourManage::~ScheduleColourManage()
{
    delete m_LifeScheduleColour;
    m_LifeScheduleColour = nullptr;
    delete m_WorkScheduleColour;
    m_WorkScheduleColour = nullptr;
    delete m_OtherScheduleColour;
    m_OtherScheduleColour = nullptr;
    delete m_SolarScheduleColour;
    m_SolarScheduleColour = nullptr;
}

void ScheduleColourManage::setTheMe(int type)
{
    setLifeScheduleTheme(type);
    setOtherScheduleTheme(type);
    setWorkScheduleTheme(type);
    setSolarScheduleTheme(type);
}

CSchedulesColor ScheduleColourManage::getColorByTypeId(const int &id)
{
    //1工作 2 生活 3其他
    CSchedulesColor color;
    switch (id) {
    case 1:
        color = WorkScheduleColour();
        break;
    case 2:
        color = LifeScheduleColour();
        break;
    case 3:
        color = OtherScheduleColour();
        break;
    case 4:
        color = SolarScheduleColour();
        break;
    default:
        color = OtherScheduleColour();
        break;
    }
    return color;
}

CSchedulesColor ScheduleColourManage::LifeScheduleColour() const
{
    return *m_LifeScheduleColour;
}

CSchedulesColor ScheduleColourManage::WorkScheduleColour() const
{
    return *m_WorkScheduleColour;
}

CSchedulesColor ScheduleColourManage::OtherScheduleColour() const
{
    return *m_OtherScheduleColour;
}

CSchedulesColor ScheduleColourManage::SolarScheduleColour() const
{
    return *m_SolarScheduleColour;
}

void ScheduleColourManage::setLifeScheduleTheme(int type)
{
    if (type == 0 || type == 1) {
        m_LifeScheduleColour->type = 2;
        m_LifeScheduleColour->dotColor = "#82D245";
        m_LifeScheduleColour->dotColor.setAlphaF(1);
        m_LifeScheduleColour->gradientFromC = "#B7E6FB";
        m_LifeScheduleColour->gradientFromC.setAlphaF(0.7);
        m_LifeScheduleColour->gradientToC = "#D4FFB3";
        m_LifeScheduleColour->gradientToC.setAlphaF(0.7);
        m_LifeScheduleColour->Purecolor = "#6FFF00";
        m_LifeScheduleColour->Purecolor.setAlphaF(0.2);
        m_LifeScheduleColour->shadowcolor = "#82D245";
        m_LifeScheduleColour->textColor = "#000000";
        m_LifeScheduleColour->timeColor = "#53A316";
        m_LifeScheduleColour->hightlightgradientFromC = "#9AE5FF";
        m_LifeScheduleColour->hightlightgradientToC = "#C7FF94";
        QColor lifeP("#C7FF94");
        //lifeP.setAlphaF(0.8);
        m_LifeScheduleColour->hightlightPurecolor = lifeP;

        m_LifeScheduleColour->hovergradientFromC = "#A8E9FF";
        m_LifeScheduleColour->hovergradientToC = "#D5FFAF";
        m_LifeScheduleColour->hovergradientFromC.setAlphaF(0.95);
        m_LifeScheduleColour->hovergradientToC.setAlphaF(0.95);
        QColor lifehP("#C4FF98");
        lifehP.setAlphaF(0.8);
        m_LifeScheduleColour->hoverPurecolor = lifehP;
        m_LifeScheduleColour->splitColor = "#82D245";
        m_LifeScheduleColour->pressgradientToC = m_LifeScheduleColour->gradientToC;
        m_LifeScheduleColour->pressgradientFromC = m_LifeScheduleColour->gradientFromC;
        m_LifeScheduleColour->pressPurecolor = m_LifeScheduleColour->Purecolor;
        m_LifeScheduleColour->pressgradientToC.setAlphaF(0.8);
        m_LifeScheduleColour->pressgradientFromC.setAlphaF(0.8);
        m_LifeScheduleColour->pressPurecolor.setAlphaF(0.2);

    } else {
        m_LifeScheduleColour->type = 2;
        m_LifeScheduleColour->dotColor = "#59F88D";
        m_LifeScheduleColour->dotColor.setAlphaF(0.5);
        m_LifeScheduleColour->gradientFromC = "#2D6883";
        m_LifeScheduleColour->gradientToC = "#5D7D44";
        m_LifeScheduleColour->gradientToC.setAlphaF(0.5);
        m_LifeScheduleColour->gradientFromC.setAlphaF(0.5);
        m_LifeScheduleColour->Purecolor = "#59F88D";
        m_LifeScheduleColour->Purecolor.setAlphaF(0.1);
        m_LifeScheduleColour->shadowcolor = "#25FA6B";
        m_LifeScheduleColour->shadowcolor.setAlphaF(0.5);
        m_LifeScheduleColour->textColor = "#C0C6D4";
        //        m_LifeScheduleColour->textColor.setAlphaF(1);
        m_LifeScheduleColour->timeColor = "#38A35B";
        m_LifeScheduleColour->hightlightgradientFromC = "#2D6883";
        m_LifeScheduleColour->hightlightgradientFromC.setAlphaF(0.8);
        m_LifeScheduleColour->hightlightgradientToC = "#5D7D44";
        m_LifeScheduleColour->hightlightgradientToC.setAlphaF(0.8);
        QColor lifeP("#337044");
        lifeP.setAlphaF(0.8);
        m_LifeScheduleColour->hightlightPurecolor = lifeP;
        m_LifeScheduleColour->splitColor = "#25FA6B";
        m_LifeScheduleColour->splitColor.setAlphaF(0.3);
        m_LifeScheduleColour->hovergradientFromC = "#2D6883";
        m_LifeScheduleColour->hovergradientToC = "#5D7D44";
        m_LifeScheduleColour->hovergradientFromC.setAlphaF(0.65);
        m_LifeScheduleColour->hovergradientToC.setAlphaF(0.65);
        QColor lifehP("#59F88D");
        lifehP.setAlphaF(0.2);
        m_LifeScheduleColour->hoverPurecolor = lifehP;
        m_LifeScheduleColour->pressgradientToC = m_LifeScheduleColour->gradientToC;
        m_LifeScheduleColour->pressgradientFromC = m_LifeScheduleColour->gradientFromC;
        m_LifeScheduleColour->pressPurecolor = m_LifeScheduleColour->Purecolor;
        m_LifeScheduleColour->pressgradientToC.setAlphaF(0.4);
        m_LifeScheduleColour->pressgradientFromC.setAlphaF(0.4);
        m_LifeScheduleColour->pressPurecolor.setAlphaF(0.15);
    }
}

void ScheduleColourManage::setWorkScheduleTheme(int type)
{
    if (type == 0 || type == 1) {
        m_WorkScheduleColour->type = 1;
        m_WorkScheduleColour->dotColor = "#FB2525";
        m_WorkScheduleColour->dotColor.setAlphaF(0.5);
        m_WorkScheduleColour->gradientFromC = "#FBCEB7";
        m_WorkScheduleColour->gradientFromC.setAlphaF(0.7);
        m_WorkScheduleColour->gradientToC = "#FA9D9A";
        m_WorkScheduleColour->gradientToC.setAlphaF(0.7);
        m_WorkScheduleColour->Purecolor = "#F85566";
        m_WorkScheduleColour->Purecolor.setAlphaF(0.2);
        m_WorkScheduleColour->shadowcolor = "#FB2525";
        m_WorkScheduleColour->shadowcolor.setAlphaF(0.5);
        m_WorkScheduleColour->textColor = "#000000";
        m_WorkScheduleColour->timeColor = "#B54A4B";
        m_WorkScheduleColour->hightlightgradientFromC = "#FFD0B8";
        m_WorkScheduleColour->hightlightgradientToC = "#FF908D";
        QColor workP("#F9AEB8");
        //workP.setAlphaF(0.8);
        m_WorkScheduleColour->hightlightPurecolor = workP;
        m_WorkScheduleColour->splitColor = "#FB2525";
        m_WorkScheduleColour->splitColor.setAlphaF(0.5);

        m_WorkScheduleColour->hovergradientFromC = "#FFD0B8";
        m_WorkScheduleColour->hovergradientFromC.setAlphaF(0.95);
        m_WorkScheduleColour->hovergradientToC = "#FF9D9A";
        m_WorkScheduleColour->hovergradientToC.setAlphaF(0.95);
        QColor workhP("#FFB6BD");
        workhP.setAlphaF(0.8);
        m_WorkScheduleColour->hoverPurecolor = workhP;

        m_WorkScheduleColour->pressgradientToC = m_WorkScheduleColour->gradientToC;
        m_WorkScheduleColour->pressgradientFromC = m_WorkScheduleColour->gradientFromC;
        m_WorkScheduleColour->pressPurecolor = m_WorkScheduleColour->Purecolor;
        m_WorkScheduleColour->pressgradientToC.setAlphaF(0.8);
        m_WorkScheduleColour->pressgradientFromC.setAlphaF(0.8);
        m_WorkScheduleColour->pressPurecolor.setAlphaF(0.2);

    } else {
        m_WorkScheduleColour->type = 1;
        m_WorkScheduleColour->dotColor = "#F85566";
        m_WorkScheduleColour->dotColor.setAlphaF(0.5);
        m_WorkScheduleColour->gradientFromC = "#965A26";
        m_WorkScheduleColour->gradientToC = "#8B2521";
        m_WorkScheduleColour->gradientToC.setAlphaF(0.5);
        m_WorkScheduleColour->gradientFromC.setAlphaF(0.5);
        m_WorkScheduleColour->Purecolor = "#F85566";
        m_WorkScheduleColour->Purecolor.setAlphaF(0.1);
        m_WorkScheduleColour->shadowcolor = "#FB2525";
        m_WorkScheduleColour->shadowcolor.setAlphaF(0.5);
        m_WorkScheduleColour->textColor = "#C0C6D4";
        //        m_WorkScheduleColour->textColor.setAlphaF(1);
        m_WorkScheduleColour->timeColor = "#B54A4B";
        m_WorkScheduleColour->hightlightgradientToC = "#992D2A";
        m_WorkScheduleColour->hightlightgradientToC.setAlphaF(0.8);
        m_WorkScheduleColour->hightlightgradientFromC = "#8B521F";
        m_WorkScheduleColour->hightlightgradientFromC.setAlphaF(0.8);
        QColor workP("#77373E");
        workP.setAlphaF(0.8);
        m_WorkScheduleColour->hightlightPurecolor = workP;

        m_WorkScheduleColour->hovergradientFromC = "#965A26";
        m_WorkScheduleColour->hovergradientToC = "#8B2521";
        m_WorkScheduleColour->hovergradientFromC.setAlphaF(0.65);
        m_WorkScheduleColour->hovergradientToC.setAlphaF(0.65);
        QColor workhP("#F85566");
        workhP.setAlphaF(0.2);
        m_WorkScheduleColour->hoverPurecolor = workhP;
        m_WorkScheduleColour->splitColor = "#F85566";
        m_WorkScheduleColour->splitColor.setAlphaF(0.5);
        m_WorkScheduleColour->pressgradientToC = m_WorkScheduleColour->gradientToC;
        m_WorkScheduleColour->pressgradientFromC = m_WorkScheduleColour->gradientFromC;
        m_WorkScheduleColour->pressPurecolor = m_WorkScheduleColour->Purecolor;
        m_WorkScheduleColour->pressgradientToC.setAlphaF(0.4);
        m_WorkScheduleColour->pressgradientFromC.setAlphaF(0.4);
        m_WorkScheduleColour->pressPurecolor.setAlphaF(0.15);
    }
}

void ScheduleColourManage::setOtherScheduleTheme(int type)
{
    if (type == 0 || type == 1) {
        m_OtherScheduleColour->type = 3;
        m_OtherScheduleColour->dotColor = "#BA60FA";
        m_OtherScheduleColour->dotColor.setAlphaF(1);
        m_OtherScheduleColour->gradientFromC = "#FBE9B7";
        m_OtherScheduleColour->gradientFromC.setAlphaF(0.7);
        m_OtherScheduleColour->gradientToC = "#DFB3FF";
        m_OtherScheduleColour->gradientToC.setAlphaF(0.7);
        m_OtherScheduleColour->Purecolor = "#D191FF";
        m_OtherScheduleColour->Purecolor.setAlphaF(0.2);
        m_OtherScheduleColour->shadowcolor = "#BA60FA";
        m_OtherScheduleColour->textColor = "#000000";
        m_OtherScheduleColour->timeColor = "#8548B1";
        m_OtherScheduleColour->hightlightgradientFromC = "#FFE8AC";
        m_OtherScheduleColour->hightlightgradientToC = "#FBA5FF";
        QColor otherP("#EAC4FF");
        //otherP.setAlphaF(0.8);
        m_OtherScheduleColour->hightlightPurecolor = otherP;

        m_OtherScheduleColour->hovergradientFromC = "#FFE8AC";
        m_OtherScheduleColour->hovergradientToC = "#E2A5FF";
        m_OtherScheduleColour->hovergradientFromC.setAlphaF(0.95);
        m_OtherScheduleColour->hovergradientToC.setAlphaF(0.95);
        QColor otherhP("#E6C5FF");
        otherhP.setAlphaF(0.8);
        m_OtherScheduleColour->hoverPurecolor = otherhP;
        m_OtherScheduleColour->splitColor = "#BA60FA";
        m_OtherScheduleColour->pressgradientToC = m_OtherScheduleColour->gradientToC;
        m_OtherScheduleColour->pressgradientFromC = m_OtherScheduleColour->gradientFromC;
        m_OtherScheduleColour->pressPurecolor = m_OtherScheduleColour->Purecolor;
        m_OtherScheduleColour->pressgradientToC.setAlphaF(0.8);
        m_OtherScheduleColour->pressgradientFromC.setAlphaF(0.8);
        m_OtherScheduleColour->pressPurecolor.setAlphaF(0.2);
    } else {
        m_OtherScheduleColour->type = 3;
        m_OtherScheduleColour->dotColor = "#C155F8";
        m_OtherScheduleColour->dotColor.setAlphaF(0.7);
        m_OtherScheduleColour->gradientFromC = "#8C4E2C";
        m_OtherScheduleColour->gradientToC = "#7D37AF";
        m_OtherScheduleColour->gradientToC.setAlphaF(0.5);
        m_OtherScheduleColour->gradientFromC.setAlphaF(0.5);
        m_OtherScheduleColour->Purecolor = "#C155F8";
        m_OtherScheduleColour->Purecolor.setAlphaF(0.1);
        m_OtherScheduleColour->shadowcolor = "#BE3DFF";
        m_OtherScheduleColour->shadowcolor.setAlphaF(0.5);
        m_OtherScheduleColour->textColor = "#C0C6D4";
        //        m_OtherScheduleColour->textColor.setAlphaF(1);
        m_OtherScheduleColour->timeColor = "#9857C8";
        m_OtherScheduleColour->hightlightgradientFromC = "#8C4E2C";
        m_OtherScheduleColour->hightlightgradientFromC.setAlphaF(0.8);
        m_OtherScheduleColour->hightlightgradientToC = "#803BAE";
        m_OtherScheduleColour->hightlightgradientToC.setAlphaF(0.8);
        QColor otherP("#613776");
        otherP.setAlphaF(0.8);
        m_OtherScheduleColour->hightlightPurecolor = otherP;

        m_OtherScheduleColour->hovergradientFromC = "#8C4E2C";
        m_OtherScheduleColour->hovergradientToC = "#7D37AF";
        m_OtherScheduleColour->hovergradientFromC.setAlphaF(0.65);
        m_OtherScheduleColour->hovergradientToC.setAlphaF(0.65);
        QColor otherhP("#C155F8");
        otherhP.setAlphaF(0.2);
        m_OtherScheduleColour->hoverPurecolor = otherhP;
        m_OtherScheduleColour->splitColor = "#BA32FF";
        m_OtherScheduleColour->splitColor.setAlphaF(0.5);
        m_OtherScheduleColour->pressgradientToC = m_OtherScheduleColour->gradientToC;
        m_OtherScheduleColour->pressgradientFromC = m_OtherScheduleColour->gradientFromC;
        m_OtherScheduleColour->pressPurecolor = m_OtherScheduleColour->Purecolor;
        m_OtherScheduleColour->pressgradientToC.setAlphaF(0.4);
        m_OtherScheduleColour->pressgradientFromC.setAlphaF(0.4);
        m_OtherScheduleColour->pressPurecolor.setAlphaF(0.15);
    }
}

void ScheduleColourManage::setSolarScheduleTheme(int type)
{
    if (type == 0 || type == 1) {
        m_SolarScheduleColour->type = 4;
        m_SolarScheduleColour->dotColor = "#FF7272";
        m_SolarScheduleColour->dotColor.setAlphaF(1);
        m_SolarScheduleColour->gradientFromC = "#FF7272";
        m_SolarScheduleColour->gradientFromC.setAlphaF(0.3);
        m_SolarScheduleColour->gradientToC = "#FF7272";
        m_SolarScheduleColour->gradientToC.setAlphaF(0.3);
        m_SolarScheduleColour->Purecolor = "#FF7272";
        m_SolarScheduleColour->Purecolor.setAlphaF(0.3);
        m_SolarScheduleColour->shadowcolor = "#BA60FA";
        m_SolarScheduleColour->textColor = "#000000";
        m_SolarScheduleColour->timeColor = "#8548B1";
        m_SolarScheduleColour->hightlightgradientFromC = "#F9AAAA";
        m_SolarScheduleColour->hightlightgradientFromC.setAlphaF(1);
        m_SolarScheduleColour->hightlightgradientToC = "#F9AAAA";
        m_SolarScheduleColour->hightlightgradientToC.setAlphaF(1);
        m_SolarScheduleColour->hightlightPurecolor = "#F9AAAA";
        m_SolarScheduleColour->hightlightPurecolor.setAlphaF(1);

        m_SolarScheduleColour->hovergradientFromC = "#FFB0B1";
        m_SolarScheduleColour->hovergradientToC = "#FFB0B1";
        m_SolarScheduleColour->hovergradientFromC.setAlphaF(0.95);
        m_SolarScheduleColour->hovergradientToC.setAlphaF(0.95);
        m_SolarScheduleColour->hoverPurecolor = "#FF7272";
        m_SolarScheduleColour->splitColor = "#FF7272";
        m_SolarScheduleColour->pressgradientFromC = "#FF7272";
        m_SolarScheduleColour->pressgradientFromC.setAlphaF(0.3);
        m_SolarScheduleColour->pressgradientToC = "#FF7272";
        m_SolarScheduleColour->pressgradientToC.setAlphaF(0.3);

        m_SolarScheduleColour->pressPurecolor = m_SolarScheduleColour->Purecolor;
        m_SolarScheduleColour->pressgradientToC.setAlphaF(0.4);
        m_SolarScheduleColour->pressgradientFromC.setAlphaF(0.4);
        m_SolarScheduleColour->pressPurecolor.setAlphaF(0.1);
    } else {
        m_SolarScheduleColour->type = 4;
        m_SolarScheduleColour->dotColor = "#FF7272";
        m_SolarScheduleColour->dotColor.setAlphaF(0.8);
        m_SolarScheduleColour->gradientFromC = "#FF7272";
        m_SolarScheduleColour->gradientFromC.setAlphaF(0.3);
        m_SolarScheduleColour->gradientToC = "#FF7272";
        m_SolarScheduleColour->gradientToC.setAlphaF(0.3);
        m_SolarScheduleColour->Purecolor = "#FF7272";
        m_SolarScheduleColour->Purecolor.setAlphaF(0.3);
        m_SolarScheduleColour->shadowcolor = "#BA60FA";
        m_SolarScheduleColour->textColor = "#C0C6D4";
        m_SolarScheduleColour->timeColor = "#8548B1";
        m_SolarScheduleColour->hightlightgradientFromC = "#A24545";
        m_SolarScheduleColour->hightlightgradientFromC.setAlphaF(0.8);
        m_SolarScheduleColour->hightlightgradientToC = "#A24545";
        m_SolarScheduleColour->hightlightgradientToC.setAlphaF(0.8);
        m_SolarScheduleColour->hightlightPurecolor = "#A24545";
        m_SolarScheduleColour->hightlightPurecolor.setAlphaF(0.8);

        m_SolarScheduleColour->hovergradientFromC = "#E56464";
        m_SolarScheduleColour->hovergradientToC = "#E56464";
        m_SolarScheduleColour->hovergradientFromC.setAlphaF(0.35);
        m_SolarScheduleColour->hovergradientToC.setAlphaF(0.35);
        m_SolarScheduleColour->hoverPurecolor = "#E56464";
        m_SolarScheduleColour->splitColor = "#FF7272";
        m_SolarScheduleColour->pressgradientFromC = "#FF7272";
        m_SolarScheduleColour->pressgradientFromC.setAlphaF(0.3);
        m_SolarScheduleColour->pressgradientToC = m_SolarScheduleColour->pressgradientFromC;
        //        m_SolarScheduleColour->pressgradientToC = "#000000";
        //        m_SolarScheduleColour->pressgradientToC.setAlphaF(0.05);

        m_SolarScheduleColour->pressPurecolor = m_SolarScheduleColour->Purecolor;
        m_SolarScheduleColour->pressgradientToC.setAlphaF(0.4);
        m_SolarScheduleColour->pressgradientFromC.setAlphaF(0.4);
        m_SolarScheduleColour->pressPurecolor.setAlphaF(0.1);
    }
}
