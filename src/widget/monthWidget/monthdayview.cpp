/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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

#include "monthdayview.h"
#include "scheduledatamanage.h"
#include "constants.h"

#include <DPalette>
#include <DHiDPIHelper>

#include <QHBoxLayout>
#include <QPainter>
#include <QBrush>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

DGUI_USE_NAMESPACE
/**
 * @brief CMonthDayView 构造函数
 * @param parent 父类
 */
CMonthDayView::CMonthDayView(QWidget *parent)
    : DFrame(parent)
{
    //新建水平布局
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->setMargin(0);
    hboxLayout->setSpacing(0);
    hboxLayout->setContentsMargins(10, 0, 10, 0);
    //新建月份的widget
    m_monthWidget = new CMonthWidget(this);
    //将月份的widget添加到水平布局中
    hboxLayout->addWidget(m_monthWidget);
    //设置布局为水平布局
    setLayout(hboxLayout);
    //设置圆角
    setFrameRounded(true);
    setLineWidth(0);
    //设置windowflags
    setWindowFlags(Qt::FramelessWindowHint);
    //连接信号
    connect(m_monthWidget,
            &CMonthWidget::signalsSelectDate,
            this,
            &CMonthDayView::signalsSelectDate);
}
/**
 * @brief ~CMonthDayView 析构函数
 */
CMonthDayView::~CMonthDayView()
{
}
/**
 * @brief setCurrentDate 设置当前月份
 * @param date 日期
 */
void CMonthDayView::setCurrentDate(const QDate date)
{
    //选择的日期
    m_selectDate = date;
    //中间的月份
    m_days[5] = m_selectDate;
    //设置前面的月份
    for (int i(4); i >= 0; i--) {
        m_days[4 - i] = m_selectDate.addMonths(-i - 1);
    }
    //设置后面的月份
    for (int i(6); i != DDEMonthCalendar::MonthNumofYear; ++i) {
        m_days[i] = m_selectDate.addMonths(i - 5);
    }
    //更新数据
    update();
    //设置数据
    m_monthWidget->setDate(m_days);
}
/**
 * @brief setRCurrentDate 设置选中的月份
 * @param date 日期
 */
void CMonthDayView::setRCurrentDate(const QDate date)
{
    if (m_selectDate == date) return;

    m_selectDate = date;
    m_days[5] = m_selectDate;

    for (int i(4); i >= 0; i--) {
        m_days[4 - i] = m_selectDate.addMonths(-i - 1);
    }

    for (int i(6); i != DDEMonthCalendar::MonthNumofYear; ++i) {
        m_days[i] = m_selectDate.addMonths(i - 5);
    }

    m_monthWidget->setDate(m_days);
    update();
}
/**
 * @brief setTheMe 根据系统类型设置颜色
 * @param type 系统类型
 */
void CMonthDayView::setTheMe(int type)
{
    QColor frameclor;
    //根据系统类型设置颜色
    if (type == 0 || type == 1) {
        frameclor = "#FFFFFF";
    } else if (type == 2) {
        frameclor = "#FFFFFF";
        frameclor.setAlphaF(0.05);
    }
    //设置背景色
    DPalette anipa = palette();
    anipa.setColor(DPalette::Background, frameclor);
    setPalette(anipa);
    setBackgroundRole(DPalette::Background);
    CMonthRect::setTheMe(type);
}
/**
 * @brief setsearchfalg
 * @param flag
 */
void CMonthDayView::setsearchfalg(bool flag)
{
    m_searchfalg = flag;
}
/**
 * @brief CMonthWidget 构造函数
 * @param parent 父类
 */
CMonthWidget::CMonthWidget(QWidget *parent)
    :QWidget(parent)
{
    //新建12个月份的rect
    for (int i = 0; i < DDEMonthCalendar::MonthNumofYear; ++i) {
        CMonthRect *monthrect = new CMonthRect();
        //将每个月份的rect添加到容器中
        m_MonthItem.append(monthrect);
    }
}
/**
  * @brief ~CMonthWidget 析构函数
  */
CMonthWidget::~CMonthWidget()
{
    for (int i = 0; i < DDEMonthCalendar::MonthNumofYear; ++i) {
        CMonthRect *monthrect = m_MonthItem.at(i);
        delete  monthrect;
    }

    m_MonthItem.clear();
}
/**
 * @brief setDate 设置月份
 * @param date 12个月份
 */
void CMonthWidget::setDate(const QDate date[12])
{
    //设置时间
    for (int i = 0; i < DDEMonthCalendar::MonthNumofYear; ++i) {
        m_MonthItem.at(i)->setDate(date[i]);
    }
    //设置选择的rect
    CMonthRect::setSelectRect(m_MonthItem.at(5));
    update();
}
/**
 * @brief resizeEvent 更新显示
 * @param event 窗口大小调整事件
 */
void CMonthWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateSize();
}
/**
 * @brief mousePressEvent 鼠标单击事件
 * @param event 鼠标事件
 */
void CMonthWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() ==Qt::RightButton)
        return;

    int itemindex = getMousePosItem(event->pos());

    if (!(itemindex<0)) {
        if (m_MonthItem.at(itemindex)->getDate().year() < DDECalendar::QueryEarliestYear) {
            return;
        }
        CMonthRect::setSelectRect(m_MonthItem.at(itemindex));
        emit signalsSelectDate(m_MonthItem.at(itemindex)->getDate());
    }
    update();
}
/**
 * @brief paintEvent 绘制item
 * @param event 绘图事件
 */
void CMonthWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    // 反锯齿;
    painter.setRenderHint(QPainter::Antialiasing);
    //绘制月份
    for (int i = 0; i < m_MonthItem.size(); ++i) {
        m_MonthItem.at(i)->paintItem(&painter,
                                     m_MonthItem.at(i)->rect());
    }
    painter.end();
}
/**
 * @brief updateSize 更新item的大小
 */
void CMonthWidget::updateSize()
{
    qreal w= this->width()/m_MonthItem.size();
    //更新每个rect的大小
    for (int i = 0; i < m_MonthItem.size(); ++i) {
        m_MonthItem.at(i)->setRect(i*w,0,w,this->height());
    }
    update();
}
/**
 * @brief getMousePosItem 获取鼠标点击月份的索引
 * @param pos 鼠标点击的点
 * @return 月份的所在位置的索引
 */
int CMonthWidget::getMousePosItem(const QPointF &pos)
{
    int res =-1;
    //获取点击rect的索引
    for (int i = 0 ; i < m_MonthItem.size(); ++i) {
        if (m_MonthItem.at(i)->rect().contains(pos)) {
            res = i;
            break;
        }
    }

    return res;
}


int         CMonthRect::m_themetype ;
qreal       CMonthRect::m_DevicePixelRatio;

QColor      CMonthRect::m_defaultTextColor;
QColor      CMonthRect::m_backgrounddefaultColor ;
QColor      CMonthRect::m_currentDayTextColor;
QColor      CMonthRect::m_backgroundcurrentDayColor;
QColor      CMonthRect::m_fillColor;
QFont       CMonthRect::m_dayNumFont;
CMonthRect         *CMonthRect::m_SelectRect = nullptr;

/**
 * @brief CMonthRect 构造函数
 */
CMonthRect::CMonthRect()
{
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeSixteen);
    m_dayNumFont.setWeight(QFont::Light);
}
/**
 * @brief setDate 设置日期
 * @param date 日期
 */
void CMonthRect::setDate(const QDate &date)
{
    m_Date = date;
}
/**
 * @brief getDate 获取日期
 * @return 日期
 */
QDate CMonthRect::getDate() const
{
    return  m_Date;
}
/**
 * @brief rect 月份所在的矩形
 * @return 矩形
 */
QRectF CMonthRect::rect() const
{
    return  m_rect;
}
/**
 * @brief setRect 设置月份所在矩形
 * @param rect 矩形
 */
void CMonthRect::setRect(const QRectF &rect)
{
    m_rect = rect;
}
/**
 * @brief setRect 设置矩形大小
 * @param x 起点x
 * @param y 起点y
 * @param w 宽度w
 * @param h 高度h
 */
void CMonthRect::setRect(qreal x, qreal y, qreal w, qreal h)
{
    m_rect.setRect(x,y,w,h);
}
/**
 * @brief paintItem 绘制item
 * @param painter 绘图函数
 * @param rect 月份所在矩形
 */
void CMonthRect::paintItem(QPainter *painter, const QRectF &rect)
{
    //选择的rect颜色
    m_selectColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    //如果时间早于1900年，return
    if (m_Date.year() < DDECalendar::QueryEarliestYear)
        return;
    //是否是今天
    const bool isCurrentDay = (m_Date.month() == QDate::currentDate().month()
                               && m_Date.year() == QDate::currentDate().year());

    painter->setPen(Qt::SolidLine);
    //月份数字
    const QString dayNum = QString::number(m_Date.month());
    //被选中的月份
    if (m_SelectRect ==this) {
        //设置矩形
        QRectF fillRect((rect.width() - 36) / 2 +rect.x() + 6,
                        (rect.height() - 36) / 2 + 7 +rect.y(),
                        24,
                        24);
        painter->setBrush(QBrush(m_selectColor));
        painter->setPen(Qt::NoPen);
        //绘制圆形
        painter->drawEllipse(fillRect);
        painter->setRenderHint(QPainter::HighQualityAntialiasing);
        painter->setPen(m_currentDayTextColor);
        painter->setFont(m_dayNumFont);
        //绘制月份
        painter->drawText(rect, Qt::AlignCenter, dayNum);
    } else {
        //没被选中
        if (isCurrentDay) {
            //设置当前月的字体颜色
            painter->setPen(m_backgroundcurrentDayColor);
        } else {
            //设置其他月字体颜色
            painter->setPen(m_defaultTextColor);
        }
        painter->setFont(m_dayNumFont);
        //绘制月份
        painter->drawText(rect, Qt::AlignCenter, dayNum);
    }

}
/**
 * @brief setDevicePixelRatio
 * @param pixel
 */
void CMonthRect::setDevicePixelRatio(const qreal pixel)
{
    m_DevicePixelRatio = pixel;
}
/**
 * @brief setTheMe 根据系统类型设置颜色
 * @param type 系统类型
 */
void CMonthRect::setTheMe(int type)
{
    m_themetype = type;
    QColor frameclor;
    //根据系统主题类型设置颜色
    if (type == 0 || type == 1) {
        //系统主题类型为浅色或跟随系统
        m_defaultTextColor = Qt::black;
        m_backgrounddefaultColor = Qt::white;
        m_currentDayTextColor = Qt::white;
        m_backgroundcurrentDayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        m_fillColor = "#FFFFFF";
        frameclor = m_fillColor;
        m_fillColor.setAlphaF(0);
    } else if (type == 2) {
        //系统主题类型为深色
        m_defaultTextColor = "#C0C6D4";
        QColor framecolor = Qt::black;
        framecolor.setAlphaF(0.5);
        m_backgrounddefaultColor = framecolor;
        m_currentDayTextColor = "#C0C6D4";
        m_backgroundcurrentDayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        m_fillColor = "#FFFFFF";
        m_fillColor.setAlphaF(0.05);
        frameclor = m_fillColor;
        m_fillColor.setAlphaF(0);
    }
}
/**
 * @brief setSelectRect 设置选择的矩形
 * @param selectRect 选择的矩形
 */
void CMonthRect::setSelectRect(CMonthRect *selectRect)
{
    m_SelectRect =selectRect;
}
