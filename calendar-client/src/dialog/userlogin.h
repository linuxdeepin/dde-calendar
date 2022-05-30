/*
* Copyright (C) 2019 ~ 2022 Uniontech Software Technology Co.,Ltd.
*
* Author:     xuezifan  <xuezifan@uniontech.com>
*
* Maintainer: xuezifan  <xuezifan@uniontech.com>
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
#ifndef USERLOGIN_H
#define USERLOGIN_H

#include "dsettingsdialog.h"

#include <QObject>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

class Userlogin: public QObject
{
public:
    explicit Userlogin(QObject *parent = nullptr);
    virtual ~Userlogin();

    static QPair<QWidget*, QWidget*> createloginButton(QObject *obj);
protected:
    QIcon *m_switchIconLight {};
    QIcon *m_switchIconDark {};
};

#endif // USERLOGIN_H
