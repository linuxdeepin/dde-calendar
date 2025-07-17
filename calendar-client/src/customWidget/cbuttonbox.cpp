// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cbuttonbox.h"
#include "commondef.h"

#include <QFocusEvent>

CButtonBox::CButtonBox(QWidget *parent)
    : DButtonBox(parent)
{
    // qCDebug(ClientLogger) << "CButtonBox constructor initialized";
    //设置接受tab焦点切换
    this->setFocusPolicy(Qt::TabFocus);
}

void CButtonBox::focusInEvent(QFocusEvent *event)
{
    // qCDebug(ClientLogger) << "CButtonBox::focusInEvent - Focus reason:" << event->reason();
    DButtonBox::focusInEvent(event);
    //窗口激活时，不设置Button焦点显示
    if (event->reason() != Qt::ActiveWindowFocusReason) {
        // qCDebug(ClientLogger) << "Setting focus to button with checked ID:" << checkedId();
        //设置当前选中项为焦点
        this->button(checkedId())->setFocus();
    }
}

void CButtonBox::focusOutEvent(QFocusEvent *event)
{
    // qCDebug(ClientLogger) << "CButtonBox::focusOutEvent - Focus reason:" << event->reason();
    DButtonBox::focusOutEvent(event);
    //当tab离开当前buttonbox窗口时，设置选中项为焦点
    if (event->reason() == Qt::TabFocusReason) {
        // qCDebug(ClientLogger) << "TabFocusReason, setting focus to button with checked ID:" << checkedId();
        //设置当前选中项为焦点
        this->button(checkedId())->setFocus();
    }
}
