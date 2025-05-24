// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sidebartreewidgetitemdelegate.h"

#include <QPainter>
#include <QPainterPath>
#include <DPalette>

Q_LOGGING_CATEGORY(sidebarDelegateLog, "calendar.widget.sidebardelegate")

SideBarTreeWidgetItemDelegate::SideBarTreeWidgetItemDelegate()
{
    qCDebug(sidebarDelegateLog) << "Creating new SideBarTreeWidgetItemDelegate";
}

void SideBarTreeWidgetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        qCDebug(sidebarDelegateLog) << "Invalid index, using default painting";
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing);

    QStyleOptionViewItem opt = option;

    QRect rect = opt.rect;
    QPainterPath path, clipPath;
    switch (opt.viewItemPosition) {
    case QStyleOptionViewItem::OnlyOne: {
        qCDebug(sidebarDelegateLog) << "Painting single item at position:" << rect;
        // 左间距
        rect.setX(rect.x() + 8);
        // 右间距
        rect.setWidth(rect.width() - 8);
    } break;
    default: {
        qCDebug(sidebarDelegateLog) << "Using default painting for item position:" << opt.viewItemPosition;
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    }

    //绘制圆角
    int radius = 8;
    clipPath.addRoundedRect(rect, radius, radius);
    painter->setClipPath(clipPath);

    if (option.state & QStyle::State_MouseOver) {
        qCDebug(sidebarDelegateLog) << "Mouse over state detected, filling with highlight color";
        painter->fillRect(painter->clipBoundingRect(), option.palette.midlight());
    }
}
