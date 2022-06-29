/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
#include "doainfoitem.h"

#include <QHBoxLayout>
#include <QLabel>

DOAInfoItem::DOAInfoItem(const QString &name, QWidget *widget)
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(10, 6, 10, 6);
    QLabel *nameLbl = new QLabel(name, this);
    nameLbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    nameLbl->setMinimumWidth(130);
    layout->addWidget(nameLbl);
    layout->addWidget(widget);
    layout->setStretchFactor(nameLbl, 1);
    layout->setStretchFactor(widget, 2);
    this->setLayout(layout);
    //根据UI图设置固定高度为48
    setFixedHeight(48);
}