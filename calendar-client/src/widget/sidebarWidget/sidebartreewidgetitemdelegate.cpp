// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sidebartreewidgetitemdelegate.h"
#include "commondef.h"

#include <QPainter>
#include <QPainterPath>
#include <DPalette>

SideBarTreeWidgetItemDelegate::SideBarTreeWidgetItemDelegate()
{
    qCDebug(ClientLogger) << "SideBarTreeWidgetItemDelegate constructed";
}

void SideBarTreeWidgetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        // qCDebug(ClientLogger) << "Invalid index for tree item delegate, using default painting";
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing);

    QStyleOptionViewItem opt = option;

    QRect rect = opt.rect;
    QPainterPath path, clipPath;
    switch (opt.viewItemPosition) {
    case QStyleOptionViewItem::OnlyOne: {
        // qCDebug(ClientLogger) << "Painting tree item as OnlyOne at row:" << index.row();
        // 左间距
        rect.setX(rect.x() + 8);
        // 右间距
        rect.setWidth(rect.width() - 8);
    } break;
    default: {
        // qCDebug(ClientLogger) << "Unsupported view item position for tree item delegate, using default painting";
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    }

    //绘制圆角
    int radius = 8;
    clipPath.addRoundedRect(rect, radius, radius);
    painter->setClipPath(clipPath);

    if (option.state & QStyle::State_MouseOver) {
        // qCDebug(ClientLogger) << "Tree item has mouse over state, highlighting";
        painter->fillRect(painter->clipBoundingRect(), option.palette.midlight());
    }
}
