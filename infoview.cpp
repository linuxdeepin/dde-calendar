/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

#include "infoview.h"
#include "spinner.h"

#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

InfoView::InfoView(QFrame *parent) :
    QFrame(parent),
    m_timeLabel(new QLabel),
    m_festivalLabel(new QLabel),
    m_todayButton(new DLinkButton),
    m_yearSpinner(new Spinner),
    m_monthSpinner(new Spinner)
{
    QFont font = m_timeLabel->font();
    font.setWeight(QFont::Light);
    font.setPixelSize(30);
    m_timeLabel->setFont(font);

    m_festivalLabel->setStyleSheet("font-size: 14px; color: #303030");

    m_todayButton->setText(tr("Today"));
    m_todayButton->setStyleSheet("Dtk--Widget--DLinkButton {"
                                 "background-color:transparent;"
                                 "border:none;"
                                 "color:#0082fa;"
                                 "}"

                                 "Dtk--Widget--DLinkButton:hover {"
                                 "color:#16b8ff;"
                                 "}"

                                 "Dtk--Widget--DLinkButton:pressed {"
                                 "color:#0060b9;"
                                 "}");

    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    QHBoxLayout * spinnerLayout = new QHBoxLayout;
    spinnerLayout->setMargin(0);
    spinnerLayout->setSpacing(0);
    spinnerLayout->addWidget(m_yearSpinner);
    spinnerLayout->addWidget(m_monthSpinner);

    QVBoxLayout * leftLayout = new QVBoxLayout;
    leftLayout->setMargin(0);
    leftLayout->setSpacing(0);
    leftLayout->addStretch();
    leftLayout->addWidget(m_timeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
    leftLayout->addWidget(m_festivalLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
    leftLayout->addSpacing(6);
    QVBoxLayout * rightLayout = new QVBoxLayout;
    rightLayout->setMargin(0);
    rightLayout->setSpacing(0);
    rightLayout->addStretch();
    rightLayout->addWidget(m_todayButton, 0, Qt::AlignVCenter | Qt::AlignRight);
    rightLayout->addSpacing(14);
    rightLayout->addLayout(spinnerLayout);
    rightLayout->addSpacing(10);

    mainLayout->addLayout(leftLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(rightLayout);

    connect(m_yearSpinner, &Spinner::valueChanged, [this](int value){
        emit yearChanged(value);
    });
    connect(m_monthSpinner, &Spinner::valueChanged, [this](int value){
        if (1 <= value && value <= 12) {
            emit monthChanged(value);
        } else {
            int month = value;
            int year = m_yearSpinner->value();
            if (value < 1) {
                year--;
                month = 12;
            } else if (value > 12) {
                year++;
                month = 1;
            }

            m_yearSpinner->blockSignals(true);
            m_yearSpinner->setValue(year);
            m_yearSpinner->blockSignals(false);

            m_monthSpinner->blockSignals(true);
            m_monthSpinner->setValue(month);
            m_monthSpinner->blockSignals(false);

            emit yearChanged(year);
            emit monthChanged(month);
        }
    });
    connect(m_todayButton, &DLinkButton::clicked, this, &InfoView::todayButtonClicked);
}

void InfoView::setTime(const QString &time) const
{
    m_timeLabel->setText(time);
}

void InfoView::setFestival(const QString &festival) const
{
    if (QLocale::system().name().contains("zh")) {
        // NOTE(hualet): the extra space before festival is a trick here,
        // it ensures that the festival label is always inflated.
        m_festivalLabel->setText(" " + festival);
    } else {
        m_festivalLabel->setText(" ");
    }
}

int InfoView::year() const
{
    return m_yearSpinner->value();
}

int InfoView::month() const
{
    return m_monthSpinner->value();
}

void InfoView::setYear(int year) const
{
    m_yearSpinner->setValue(year);
}

void InfoView::setMonth(int month) const
{
    m_monthSpinner->setValue(month);
}

void InfoView::setYearRange(int min, int max) const
{
    m_yearSpinner->setRange(min, max);
}

void InfoView::increaseMonth(bool increase)
{
    int year = m_yearSpinner->value();
    int month = m_monthSpinner->value();

    if (increase) {
        if (month < 12) {
            month += 1;
        } else {
            month = 1;
            year += 1;
        }
    } else {
        if (month == 1) {
            month = 12;
            year -= 1;
        } else {
            month -= 1;
        }
    }

    if (year != m_yearSpinner->value()) {
        m_yearSpinner->setValue(year);
    }

    // fix by jscl: must be run
    m_monthSpinner->setValue(month);
}

void InfoView::setTodayButtonVisible(bool visible) const
{
    m_todayButton->setVisible(visible);
}
