// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "labelwidget.h"
#include "commondef.h"
#include <QPainter>
#include <QStyleOptionFocusRect>

LabelWidget::LabelWidget(QWidget *parent)
    : QLabel(parent)
{
    // qCDebug(ClientLogger) << "LabelWidget constructor";
    //设置焦点选中类型
    setFocusPolicy(Qt::FocusPolicy::TabFocus);
}

LabelWidget::~LabelWidget()
{
    // qCDebug(ClientLogger) << "LabelWidget destructor";
}

void LabelWidget::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    if (hasFocus()) {
        // qCDebug(ClientLogger) << "LabelWidget paintEvent: has focus, drawing focus rect";
        //有焦点，绘制焦点
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.backgroundColor = palette().color(QPalette::Window);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
    QLabel::paintEvent(ev);
}
