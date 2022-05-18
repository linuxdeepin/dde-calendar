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
#include "cdialogiconbutton.h"
#include "cschedulebasewidget.h"

CDialogIconButton::CDialogIconButton(QWidget *parent) : DIconButton(parent)
{
    initView();
    connect(this, &DIconButton::clicked, this, &CDialogIconButton::slotIconClicked);
}

void CDialogIconButton::initView()
{
    setIcon(DStyle::SP_ArrowDown);
    m_dialog = new TimeJumpDialog(DArrowRectangle::ArrowTop, this);
    setFlat(true);
}

void CDialogIconButton::slotIconClicked()
{
    //获取全局时间并显示弹窗
    m_dialog->showPopup(CScheduleBaseWidget::getSelectDate(), mapToGlobal(QPoint(width()/2, height())));
}
