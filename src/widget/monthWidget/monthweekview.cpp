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

#include "monthweekview.h"
#include "scheduledatamanage.h"

#include <DPalette>
#include <DApplicationHelper>
#include <DPalette>

#include <QLabel>
#include <QDebug>
#include <QDate>

DGUI_USE_NAMESPACE
/**
 * @brief CMonthWeekView::CMonthWeekView 构造函数
 * @param parent 父类
 */
CMonthWeekView::CMonthWeekView(QWidget *parent)
    : DWidget(parent)
{
    //新建水平布局
    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    //设置布局为水平布局
    setLayout(m_mainLayout);
}
/**
 * @brief CMonthWeekView::setList 设置一周的时间
 * @param weekday 周几
 */
void CMonthWeekView::setList(int weekday)
{
    QLayoutItem *child;
    while ((child = m_mainLayout->takeAt(0)) != nullptr) {
        if (child->widget() != nullptr) {
            delete child->widget();
        }
        delete child;
    }

    m_weekData.clear();
    QLocale locale;
    //设置一周的周数
    for (int i = 0; i != DDEMonthCalendar::AFewDaysofWeek; ++i) {
        //检查周数书否正确
        int d = checkDay(i + weekday);
        //新建垂直布局
        QVBoxLayout *hhLayout = new QVBoxLayout;
        hhLayout->setMargin(0);
        hhLayout->setSpacing(0);
        hhLayout->setContentsMargins(0, 0, 0, 0);
        //新建周数所在frame
        CustomFrame *label = new CustomFrame();
        //设置frame显示的内容
        label->setTextStr(locale.dayName(d ? d : DDEMonthCalendar::AFewDaysofWeek, QLocale::ShortFormat));
        label->setContentsMargins(0, 0, 0, 0);
        //字体
        QFont weekfont;
        //设置字体权重
        weekfont.setWeight(QFont::Medium);
        //设置字体大小
        weekfont.setPixelSize(DDECalendar::FontSizeSixteen);
        //设置字体
        label->setTextFont(weekfont);
        //设置圆角
        if (i ==0) {
            //周日，设置左上角是圆角
            label->setRoundState(true, false, false, false);
        } else if (i == DDEMonthCalendar::AFewDaysofWeek - 1) {
            //周六，设置右上角是圆角
            label->setRoundState(false, true, false, false);
        }

        if (d == 0) {
            //设置背景色
            QColor textbC(0, 66, 154);
            //设置周末的文字颜色
            QColor colorSeven = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
            //设置文字颜色
            label->setTextColor(colorSeven);
            //设置背景色
            label->setBColor(textbC);
            //添加到容器中
            m_weekData.append(qMakePair(label, 1));
        } else if (d == DDEMonthCalendar::AFewDaysofWeek - 1) {
            //背景色
            QColor textbC(0, 66, 154);
            //设置周六的文字颜色
            QColor colorSix = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
            //设置文字颜色
            label->setTextColor(colorSix);
            //设置背景色
            label->setBColor(textbC);
            //添加到容器中
            m_weekData.append(qMakePair(label, 1));
        } else {
            QColor textC = Qt::black;
            QColor textbC(0, 66, 154);
            //设置背景色透明度
            textbC.setAlphaF(0.05);
            //设置文字的颜色
            label->setTextColor(textC);
            //设置背景色
            label->setBColor(textbC);
            //添加到容器中
            m_weekData.append(qMakePair(label, 0));
        }

        if ((i == weekday - 1 && weekday != 0) || i == weekday || (weekday == 0 && i == 6)) {
            label->setObjectName("MonthHeaderWeekend");
        } else {
            label->setObjectName("MonthHeaderWeekday");
        }

        DHorizontalLine *splitline = new DHorizontalLine;

        if (i == 0 || i == DDEMonthCalendar::AFewDaysofWeek - 1) {
            label->setFixedSize(DDEMonthCalendar::MWeekCellWidth - 1, DDEMonthCalendar::MWeekCellHeight);
            splitline->setFixedSize(DDEMonthCalendar::MWeekCellWidth - 1, 2);
        } else {
            label->setFixedSize(DDEMonthCalendar::MWeekCellWidth - 3, DDEMonthCalendar::MWeekCellHeight);
            splitline->setFixedSize(DDEMonthCalendar::MWeekCellWidth - 3, 2);
        }
        //将显示月份的label添加到布局中
        hhLayout->addWidget(label);
        hhLayout->addWidget(splitline);
        //可以设置背景色
        splitline->setAutoFillBackground(true);
        m_vline.append(splitline);
        splitline->setVisible(false);
        m_mainLayout->addLayout(hhLayout);
    }
}
/**
 * @brief CMonthWeekView::setTheMe 设置系统主题类型
 * @param type 系统主题类型
 */
void CMonthWeekView::setTheMe(int type)
{
    //根据系统主题设置颜色
    if (type == 0 || type == 1) {
        //系统主题为浅色或跟随系统
        for (int i = 0; i < m_weekData.count(); i++) {
            if (m_weekData.at(i).second == 1) {
                //获取系统活动色
                QColor color = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
                //背景色
                QColor textbC("#75C18E");
                //透明度
                textbC.setAlphaF(0.1);
                //设置文字颜色
                m_weekData.at(i).first->setTextColor(color);
                //设置背景色
                m_weekData.at(i).first->setBColor(textbC);
            } else {
                //文字颜色
                QColor textC = "#6F6F6F";
                //背景色
                QColor textbC("#75C18E");
                //透明度
                textbC.setAlphaF(0.1);
                //设置文字颜色
                m_weekData.at(i).first->setTextColor(textC);
                //设置背景色
                m_weekData.at(i).first->setBColor(textbC);
            }
            m_vline.at(i)->setBackgroundRole(DPalette::Highlight);
        }

    } else if (type == 2) {
        //系统主题类型为深色
        for (int i = 0; i < m_weekData.count(); i++) {
            if (m_weekData.at(i).second == 1) {
                //获取系统活动色，作为文字颜色
                QColor color = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
                //背景色
                QColor textbC = "#82AEC1";
                //背景色透明度
                textbC.setAlphaF(0.10);
                //设置文字颜色
                m_weekData.at(i).first->setTextColor(color);
                //设置背景色
                m_weekData.at(i).first->setBColor(textbC);
            } else {
                //文字颜色
                QColor textC = "#C0C6D4";
                //背景色
                QColor textbC = "#82AEC1";
                //背景色透明度
                textbC.setAlphaF(0.10);
                //设置文字颜色
                m_weekData.at(i).first->setTextColor(textC);
                //设置背景色
                m_weekData.at(i).first->setBColor(textbC);
            }
            m_vline.at(i)->setBackgroundRole(DPalette::Highlight);
        }
    }
}
/**
 * @brief CMonthWeekView::updateWeek 更新数据
 */
void CMonthWeekView::updateWeek()
{
    for (int i = 0; i < m_vline.count(); ++i) {
        m_vline.at(i)->setVisible(false);
    }
    //设置日期为今天
    QDate date = QDate::currentDate();
    //设置今天是周几
    int d = date.dayOfWeek();
    QLocale locale;
    //设置日期的本地化名称
    QString str = locale.dayName(d ? d : DDEMonthCalendar::AFewDaysofWeek, QLocale::ShortFormat);

    for (int i = 0; i < m_vline.count(); ++i) {
        if (m_weekData.at(i).first->getTextStr() == str) {
            m_vline.at(i)->setVisible(true);
        }
    }
}
/**
 * @brief CMonthWeekView::checkDay 检查周数
 * @param weekday 周几
 * @return 周几
 */
int CMonthWeekView::checkDay(int weekday)
{
    // check the week, calculate the correct order in the custom.
    return weekday % DDEMonthCalendar::AFewDaysofWeek;
}
/**
 * @brief CMonthWeekView::resizeEvent 更新窗口大小
 * @param event 窗口大小调整事件
 */
void CMonthWeekView::resizeEvent(QResizeEvent *event)
{
    int tw = static_cast<int>(width() / 7.0 + 0.5);
    int th = height();
    //设置大小
    for (int i = 0; i < m_weekData.count(); i++) {
        if (m_weekData.at(i).second == 1) {
            m_weekData.at(i).first->setFixedSize(tw, th);
        } else {
            m_weekData.at(i).first->setFixedSize(tw, th);
        }
        m_vline.at(i)->setFixedWidth(tw);
    }
    DWidget::resizeEvent(event);
}
