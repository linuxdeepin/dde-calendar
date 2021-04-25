/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DRAGINFOITEM_H
#define DRAGINFOITEM_H
#include "scheduledatamanage.h"

#include <DFontSizeManager>

#include <QGraphicsRectItem>

DWIDGET_USE_NAMESPACE

class QPropertyAnimation;
class QSequentialAnimationGroup;

class DragInfoItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    Q_PROPERTY(int offset READ readOffset WRITE setOffset)
public:
    explicit DragInfoItem(QRectF rect, QGraphicsItem *parent = nullptr);
    ~DragInfoItem() override;
    void setData(const ScheduleDtailInfo  &vScheduleInfo);
    ScheduleDtailInfo getData() const;

    void setFont(DFontSizeManager::SizeType type);
    void setOffset(const int &offset);
    int readOffset() const
    {
        return m_offset;
    }
    void setStartValue(const int value);
    void setEndValue(const int value);
    void startAnimation();
    bool isRunning()const
    {
        return m_isAnimation;
    }
public:
    static void setPressFlag(const bool flag);
public slots:
    void animationFinished();
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
protected:
    virtual void paintBackground(QPainter *painter,const QRectF &rect,const int isPixMap = false) =0;
protected:
    ScheduleDtailInfo                   m_vScheduleInfo;
    QFont                               m_font;
    bool                                m_vSelectflag =false;
    bool                                m_vHoverflag = false;
    bool                                m_vHighflag = false;
    QRectF                              m_rect;
    bool                                m_isAnimation = false;
    int                                 m_offset = 0;
    DFontSizeManager::SizeType          m_sizeType  = DFontSizeManager::T8;
    QPropertyAnimation *m_properAnimationFirst = nullptr;
    QPropertyAnimation *m_properAnimationSecond = nullptr;
    QSequentialAnimationGroup *m_Group = nullptr;
    static bool                         m_press;
    static ScheduleDtailInfo            m_HoverInfo;
};

#endif // DRAGINFOITEM_H
