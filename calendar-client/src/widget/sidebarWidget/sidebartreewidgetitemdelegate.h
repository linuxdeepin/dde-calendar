#ifndef SIDEBARTREEWIDGETITEMDELEGATE_H
#define SIDEBARTREEWIDGETITEMDELEGATE_H

#include <QStyledItemDelegate>

class SideBarTreeWidgetItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SideBarTreeWidgetItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif // SIDEBARTREEWIDGETITEMDELEGATE_H
