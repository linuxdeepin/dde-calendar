// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "draginfoitem.h"
#include "commondef.h"

#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QDebug>
#include <QMutex>

bool DragInfoItem::m_press = false;
DSchedule::Ptr DragInfoItem::m_HoverInfo;
DSchedule::Ptr DragInfoItem::m_pressInfo;
DSchedule::List DragInfoItem::m_searchScheduleInfo;

DragInfoItem::DragInfoItem(QRectF rect, QGraphicsItem *parent)
    : CFocusItem(parent)
    , m_rect(rect)
{
    qCDebug(ClientLogger) << "DragInfoItem constructor - rect:" << rect;
    setRect(m_rect);
    setAcceptHoverEvents(true);
    const int duration = 200;
    m_properAnimationFirst = new QPropertyAnimation(this, "offset", this);
    m_properAnimationFirst->setObjectName("First");
    m_properAnimationSecond = new QPropertyAnimation(this, "offset", this);
    m_properAnimationSecond->setObjectName("Second");
    m_properAnimationFirst->setDuration(duration);
    m_properAnimationSecond->setDuration(duration);
    m_properAnimationFirst->setEasingCurve(QEasingCurve::InOutQuad);
    m_properAnimationSecond->setEasingCurve(QEasingCurve::InOutQuad);
    m_Group = new QSequentialAnimationGroup(this);
    m_Group->addAnimation(m_properAnimationFirst);
    m_Group->addAnimation(m_properAnimationSecond);
    connect(m_Group
            , &QPropertyAnimation::finished
            , this
            , &DragInfoItem::animationFinished);
    setItemType(CITEM);
}

DragInfoItem::~DragInfoItem()
{
    qCDebug(ClientLogger) << "DragInfoItem destructor";
}

void DragInfoItem::setData(const DSchedule::Ptr &vScheduleInfo)
{
    // qCDebug(ClientLogger) << "DragInfoItem::setData - schedule:" << (vScheduleInfo ? vScheduleInfo->summary() : "null");
    QMutexLocker locker(&m_Mutex);
    m_vScheduleInfo = vScheduleInfo;
}

DSchedule::Ptr DragInfoItem::getData()
{
    QMutexLocker locker(&m_Mutex);
    // qCDebug(ClientLogger) << "DragInfoItem::getData - returning schedule:" << (m_vScheduleInfo ? m_vScheduleInfo->summary() : "null");
    return  m_vScheduleInfo;
}

void DragInfoItem::setPressFlag(const bool flag)
{
    // qCDebug(ClientLogger) << "DragInfoItem::setPressFlag - flag:" << flag;
    m_press = flag;
}

/**
 * @brief DragInfoItem::setPressSchedule        记录选中日程
 * @param pressSchedule
 */
void DragInfoItem::setPressSchedule(const DSchedule::Ptr &pressSchedule)
{
    // qCDebug(ClientLogger) << "DragInfoItem::setPressSchedule - schedule:" << (pressSchedule ? pressSchedule->summary() : "null");
    m_pressInfo = pressSchedule;
}

/**
 * @brief DragInfoItem::getPressSchedule        获取选中日程
 * @return
 */
DSchedule::Ptr DragInfoItem::getPressSchedule()
{
    // qCDebug(ClientLogger) << "DragInfoItem::getPressSchedule - returning schedule:" << (m_pressInfo ? m_pressInfo->summary() : "null");
    return  m_pressInfo;
}

/**
 * @brief DragInfoItem::setSearchScheduleInfo       设置搜索日程新
 * @param searchScheduleInfo
 */
void DragInfoItem::setSearchScheduleInfo(const DSchedule::List &searchScheduleInfo)
{
    // qCDebug(ClientLogger) << "DragInfoItem::setSearchScheduleInfo - count:" << searchScheduleInfo.size();
    m_searchScheduleInfo = searchScheduleInfo;
}

void DragInfoItem::setFont(DFontSizeManager::SizeType type)
{
    // qCDebug(ClientLogger) << "DragInfoItem::setFont - type:" << type;
    m_sizeType = type;
}

void DragInfoItem::setOffset(const int &offset)
{
    qCDebug(ClientLogger) << "DragInfoItem::setOffset - offset:" << offset;
    m_offset = offset;
    setRect(QRectF(m_rect.x() - offset,
                   m_rect.y() - offset / 2,
                   m_rect.width() + offset * 2,
                   m_rect.height() + offset));
    setZValue(offset);
}

void DragInfoItem::setStartValue(const int value)
{
    qCDebug(ClientLogger) << "DragInfoItem::setStartValue - value:" << value;
    m_properAnimationFirst->setStartValue(value);
    m_properAnimationSecond->setEndValue(value);
}

void DragInfoItem::setEndValue(const int value)
{
    qCDebug(ClientLogger) << "DragInfoItem::setEndValue - value:" << value;
    m_properAnimationFirst->setEndValue(value);
    m_properAnimationSecond->setStartValue(value);
}

void DragInfoItem::startAnimation()
{
    qCDebug(ClientLogger) << "DragInfoItem::startAnimation - current state:" << m_Group->state();
    if (m_Group->state() != QAnimationGroup::Running) {
        qCDebug(ClientLogger) << "Starting animation";
        m_Group->start();
    }
}

void DragInfoItem::animationFinished()
{
    // qCDebug(ClientLogger) << "DragInfoItem::animationFinished";
    m_isAnimation = false;
}

void DragInfoItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // qCDebug(ClientLogger) << "DragInfoItem::hoverEnterEvent";
    QMutexLocker locker(&m_Mutex);
    Q_UNUSED(event);
    m_HoverInfo = m_vScheduleInfo;
    update();
}

void DragInfoItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    // qCDebug(ClientLogger) << "DragInfoItem::hoverLeaveEvent";
    QMutexLocker locker(&m_Mutex);
    Q_UNUSED(event);
    m_HoverInfo = DSchedule::Ptr();
    update();
}

void DragInfoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // qCDebug(ClientLogger) << "DragInfoItem::paint - schedule:" 
    //                      << (m_vScheduleInfo ? m_vScheduleInfo->summary() : "null");
    QMutexLocker locker(&m_Mutex);
    Q_UNUSED(option);
    Q_UNUSED(widget);
    m_vHoverflag = m_HoverInfo == m_vScheduleInfo;
    m_vHighflag = false;
    m_vSelectflag = false;
    m_vHighflag = m_searchScheduleInfo.contains(m_vScheduleInfo);
    paintBackground(painter, this->rect());
}
