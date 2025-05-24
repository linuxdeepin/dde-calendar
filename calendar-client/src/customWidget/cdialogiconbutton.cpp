// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cdialogiconbutton.h"
#include "cschedulebasewidget.h"
#include <DLog>

Q_LOGGING_CATEGORY(dialogIconLog, "calendar.widget.dialogicon")

CDialogIconButton::CDialogIconButton(QWidget *parent) : DIconButton(parent)
{
    qCDebug(dialogIconLog) << "Initializing CDialogIconButton";
    initView();
    connect(this, &DIconButton::clicked, this, &CDialogIconButton::slotIconClicked);
}

void CDialogIconButton::initView()
{
    qCDebug(dialogIconLog) << "Setting up dialog icon button view";
    setIcon(DStyle::SP_ArrowDown);
    m_dialog = new TimeJumpDialog(DArrowRectangle::ArrowTop, this);
    setFlat(true);
}

void CDialogIconButton::slotIconClicked()
{
    qCDebug(dialogIconLog) << "Dialog icon clicked, showing time jump dialog";
    //获取全局时间并显示弹窗
    m_dialog->showPopup(CScheduleBaseWidget::getSelectDate(), mapToGlobal(QPoint(width()/2, height())));
}
