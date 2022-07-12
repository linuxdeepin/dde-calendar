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
#ifndef MYSCHEDULEVIEW_H
#define MYSCHEDULEVIEW_H

#include "dschedule.h"
#include "dcalendarddialog.h"

#include <DTextEdit>

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>

DWIDGET_USE_NAMESPACE
/**
 * @brief The CMyScheduleView class
 * 日程概括对话框
 */
class CMyScheduleView : public DCalendarDDialog
{
    Q_OBJECT
public:
    explicit CMyScheduleView(const DSchedule::Ptr &schduleInfo, QWidget *parent = nullptr);

    DSchedule::Ptr getSchedules()
    {
        return  m_scheduleInfo;
    }

    void setSchedules(const DSchedule::Ptr &schduleInfo);
    void updateFormat();

signals:
    void signalsEditorDelete(int type = 0);
public slots:
    //按钮点击事件
    void slotBtClick(int buttonIndex, const QString &buttonName);

    void slotAutoFeed(const QFont &font = QFont());

    //帐户状态更新
    void slotAccountStateChange();
private:
    //界面初始化
    void initUI();
    void initConnection();
    //设置label文字颜色
    void setLabelTextColor(const int type);
    //设置调色板颜色
    void setPaletteTextColor(QWidget *widget, QColor textColor);
protected:
    void updateDateTimeFormat() override;

private:
    /**
     * @brief getDataByFormat
     * 获取格式化后的事件字符串
     * @param format 格式化规则
     * @return 格式化后的字符串
     */
    QString getDataByFormat(const QDate &, QString format);

private:
    QLabel *m_scheduleLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    QLabel *m_Title = nullptr;
    DSchedule::Ptr m_scheduleInfo; //日程
    QScrollArea *area = nullptr;
    QFont labelF;
    int m_defaultH = 117; //时间显示高度
    int m_timeLabelH = 0; //时间显示高度
    int m_scheduleLabelH = 0; //日程详情显示高度
};

#endif // MYSCHEDULEVIEW_H
