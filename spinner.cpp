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

#include "spinner.h"

#include <QLabel>
#include <QHBoxLayout>

Spinner::Spinner(QWidget *parent) :
    QWidget(parent),
    m_prevButton(new DImageButton),
    m_nextButton(new DImageButton),
    m_label(new QLabel)
{
    setFixedHeight(23);

    setPrevButtonDisabled(false);
    setNextButtonDisabled(false);

    m_label->setFixedWidth(40);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setText(QString::number(m_value));

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_prevButton);
    layout->addWidget(m_label);
    layout->addWidget(m_nextButton);
    setLayout(layout);

    connect(m_prevButton, &DImageButton::clicked, [this]{
        setValue(m_value - 1);
    });
    connect(m_nextButton, &DImageButton::clicked, [this]{
        setValue(m_value + 1);
    });
}

void Spinner::setRange(int min, int max)
{
    m_min = min;
    m_max = max;
}

int Spinner::value() const
{
    return m_value;
}

void Spinner::setValue(int value)
{
    if (value != m_value) {
        m_value = value;
        m_label->setText(QString::number(m_value));

        setPrevButtonDisabled(value <= m_min);
        setNextButtonDisabled(value >= m_max);

        emit valueChanged(value);
    }
}

void Spinner::setPrevButtonDisabled(bool disabled) const
{
    // fix by jscl:do not load png everytime
    if (m_preBtnState != disabled) {
        m_prevButton->setDisabled(disabled);
        if (disabled) {
            m_prevButton->setHoverPic(":/resources/icon/previous_disabled.png");
            m_prevButton->setNormalPic(":/resources/icon/previous_disabled.png");
            m_prevButton->setPressPic(":/resources/icon/previous_disabled.png");
        } else {
            m_prevButton->setHoverPic(":/resources/icon/previous_hover.png");
            m_prevButton->setNormalPic(":/resources/icon/previous_normal.png");
            m_prevButton->setPressPic(":/resources/icon/previous_press.png");
        }

        m_preBtnState = disabled;
    }
}

void Spinner::setNextButtonDisabled(bool disabled) const
{
    // fix by jscl:do not load png everytime
    if (m_nextBtnState != disabled) {
        m_nextButton->setDisabled(disabled);
        if (disabled) {
            m_nextButton->setHoverPic(":/resources/icon/next_disabled.png");
            m_nextButton->setNormalPic(":/resources/icon/next_disabled.png");
            m_nextButton->setPressPic(":/resources/icon/next_disabled.png");
        } else {
            m_nextButton->setHoverPic(":/resources/icon/next_hover.png");
            m_nextButton->setNormalPic(":/resources/icon/next_normal.png");
            m_nextButton->setPressPic(":/resources/icon/next_press.png");
        }

        m_nextBtnState = disabled;
    }
}
