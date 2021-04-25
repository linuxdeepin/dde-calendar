/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
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
#ifndef SchecduleRemindWidget_H
#define SchecduleRemindWidget_H

#include "schedulestructs.h"
#include "scheduledatamanage.h"

#include <DWidget>
#include <DArrowRectangle>
#include <DFrame>

DWIDGET_USE_NAMESPACE

class CenterWidget;
class SchecduleRemindWidget : public DArrowRectangle
{
    Q_OBJECT
    Q_DISABLE_COPY(SchecduleRemindWidget)
public:
    explicit SchecduleRemindWidget(QWidget *parent = nullptr);
    ~SchecduleRemindWidget() override;
    void setData(const ScheduleDtailInfo  &vScheduleInfo, const CSchedulesColor &gcolor);

signals:

public slots:
private:
    CenterWidget *m_centerWidget = nullptr;
    ScheduleDtailInfo     m_ScheduleInfo;
    CSchedulesColor gdcolor;
};

class CenterWidget : public DFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(CenterWidget)
public:
    explicit CenterWidget(DWidget *parent = nullptr);
    ~CenterWidget() override;
    void setData(const ScheduleDtailInfo  &vScheduleInfo, const CSchedulesColor &gcolor);
    void setTheMe(const int type = 0);
private:
    void UpdateTextList();
protected:
    void paintEvent(QPaintEvent *e) override;
private:
    QStringList testList;
    QFont textfont;
    int textwidth;
    int textheight;
    const int textRectWidth = 165;
    ScheduleDtailInfo     m_ScheduleInfo;
    CSchedulesColor gdcolor;
    QColor textColor;
    QColor timeColor;
};

#endif // SchecduleRemindWidget_H
