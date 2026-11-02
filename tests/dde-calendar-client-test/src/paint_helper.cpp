// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>

void invokeDelegatePaint(QStyledItemDelegate *delegate, QPainter *painter,
                         const QStyleOptionViewItem &option, const QModelIndex &index)
{
    delegate->paint(painter, option, index);
}
