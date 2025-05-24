// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "todaybutton.h"
#include "constants.h"

#include <DPalette>

#include <QPainter>
#include <QMouseEvent>

// Add logging category
Q_LOGGING_CATEGORY(todayButtonLog, "calendar.widget.todaybutton")

DGUI_USE_NAMESPACE
CTodayButton::CTodayButton(QWidget *parent)
    : DPushButton(parent)
{
    qCDebug(todayButtonLog) << "Initializing CTodayButton";
}

void CTodayButton::keyPressEvent(QKeyEvent *event)
{
    //添加回车点击效果处理
    if (event->key() == Qt::Key_Return) {
        qCDebug(todayButtonLog) << "Return key pressed, emitting click";
        click();
    }
    DPushButton::keyPressEvent(event);
}
