// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "labelwidget.h"
#include <QPainter>
#include <QStyleOptionFocusRect>

Q_LOGGING_CATEGORY(LabelWidgetLog, "calendar.widget.labelwidget")

LabelWidget::LabelWidget(QWidget *parent)
    : QLabel(parent)
{
    qCDebug(LabelWidgetLog) << "Creating LabelWidget";
    //设置焦点选中类型
    setFocusPolicy(Qt::FocusPolicy::TabFocus);
}

LabelWidget::~LabelWidget()
{
    qCDebug(LabelWidgetLog) << "Destroying LabelWidget";
}

void LabelWidget::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    if (hasFocus()) {
        qCDebug(LabelWidgetLog) << "Painting focus rectangle";
        //有焦点，绘制焦点
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.backgroundColor = palette().color(QPalette::Window);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
    QLabel::paintEvent(ev);
}
