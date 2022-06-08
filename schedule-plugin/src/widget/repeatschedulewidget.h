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
#ifndef REPEATSCHEDULEWIDGET_H
#define REPEATSCHEDULEWIDGET_H

#include "icondframe.h"
#include "dschedule.h"

class scheduleitemwidget;
class repeatScheduleWidget : public IconDFrame
{
    Q_OBJECT
public:
    enum Operation_type { Operation_Cancel,
                          Operation_Change
    };
    enum Widget_type { Widget_Repeat,
                       Widget_Confirm
    };

public:
    explicit repeatScheduleWidget(Operation_type operation, Widget_type widgetype, bool tocreateBtn = true, QWidget *parent = nullptr);
    void setSchedule(const DSchedule::Ptr &info);

private:
    void initUI();
signals:
    void signalButtonCheckNum(int index, const QString &text, const int buttonCount);
public slots:
    void slotButtonCheckNum(int index, const QString &text);

private:
    scheduleitemwidget *m_scheduleitemwidget {nullptr};
    DSchedule::List m_scheduleInfo;
    Operation_type m_OperationType {Operation_Cancel};
    Widget_type m_WidgetType {Widget_Confirm};
    int m_buttonCount {0};
    bool m_createBtnBool {true};
};

#endif // REPEATSCHEDULEWIDGET_H
