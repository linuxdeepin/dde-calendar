// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cdialogiconbutton.h"
#include "cschedulebasewidget.h"
#include "commondef.h"

CDialogIconButton::CDialogIconButton(QWidget *parent) : DIconButton(parent)
{
    qCDebug(ClientLogger) << "CDialogIconButton constructor initialized";
    initView();
    connect(this, &DIconButton::clicked, this, &CDialogIconButton::slotIconClicked);
}

void CDialogIconButton::initView()
{
    qCDebug(ClientLogger) << "CDialogIconButton::initView - Setting up button and dialog";
    setIcon(DStyle::SP_ArrowDown);
    m_dialog = new TimeJumpDialog(DArrowRectangle::ArrowTop, this);
    setFlat(true);
}

void CDialogIconButton::slotIconClicked()
{
    qCDebug(ClientLogger) << "CDialogIconButton::slotIconClicked - Button clicked, showing popup";
    //获取全局时间并显示弹窗
    m_dialog->showPopup(CScheduleBaseWidget::getSelectDate(), mapToGlobal(QPoint(width()/2, height())));
}
