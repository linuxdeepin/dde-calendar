/*
   * Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
   *
   * Author:     chenhaifeng <chenhaifeng@uniontech.com>
   *
   * Maintainer: chenhaifeng <chenhaifeng@uniontech.com>
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
#include "cbuttonbox.h"

#include <QFocusEvent>

CButtonBox::CButtonBox(QWidget *parent)
    : DButtonBox(parent)
{
    //设置接受tab焦点切换
    this->setFocusPolicy(Qt::TabFocus);
}

void CButtonBox::focusInEvent(QFocusEvent *event)
{
    DButtonBox::focusInEvent(event);
    //窗口激活时，不设置Button焦点显示
    if (event->reason() != Qt::ActiveWindowFocusReason) {
        //设置当前选中项为焦点
        this->button(checkedId())->setFocus();
    }
}

void CButtonBox::focusOutEvent(QFocusEvent *event)
{
    DButtonBox::focusOutEvent(event);
    //当tab离开当前buttonbox窗口时，设置选中项为焦点
    if (event->reason() == Qt::TabFocusReason) {
        //设置当前选中项为焦点
        this->button(checkedId())->setFocus();
    }
}
