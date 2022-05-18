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
#include "timejumpdialog.h"
#include "cschedulebasewidget.h"
#include <QHBoxLayout>

TimeJumpDialog::TimeJumpDialog(ArrowDirection direction, QWidget *parent) : DArrowRectangle(direction, DArrowRectangle::FloatWindow, parent)
{
    setWindowFlags(Qt::Popup);
    initView();
    initConnect();
}

void TimeJumpDialog::initView()
{
    setRadius(16);  //设置圆角大小
    setArrowWidth(60);  //设置箭头宽度
    setArrowHeight(30); //设置箭头高度
    setRadiusArrowStyleEnable(true);    //设置箭头圆角样式
    setBackgroundColor(DBlurEffectWidget::AutoColor);   //设置背景样式
    setMargin(10);      //设置边距

    m_yearEdit = new CTimeLineEdit(EditYear, this);
    m_monthEdit = new CTimeLineEdit(EditMonth, this);
    m_dayEdit = new CTimeLineEdit(EditDay, this);
    m_jumpButton = new DSuggestButton(tr("Go"), this);

    m_yearEdit->setFixedSize(70, 36);
    m_monthEdit->setFixedSize(70, 36);
    m_dayEdit->setFixedSize(70, 36);
    m_jumpButton->setFixedSize(82, 36);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setMargin(0);
    hLayout->setSpacing(10);

    hLayout->addWidget(m_yearEdit);
    hLayout->addWidget(m_monthEdit);
    hLayout->addWidget(m_dayEdit);
    hLayout->addWidget(m_jumpButton);

    QWidget *wgt = new QWidget();
    wgt->setLayout(hLayout);
    setContent(wgt);
}

void TimeJumpDialog::initConnect()
{
    connect(m_yearEdit, &CTimeLineEdit::signalNumChange, this, &TimeJumpDialog::slotEditNumChange);
    connect(m_monthEdit, &CTimeLineEdit::signalNumChange, this, &TimeJumpDialog::slotEditNumChange);
    connect(m_dayEdit, &CTimeLineEdit::signalNumChange, this, &TimeJumpDialog::slotEditNumChange);
    connect(m_yearEdit, &CTimeLineEdit::signalDateJump, this, &TimeJumpDialog::slotDateJump);
    connect(m_monthEdit, &CTimeLineEdit::signalDateJump, this, &TimeJumpDialog::slotDateJump);
    connect(m_dayEdit, &CTimeLineEdit::signalDateJump, this, &TimeJumpDialog::slotDateJump);
    connect(m_jumpButton, &QPushButton::clicked, this, &TimeJumpDialog::slotJumpButtonClicked);
}

void TimeJumpDialog::initData()
{
    m_yearEdit->setNumberRange(1900, 2100);
    m_monthEdit->setNumberRange(1, 12);
    //设置天编辑器限制范围
    updateDayEditRange();
    m_yearEdit->setNum(m_date.year());
    m_monthEdit->setNum(m_date.month());
    m_dayEdit->setNum(m_date.day());
}

/**
 * @brief TimeJumpDialog::showPopup
 * 显示函数
 * @param date 初始显示的日期
 * @param pos 显示位置
 */
void TimeJumpDialog::showPopup(const QDate& date, const QPoint& pos)
{
    showPopup(date, pos.x(), pos.y());
}

/**
 * @brief TimeJumpDialog::showPopup
 * 显示函数
 * @param date 初始显示事件
 * @param x 显示位置x坐标
 * @param y 显示位置y坐标
 */
void TimeJumpDialog::showPopup(const QDate& date, int x, int y)
{
    //保存时间
    m_date = date;
    //初始化数据
    initData();
    //将焦点转移到年编辑器
    m_yearEdit->setFocus();
    //显示
    show(x, y);
}

/**
 * @brief TimeJumpDialog::updateDayEditRange
 * 设置天编辑器数字范围
 */
void TimeJumpDialog::updateDayEditRange() const
{
    static int dayNum[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = dayNum[m_date.month()-1];
    if (m_date.month() == 2
            && (m_date.year() % 400 == 0 || (m_date.year() % 4 == 0 && m_date.year() % 100 != 0))) {
        maxDay = 29;
    }
    m_dayEdit->setNumberRange(1, maxDay);
}

/**
 * @brief TimeJumpDialog::slotEditNumChange
 * 编辑器数字改变事件
 * @param id    编辑器id
 * @param num   编辑器内容改变后的数字
 */
void TimeJumpDialog::slotEditNumChange(int id, int num)
{
    switch (id) {
    case EditYear:
        //更改年
        m_date.setDate(num, m_date.month(), m_date.day());
        updateDayEditRange();
        break;
    case EditMonth:
        //更改月
        m_date.setDate(m_date.year(), num, m_date.day());
        updateDayEditRange();
        break;
    case EditDay:
        //更改日
        m_date.setDate(m_date.year(), m_date.month(), num);
        break;
    }
}

/**
 * @brief TimeJumpDialog::slotDateJump
 * 编辑器事件跳转事件
 * @param id 编辑器id
 * @param num 跳转量
 */
void TimeJumpDialog::slotDateJump(int id, int num)
{
    switch (id) {
    case EditYear:
        m_date = m_date.addYears(num);
        break;
    case EditMonth:
        m_date = m_date.addMonths(num);
        break;
    case EditDay:
        m_date = m_date.addDays(num);
        break;
    }
    //初始化数据
    initData();
}

/**
 * @brief TimeJumpDialog::slotJumpButtonClicked
 * 跳转按钮点击事件
 */
void TimeJumpDialog::slotJumpButtonClicked()
{
    //将焦点转移到button控件上，使在跳转时间前正在编辑的edit控件触发finished事件
    m_jumpButton->setFocus();
    //设置全局时间
    CScheduleBaseWidget::setSelectDate(m_date);
    //隐藏控件
    hide();
}
