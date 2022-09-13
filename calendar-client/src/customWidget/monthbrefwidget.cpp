// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthbrefwidget.h"
#include "constants.h"
#include "scheduledatamanage.h"
#include "calendarglobalenv.h"

#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

CMonthDayRect *CMonthDayRect::m_CurrentRect = nullptr;
qreal                       CMonthDayRect::m_DevicePixelRatio = 0;
QColor                      CMonthDayRect::m_ceventColor("#FF5D00");
QColor                      CMonthDayRect::m_notCurrentTextColor = "#b2b2b2";
CMonthDayRect::CellColor    CMonthDayRect::m_currentColor;
QColor                      CMonthDayRect::m_defaultTextColor;
QColor                      CMonthDayRect::m_selectedTextColor;


MonthBrefWidget::MonthBrefWidget(QWidget *parent)
    : QWidget(parent)
{
    for (int i = 0; i < DDEYearCalendar::RectSizeOfEveryMonth; ++i) {
        CMonthDayRect *item = new CMonthDayRect();
        m_DayItem.append(item);
    }
    setMouseTracking(true);
    CMonthDayRect::setDevicePixelRatio(devicePixelRatioF());
}

MonthBrefWidget::~MonthBrefWidget()
{
    for (int i = 0; i < m_DayItem.size(); ++i) {
        delete m_DayItem[i];
    }
    m_DayItem.clear();
}

/**
 * @brief setDate 设置每个月的日期
 * @param date 日期
 */
void MonthBrefWidget::setDate(const int showMonth, const QVector<QDate> &showDate)
{
    Q_ASSERT(showDate.size() == m_DayItem.size());
    m_currentMonth = showMonth;
    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->setDate(showDate[i]);
        m_DayItem.at(i)->setIsCurrentMonth(showDate[i].month() == m_currentMonth);
    }
    update();
}

/**
 * @brief setTheMe 根据系统主题设置颜色
 * @param type 系统主题
 */
void MonthBrefWidget::setTheMe(int type)
{
    CMonthDayRect::setTheMe(type);
}

/**
 * @brief setLintFlag 设置是否有日程的标志
 * @param lineFlag 是否有日程的标志
 */
void MonthBrefWidget::setLintFlag(const QVector<bool> &lineFlag)
{
    if (lineFlag.size() == DDEYearCalendar::RectSizeOfEveryMonth) {
        for (int i = 0; i < DDEYearCalendar::RectSizeOfEveryMonth; ++i) {
            m_DayItem.at(i)->setLineFlag(lineFlag.at(i));
        }
    }
    update();
}

/**
 * @brief MonthBrefWidget::setSearchScheduleFlag        设置搜索日程标志
 * @param searchFlag
 */
void MonthBrefWidget::setSearchScheduleFlag(const QVector<bool> &searchFlag)
{
    if (searchFlag.size() == DDEYearCalendar::RectSizeOfEveryMonth) {
        for (int i = 0; i < DDEYearCalendar::RectSizeOfEveryMonth; ++i) {
            m_DayItem.at(i)->setSearchScheduleFlag(searchFlag.at(i));
        }
    }
    update();
}

/**
 * @brief MonthBrefWidget::updateSize       设置每天的日期所在矩形框的大小
 */
void MonthBrefWidget::updateSize()
{
    qreal w = width() / 7;
    qreal h = height() / 6;
    QRectF rect ;
    int w_offset = 0;
    int h_offset = 0;

    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        h_offset = i / 7;
        w_offset = i % 7;
        rect.setRect(w * w_offset,
                     h * h_offset,
                     w,
                     h);
        m_DayItem.at(i)->setRect(rect);
    }
    update();
}

/**
 * @brief MonthBrefWidget::getMousePosItem      获取点击日期的索引
 * @param pos
 * @return
 */
int MonthBrefWidget::getMousePosItem(const QPointF &pos)
{
    int res = -1;

    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        if (m_DayItem.at(i)->rect().contains(pos)) {
            res = i;
            break;
        }
    }
    return res;
}

/**
 * @brief MonthBrefWidget::resizeEvent      设置每天的日期所在矩形框的大小
 * @param event
 */
void MonthBrefWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateSize();
}

/**
 * @brief MonthBrefWidget::mousePressEvent      鼠标单击事件，单击日期显示当天的日程和节日。
 * @param event
 */
void MonthBrefWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果为触摸转换则设置触摸状态和触摸开始坐标
        m_touchState = 1;
        m_touchBeginPoint = event->pos();
        QWidget::mousePressEvent(event);
    } else {
        if (event->button() == Qt::LeftButton)
            mousePress(event->pos());
    }
}

/**
 * @brief MonthBrefWidget::mouseDoubleClickEvent        鼠标双击事件，双击跳转到上次打开的视图
 * @param event
 */
void MonthBrefWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    int itemindex = getMousePosItem(event->pos());

    if (!(itemindex < 0)) {
        if (m_pressIndex == itemindex) {
            m_DayItem.at(itemindex)->setCellEvent(CMonthDayRect::CellPress);
            m_press = true;
            if (event->button() == Qt::LeftButton) {
                emit signalDoubleClickDate(m_DayItem.at(itemindex)->getDate());
            }
        }
    }
}

/**
 * @brief MonthBrefWidget::mouseReleaseEvent        鼠标释放事件
 * @param event
 */
void MonthBrefWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        if (m_touchState == 1) {
            //如果为触摸且状态为点击则为触摸点击
            mousePress(event->pos());
            m_touchState = 0;
        }
        QWidget::mouseReleaseEvent(event);
    }

    m_press = false;
    mouseMoveEvent(event);
}

/**
 * @brief MonthBrefWidget::mouseMoveEvent       鼠标移动事件，设置hover状态
 * @param event
 */
void MonthBrefWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        QPoint currentPoint = event->pos();
        //如果移动距离大与5则为触摸移动状态
        if (QLineF(m_touchBeginPoint, currentPoint).length() > 5) {
            m_touchState = 2;
        }
        QWidget::mouseMoveEvent(event);
    }

    if (!m_press) {
        int itemindex = getMousePosItem(event->pos());
        if (!(itemindex < 0)) {
            m_DayItem.at(itemindex)->setCellEvent(CMonthDayRect::Cellhover);
        }
        update();
    }
}

/**
 * @brief MonthBrefWidget::paintEvent       绘制日期以及当天状态
 * @param event
 */
void MonthBrefWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        m_DayItem[i]->paintItem(&painter, m_DayItem[i]->rect());
    }
    painter.end();
}

/**
 * @brief MonthBrefWidget::leaveEvent       离开事件，设置当前选中的日期为空
 * @param event
 */
void MonthBrefWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    CMonthDayRect::setCurrentRect(nullptr);
    update();
}

/**
 * @brief MonthBrefWidget::mousePress       鼠标点击触发事件
 * @param point
 */
void MonthBrefWidget::mousePress(const QPoint &point)
{
    //获取当前item编号
    int itemindex = getMousePosItem(point);
    CalendarGlobalEnv::getGlobalEnv()->reviseValue(DDECalendar::CursorPointKey, mapToGlobal(point));
    if (!(itemindex < 0)) {
        //设置选中item为press状态
        m_DayItem.at(itemindex)->setCellEvent(CMonthDayRect::CellPress);
        m_press = true;
        m_pressIndex = itemindex;
        emit signalPressDate(m_DayItem.at(itemindex)->getDate());
    }
    update();
}

CMonthDayRect::CMonthDayRect()
    : m_rect(0, 0, 0, 0)
{
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeTwelve);
    m_hightFont.setPixelSize(DDECalendar::FontSizeTwelve);
}

/**
 * @brief CMonthDayRect::setTheMe       根据系统主题类型设置颜色
 * @param type
 */
void CMonthDayRect::setTheMe(int type)
{
    if (type == 2) {
        m_currentColor.hoverColor = "#FFFFFF";
        m_currentColor.hoverColor.setAlphaF(0.1);
        m_currentColor.pressColor = "#252525";
        m_currentColor.pressColor.setAlphaF(1);
        m_currentColor.normalColor = "#000000";
        m_currentColor.normalColor.setAlphaF(0.00);
        m_ceventColor = QColor(204, 77, 3);

        m_notCurrentTextColor = "#C0C6D4";
        m_notCurrentTextColor.setAlphaF(0.5);
        m_defaultTextColor = "#C0C6D4";
        m_selectedTextColor = Qt::white;
    } else if (type == 0 || type == 1) {
        m_currentColor.hoverColor = "#000000";
        m_currentColor.hoverColor.setAlphaF(0.05);
        m_currentColor.pressColor = "#000000";
        m_currentColor.pressColor.setAlphaF(0.2);
        m_currentColor.normalColor = "#FFFFFF";
        m_currentColor.normalColor.setAlphaF(1);
        m_ceventColor = QColor(255, 93, 0);

        m_selectedTextColor = Qt::white;
        m_defaultTextColor = Qt::black;
        m_notCurrentTextColor = "#b2b2b2";
    }
}

CMonthDayRect::~CMonthDayRect()
{

}

/**
 * @brief CMonthDayRect::setDate        设置某一天的时间
 * @param date
 */
void CMonthDayRect::setDate(const QDate &date)
{
    m_date = date;
}

/**
 * @brief CMonthDayRect::getDate        获取某一天的时间
 * @return
 */
QDate CMonthDayRect::getDate() const
{
    return  m_date;
}

/**
 * @brief CMonthDayRect::setCellEvent       设置某一天的状态，CellEventType
 * @param type
 */
void CMonthDayRect::setCellEvent(const CMonthDayRect::CellEventType &type)
{
    m_cellEventType = type;
    m_CurrentRect = this;
}

/**
 * @brief CMonthDayRect::setIsCurrentMonth      设置是否是当前月
 * @param isCurrMonth
 */
void CMonthDayRect::setIsCurrentMonth(const bool isCurrMonth)
{
    m_isCurrentMonth = isCurrMonth;
}

/**
 * @brief CMonthDayRect::rect       当天日期所在的矩形
 * @return
 */
QRectF CMonthDayRect::rect() const
{
    return  m_rect;
}

/**
 * @brief CMonthDayRect::setRect        设置当天日期的矩形
 * @param rect
 */
void CMonthDayRect::setRect(const QRectF &rect)
{
    m_rect = rect;
}

/**
 * @brief CMonthDayRect::setRect        设置当天日期的矩形
 * @param x
 * @param y
 * @param w
 * @param h
 */
void CMonthDayRect::setRect(qreal x, qreal y, qreal w, qreal h)
{
    m_rect.setRect(x, y, w, h);
}

/**
 * @brief CMonthDayRect::paintItem  绘制年视图一天的所有状态并展示出来。
 * @param painter
 * @param rect
 */
void CMonthDayRect::paintItem(QPainter *painter, const QRectF &rect)
{
    m_highColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();

    if (m_date.year() < DDECalendar::QueryEarliestYear)
        return;
    const bool isCurrentDay = m_date == QDate::currentDate() && m_isCurrentMonth;
    bool isSelectedCell  = false;
    if (isCurrentDay) {
        isSelectedCell = true;
    }

    const qreal r = rect.width() > rect.height() ? rect.height() * 0.9 : rect.width() * 0.9 ;
    int fontsize = qRound(DDECalendar::FontSizeTwelve + (r - 18) * 6 / 17);

    if (fontsize < DDECalendar::FontSizeTwelve) {
        fontsize = DDECalendar::FontSizeTwelve;
    }
    m_dayNumFont.setPixelSize(fontsize);
    m_hightFont.setPixelSize(fontsize);
    const qreal x = rect.x() + (rect.width() - r) / 2;
    const qreal y = rect.y() + (rect.height() - r) / 2;
    QRectF fillRect = QRectF(x, y, r, r);
    QColor m_cellBackgroundColor;

    if (m_CurrentRect != this) {
        m_cellEventType = CellNormal;
    }
    if (m_cellEventType == CellPress) {
        m_cellBackgroundColor = m_currentColor.pressColor;
    } else if (m_cellEventType == Cellhover) {
        m_cellBackgroundColor = m_currentColor.hoverColor;
    } else {
        m_cellBackgroundColor = m_currentColor.normalColor;
    }
    painter->setBrush(m_cellBackgroundColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(fillRect);

    bool highflag = false;

    if (m_isCurrentMonth) {
        highflag = m_searchScheduleFlag;
    }
    if (highflag) {
        painter->setBrush(QBrush(m_highColor));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(fillRect);
        painter->setPen(Qt::SolidLine);
        painter->setPen(m_highTextColor);
        painter->setFont(m_hightFont);
        painter->drawText(fillRect, Qt::AlignCenter, QString::number(m_date.day()));
    } else {
        // draw selected cell background circle
        if (isSelectedCell) {
            painter->setBrush(QBrush(m_highColor));
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(fillRect);
        }
        painter->setPen(Qt::SolidLine);

        // draw text of day
        if (isSelectedCell) {
            painter->setPen(m_selectedTextColor);
        } else if (isCurrentDay) {
            painter->setPen(m_currentDayTextColor);
        } else {
            if (m_isCurrentMonth)
                painter->setPen(m_defaultTextColor);
            else
                painter->setPen(m_notCurrentTextColor);
        }
        painter->setFont(m_dayNumFont);
        painter->drawText(fillRect, Qt::AlignCenter, QString::number(m_date.day()));

        if (m_vlineflag) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setRenderHint(QPainter::HighQualityAntialiasing);
            painter->setRenderHint(QPainter::SmoothPixmapTransform);
            QPen pen;
            pen.setWidth(2);
            pen.setColor(m_ceventColor);
            painter->setPen(pen);
            painter->setBrush(QBrush(m_ceventColor));
            painter->setPen(Qt::NoPen);
            qreal ellipse_r = rect.width() * (4 / 25);
            if (ellipse_r < 4) {
                ellipse_r = 4;
            } else if (r > 7) {
                ellipse_r = 7;
            }
            painter->drawEllipse(QRectF(rect.width() - ellipse_r + rect.x(), rect.y(), ellipse_r, ellipse_r));
            painter->restore();
        }
    }
}

/**
 * @brief CMonthDayRect::setLineFlag        设置是否有日程的标志
 * @param flag
 */
void CMonthDayRect::setLineFlag(const bool flag)
{
    m_vlineflag = flag;
}

/**
 * @brief CMonthDayRect::setSearchScheduleFlag      设置是否有搜索日程标志
 * @param flag
 */
void CMonthDayRect::setSearchScheduleFlag(const bool flag)
{
    m_searchScheduleFlag = flag;
}

/**
 * @brief CMonthDayRect::setDevicePixelRatio        设置显示缩放比例
 * @param pixel
 */
void CMonthDayRect::setDevicePixelRatio(const qreal pixel)
{
    m_DevicePixelRatio = pixel;
}

/**
 * @brief CMonthDayRect::setCurrentRect     设置当前天
 * @param currrect
 */
void CMonthDayRect::setCurrentRect(CMonthDayRect *currrect)
{
    m_CurrentRect = currrect;
}

void CMonthDayRect::setSystemActiveColor(const QColor &activeColor)
{
    m_highColor = activeColor;
}
