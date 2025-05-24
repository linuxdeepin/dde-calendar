// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cbuttonbox.h"

#include <QFocusEvent>

Q_LOGGING_CATEGORY(buttonBoxLog, "calendar.widget.buttonbox")

CButtonBox::CButtonBox(QWidget *parent)
    : DButtonBox(parent)
{
    //设置接受tab焦点切换
    this->setFocusPolicy(Qt::TabFocus);
    qCDebug(buttonBoxLog) << "CButtonBox initialized with TabFocus policy";
}

void CButtonBox::focusInEvent(QFocusEvent *event)
{
    qCDebug(buttonBoxLog) << "Focus in event with reason:" << event->reason();
    DButtonBox::focusInEvent(event);
    //窗口激活时，不设置Button焦点显示
    if (event->reason() != Qt::ActiveWindowFocusReason) {
        //设置当前选中项为焦点
        this->button(checkedId())->setFocus();
    }
}

void CButtonBox::focusOutEvent(QFocusEvent *event)
{
    qCDebug(buttonBoxLog) << "Focus out event with reason:" << event->reason();
    DButtonBox::focusOutEvent(event);
    //当tab离开当前buttonbox窗口时，设置选中项为焦点
    if (event->reason() == Qt::TabFocusReason) {
        //设置当前选中项为焦点
        this->button(checkedId())->setFocus();
    }
}
