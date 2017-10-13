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

#ifndef SPINNER_H
#define SPINNER_H

#include <QWidget>

#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

class QLabel;

class Spinner : public QWidget
{
    Q_OBJECT
public:
    explicit Spinner(QWidget *parent = 0);

    void setRange(int min, int max);
    int value() const;

signals:
    void valueChanged(int value);

public slots:
    void setValue(int value);

private:
    DImageButton *m_prevButton;
    DImageButton *m_nextButton;
    QLabel *m_label;

    int m_value = 0;
    int m_min = INT_MIN;
    int m_max = INT_MAX;

    // add by jscl:do not load png everytime
    bool m_preBtnState = true;
    bool m_nextBtnState = true;

    void setPrevButtonDisabled(bool disabled) const;
    void setNextButtonDisabled(bool disabled) const;
};

#endif // SPINNER_H
