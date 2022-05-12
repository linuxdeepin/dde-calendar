/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
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
#ifndef SIDEBARCALENDARWIDGET_H
#define SIDEBARCALENDARWIDGET_H

#include <DLabel>
#include <DPushButton>
#include <QWidget>
#include <QGridLayout>
#include <QDate>

DWIDGET_USE_NAMESPACE

class SidebarCalendarKeyButton;
//小日历类
class SidebarCalendarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarCalendarWidget(QWidget *parent = nullptr);
    //设置选中的日期
    void setDate(QDate&);

signals:

public slots:
    //日期按键点击事件
    void slotKeyButtonClicked(SidebarCalendarKeyButton*);
    //下一页按钮点击事件
    void slotNextPageClicked();
    //上一页按钮点击事件
    void slotPreviousPageClicked();

private:
    void initView();
    void initConnection();
    void initData();

    //根据选中的日期设置按钮显示的日期范围 date:选中的日期
    void setKeyDate(QDate date);

private:

    QWidget* m_headWidget = nullptr;    //头部控件
    QWidget* m_weekWidget = nullptr;    //周显示区域控件
    QWidget* m_keyWidget = nullptr;     //日期按钮区域控件
    QGridLayout* m_keyLayout = nullptr; //按钮布局

    QLabel* m_dateLabel = nullptr;      //头部日期显示label
    DPushButton* m_nextPage = nullptr;  //下一页切换按键
    DPushButton* m_previousPage = nullptr;  //上一页切换按键

    QList<SidebarCalendarKeyButton*> m_keyButtonList;   //所有的日期按钮控件
};


//日历按键类
class SidebarCalendarKeyButton : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarCalendarKeyButton(QWidget *parent = nullptr);

    //设置当前按钮显示的日期
    void setDate(const QDate&);
    //获取当前按钮显示的日期
    QDate getDate();

signals:
    //点击事件信号
    void signaClicked(SidebarCalendarKeyButton*);

public slots:
    void click();

private:
    //设置被选中的日期
    static void setSelectedDate(const QDate&);
    //获取当前被选中的日期
    static QDate getSelectedDate();
    //设置显示的月份日期
    static void setDisplayedMonth(const QDate&);
    //获取当前显示的月份日期
    static QDate getDisplayedMonth();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    static QDate m_selectedData;    //被选中的日期
    static QDate m_displayedMonth;    //显示的月份日期
    QDate m_displayedDate;    //当前显示的日期
    bool pressed = false;    //是否按下

    friend SidebarCalendarWidget;
};

#endif // SIDEBARCALENDARWIDGET_H
