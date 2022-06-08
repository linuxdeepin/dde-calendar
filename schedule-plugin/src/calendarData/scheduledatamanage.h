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
#ifndef SCHEDULEDATAMANAGE_H
#define SCHEDULEDATAMANAGE_H
#include "dschedule.h"
#include "dscheduletype.h"

#include <DGuiApplicationHelper>

#include <QThread>
#include <QDate>
#include <QMutex>

DGUI_USE_NAMESPACE
struct CSchedulesColor {
    QColor normalColor; //正常状态颜色
    QColor hoverColor; //鼠标悬浮颜色
    QColor pressColor; //鼠标点击颜色
    QColor hightColor; //高亮色
    QColor orginalColor; //最初的颜色
};

class CScheduleDataManage : public QObject
{
    Q_OBJECT
public:
    static CScheduleDataManage *getScheduleDataManage();
    //根据日程类型ID获取颜色信息
    CSchedulesColor getScheduleColorByType(const QString &type);
    static QColor getSystemActiveColor();
    static QColor getTextColor();
    void setTheMe(int type = 0);
    int getTheme() const
    {
        return m_theme;
    }

private:
    CScheduleDataManage(QObject *parent = nullptr);
    ~CScheduleDataManage();

private:
    int m_theme = 0;
    static CScheduleDataManage *m_vscheduleDataManage;
};
#endif // SCHEDULEVIEW_H
