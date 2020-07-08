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

#include <QLabel>
#include <QDebug>
#include <QDate>
#include <DPalette>
#include <DApplicationHelper>
#include <DPalette>
#include "scheduledatamanage.h"
DGUI_USE_NAMESPACE
CMonthWeekView::CMonthWeekView(QWidget *parent) : DWidget(parent)
{
    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    setLayout(m_mainLayout);
}

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
    for (int i = 0; i != 7; ++i) {

        int d = checkDay(i + weekday);

        QVBoxLayout *hhLayout = new QVBoxLayout;
        hhLayout->setMargin(0);
        hhLayout->setSpacing(0);
        hhLayout->setContentsMargins(0, 0, 0, 0);

        CustomFrame *label = new CustomFrame();
        label->setTextStr(locale.dayName(d ? d : 7, QLocale::ShortFormat));
        label->setContentsMargins(0, 0, 0, 0);
        QFont weekfont;
//        weekfont.setFamily("SourceHanSansSC");
        weekfont.setWeight(QFont::Medium);
        weekfont.setPixelSize(16);
        label->setTextFont(weekfont);
        if (i ==0) {
            label->setRoundState(true, false, false, false);
        } else if (i==6) {
            label->setRoundState(false, true, false, false);
        }
        if (d == 0) {
            QColor textbC(0, 66, 154);
            QColor colorSeven = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
//            label->setTextColor("#0887FF");
            label->setTextColor(colorSeven);
            label->setBColor(textbC);
            m_weekData.append(qMakePair(label, 1));
#if 0
            DPalette monthpa = label->palette();
            QColor textC = "#0887FF";
            QColor textbC(0, 66, 154);
            textbC.setAlphaF(0.05);
            monthpa.setColor(DPalette::WindowText, textC);
            monthpa.setColor(DPalette::Background, textbC);
            label->setAutoFillBackground(true);
            label->setPalette(monthpa);
            m_weekData.append(qMakePair(label, 1));
            //label->setStyleSheet("color:#0887FF;background: rgba(0,66,154,0.05);");
#endif
        } else if (d == 6) {
            QColor textbC(0, 66, 154);
            QColor colorSix = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
//            label->setTextColor("#0887FF");
            label->setTextColor(colorSix);
            label->setBColor(textbC);
            m_weekData.append(qMakePair(label, 1));
        } else {
            QColor textC = Qt::black;
            QColor textbC(0, 66, 154);
            textbC.setAlphaF(0.05);
            label->setTextColor(textC);
            label->setBColor(textbC);
            m_weekData.append(qMakePair(label, 0));
            //label->setStyleSheet("color:black;background: rgba(0,66,154,0.05);");
        }
        if ((i == weekday - 1 && weekday != 0) || i == weekday || (weekday == 0 && i == 6)) {
            label->setObjectName("MonthHeaderWeekend");
        } else {
            label->setObjectName("MonthHeaderWeekday");
        }

        // label->setAlignment(Qt::AlignCenter);
        DHorizontalLine *splitline = new DHorizontalLine;
        if (i == 0 || i == 6) {
            label->setFixedSize(DDEMonthCalendar::MWeekCellWidth - 1, DDEMonthCalendar::MWeekCellHeight);
            splitline->setFixedSize(DDEMonthCalendar::MWeekCellWidth - 1, 2);
        } else {
            label->setFixedSize(DDEMonthCalendar::MWeekCellWidth - 3, DDEMonthCalendar::MWeekCellHeight);
            splitline->setFixedSize(DDEMonthCalendar::MWeekCellWidth - 3, 2);
        }
        hhLayout->addWidget(label);
        hhLayout->addWidget(splitline);
        splitline->setAutoFillBackground(true);
        m_vline.append(splitline);
        splitline->setVisible(false);
        m_mainLayout->addLayout(hhLayout);
    }
}

void CMonthWeekView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        for (int i = 0; i < m_weekData.count(); i++) {
            if (m_weekData.at(i).second == 1) {
//                QColor textC = "#0887FF";
                QColor color = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
                QColor textbC("#75C18E");
                textbC.setAlphaF(0.1);
//                m_weekData.at(i).first->setTextColor(textC);
                m_weekData.at(i).first->setTextColor(color);
                m_weekData.at(i).first->setBColor(textbC);
            } else {
                QColor textC = "#6F6F6F";
                QColor textbC("#75C18E");
                textbC.setAlphaF(0.1);
                m_weekData.at(i).first->setTextColor(textC);
                m_weekData.at(i).first->setBColor(textbC);
            }
            m_vline.at(i)->setBackgroundRole(DPalette::Highlight);
        }

    } else if (type == 2) {
        for (int i = 0; i < m_weekData.count(); i++) {
            if (m_weekData.at(i).second == 1) {
//                QColor textC = "#0887FF";
                QColor color = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
                QColor textbC = "#82AEC1";
                textbC.setAlphaF(0.10);
//                m_weekData.at(i).first->setTextColor(textC);
                m_weekData.at(i).first->setTextColor(color);
                m_weekData.at(i).first->setBColor(textbC);
            } else {
                QColor textC = "#C0C6D4";
                QColor textbC = "#82AEC1";
                textbC.setAlphaF(0.10);
                m_weekData.at(i).first->setTextColor(textC);
                m_weekData.at(i).first->setBColor(textbC);
            }
            //DPalette monthpa = m_vline.at(i)->palette();
            //monthpa.setColor(DPalette::Background, "#0059D2");
            //m_vline.at(i)->setPalette(monthpa);
            m_vline.at(i)->setBackgroundRole(DPalette::Highlight);
        }
    }
}

void CMonthWeekView::updateWeek()
{
    for (int i = 0; i < m_vline.count(); ++i) {
        m_vline.at(i)->setVisible(false);
    }
    QDate date = QDate::currentDate();
    int d = date.dayOfWeek();
    QLocale locale;
    QString str = locale.dayName(d ? d : 7, QLocale::ShortFormat);
    for (int i = 0; i < m_vline.count(); ++i) {

        if (m_weekData.at(i).first->getTextStr() == str) {
            m_vline.at(i)->setVisible(true);
        }
    }
}

int CMonthWeekView::checkDay(int weekday)
{

    // check the week, calculate the correct order in the custom.

//    if (weekday <= 0)
//        return weekday += 7;

//    if (weekday > 7)
//        return weekday -= 7;

    return weekday % 7;
}

void CMonthWeekView::resizeEvent(QResizeEvent *event)
{
    int tw = static_cast<int>(width() / 7.0 + 0.5);
    int th = height();
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
