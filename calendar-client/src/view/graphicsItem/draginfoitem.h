// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DRAGINFOITEM_H
#define DRAGINFOITEM_H
#include "scheduledatamanage.h"
#include "src/scheduledatainfo.h"
#include "cfocusitem.h"

#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

class QPropertyAnimation;
class QSequentialAnimationGroup;
/**
 * @brief The DragInfoItem class
 * 显示项  支持拖拽
 */
class DragInfoItem : public CFocusItem
{
    Q_OBJECT
    Q_PROPERTY(int offset READ readOffset WRITE setOffset)
public:
    explicit DragInfoItem(QRectF rect, QGraphicsItem *parent = nullptr);
    ~DragInfoItem() override;
    void setData(const ScheduleDataInfo  &vScheduleInfo);
    ScheduleDataInfo getData() const;

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
    //设置选中日程
    static void setPressSchedule(const ScheduleDataInfo &pressSchedule);
    //获取选中日程
    static ScheduleDataInfo getPressSchedule();
    //设置搜索日程
    static void setSearchScheduleInfo(const QVector<ScheduleDataInfo> &searchScheduleInfo);
public slots:
    void animationFinished();
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
protected:
    virtual void paintBackground(QPainter *painter, const QRectF &rect, const bool isPixMap = false) = 0;

protected:
    ScheduleDataInfo                   m_vScheduleInfo;
    QFont                               m_font;
    bool                                m_vSelectflag = false;
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
    static ScheduleDataInfo             m_HoverInfo;
    static ScheduleDataInfo             m_pressInfo;
    static QVector<ScheduleDataInfo>    m_searchScheduleInfo;
};

#endif // DRAGINFOITEM_H
