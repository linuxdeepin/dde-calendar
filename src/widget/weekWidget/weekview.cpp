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

#include "weekview.h"
#include "scheduledatamanage.h"
#include "constants.h"

#include <DWidget>
#include <DHiDPIHelper>

#include <QHBoxLayout>
#include <QPainter>
#include <QBrush>
#include <QEvent>
#include <QMessageBox>
#include <QWheelEvent>
#include <QtGlobal>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE
/**
 * @brief CWeekView 构造函数
 * @param parent 父类
 */
CWeekView::CWeekView(QWidget *parent)
    : QWidget(parent)
{
    //设置字体大小
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeSixteen);
    //设置字体权重
    m_dayNumFont.setWeight(QFont::Light);
    // cells grid
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->setMargin(0);
    hboxLayout->setSpacing(0);
    //显示周数的widget
    for (int c = 0; c != DDEWeekCalendar::NumWeeksDisplayed; ++c) {
        QWidget *cell = new QWidget;
        //设置大小
        cell->setFixedSize(DDEWeekCalendar::WWeekCellWidth, DDEWeekCalendar::WWeekCellHeight);
        //设置事件过滤器
        cell->installEventFilter(this);
        cell->setFocusPolicy(Qt::ClickFocus);
        //添加到布局中
        hboxLayout->addWidget(cell);
        m_cellList.append(cell);
    }
    //设置布局
    setLayout(hboxLayout);
    //设置最小宽度
    setMinimumWidth(150);
}
/**
  * @brief ~CWeekView 析构函数
  */
CWeekView::~CWeekView()
{
    for (int i = 0; i < DDEWeekCalendar::NumWeeksDisplayed; i++) {
        m_cellList.at(i)->removeEventFilter(this);
        delete m_cellList.at(i);
    }
    m_cellList.clear();
}
/**
 * @brief setFirstWeekDay 设置每周的第一天是周几，并更新
 * @param weekday 周几
 */
void CWeekView::setFirstWeekDay(int weekday)
{
    m_firstWeekDay = weekday;
    updateDate();
}
/**
 * @brief setCurrentDate 设置当天时间，并更新
 * @param date 时间
 */
void CWeekView::setCurrentDate(const QDate date)
{
    m_selectDate = date;
    m_selectedCell = 0;
    updateDate();
    update();
}
/**
 * @brief setTheMe 根据系统主题类型设置颜色
 * @param type 系统主题类型
 */
void CWeekView::setTheMe(int type)
{
    m_themetype = type;
    //根据系统主题类型设置颜色
    if (type == 0 || type == 1) {
        //系统主题类型为浅色或跟随系统
        //默认字体颜色
        m_defaultTextColor = Qt::black;
        //默认背景色
        m_backgrounddefaultColor = Qt::white;
        //默认当前日期字体颜色
        m_currentDayTextColor = Qt::white;
        //系统活动色
        m_backgroundcurrentDayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        //背景色
        m_fillColor = "#FFFFFF";
    } else if (type == 2) {
        //系统主题类型为神色
        //设置默认字体颜色
        m_defaultTextColor = "#C0C6D4";
        //设置默认背景色
        m_backgrounddefaultColor = "#FFFFFF";
        //设置透明度
        m_backgrounddefaultColor.setAlphaF(0.05);
        //设置当前日期字体颜色
        m_currentDayTextColor = "#B8D3FF";
        //系统活动色
        m_backgroundcurrentDayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        //设置背景色
        m_fillColor = "#000000";
        //透明度
        m_fillColor.setAlphaF(0.05);
    }
}
/**
 * @brief setsearchfalg
 * @param flag
 */
void CWeekView::setsearchfalg(bool flag)
{
    m_searchfalg = flag;
}
/**
 * @brief slotprev 选择上一周
 */
void CWeekView::slotprev()
{
    if (m_selectDate.year() >= DDECalendar::QueryEarliestYear) {
        QDate date = m_selectDate.addDays(-6);
        setCurrentDate(date);
    } else {
    }
}
/**
 * @brief slotnext 选择下一周
 */
void CWeekView::slotnext()
{
    QDate date  = m_selectDate.addDays(8);;
    setCurrentDate(date);
}
/**
 * @brief paintCell 绘制周数
 * @param cell 周数所在的widget
 */
void CWeekView::paintCell(QWidget *cell)
{
    //矩形
    const QRect rect(0, 0, cell->width(), cell->height());
    //cell所在的索引
    const int pos = m_cellList.indexOf(cell);
    //计算当前日期周数
    int weekNumber = QDate::currentDate().weekNumber();
    //In accordance with ISO 8601, weeks start on Monday and the first
    if (QDate::currentDate().dayOfWeek() == DDEWeekCalendar::AFewDaysofWeek) {
        ++weekNumber;
    }
    //是否是今天
    const bool isCurrentDay = m_days[pos].addDays(3).weekNumber() == weekNumber &&
                              m_days[pos].addDays(3).year() == QDate::currentDate().year();
    //是否是选择的周数
    const bool isSelectDay = m_days[pos].addDays(m_weekAddDay).weekNumber() == m_selectDate.addDays(m_weekAddDay).weekNumber();
    //绘制cell
    QPainter painter(cell);
    painter.save();
    painter.setRenderHints(QPainter::HighQualityAntialiasing);
    painter.setBrush(QBrush(m_fillColor));
    painter.setPen(Qt::NoPen);
    //画矩形
    painter.drawRect(rect);
    painter.restore();
    painter.setPen(Qt::SolidLine);

    const QString dayNum = QString::number(m_days[pos].addDays(3).weekNumber());
    //早于1900年第一周，return
    if (m_days[pos].year() < DDECalendar::QueryEarliestYear && dayNum != "1")
        return;

    if (isSelectDay) {
        //选择周数
        QRect fillRect((cell->width() - 24) / 2, (cell->height() - 32) / 2 + 4, 24, 24);
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor()));
        painter.setPen(Qt::NoPen);
        //绘制背景圆
        painter.drawEllipse(fillRect);
        painter.restore();
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        //设置文字颜色
        painter.setPen(m_currentDayTextColor);
        //设置字体
        painter.setFont(m_dayNumFont);
        //绘制周数
        painter.drawText(QRect(0, 0, cell->width(), cell->height()), Qt::AlignCenter, dayNum);
    } else {
        if (isCurrentDay) {
            //设置今天的背景色
            painter.setPen(m_backgroundcurrentDayColor);
        } else {
            //设置默认的背景色
            painter.setPen(m_defaultTextColor);
        }
        //设置字体
        painter.setFont(m_dayNumFont);
        //绘制周数
        painter.drawText(QRect(0, 0, cell->width(), cell->height()), Qt::AlignCenter, dayNum);
    }
    painter.end();
}
/**
 * @brief eventFilter 过滤器
 * @param o 事件对象
 * @param e 事件类型
 * @return  false
 */
bool CWeekView::eventFilter(QObject *o, QEvent *e)
{
    QWidget *cell = qobject_cast<QWidget *>(o);

    if (cell && m_cellList.contains(cell)) {
        if (e->type() == QEvent::Paint) {
            //绘图事件
            paintCell(cell);
        } else if (e->type() == QEvent::MouseButtonPress) {
            //鼠标左击事件
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
            if (mouseEvent->button() ==Qt::LeftButton) {
                cellClicked(cell);
            }
        }
    }

    return false;
}
/**
 * @brief cellClicked 选择点击的日期
 * @param cell 周数所在的widget
 */
void CWeekView::cellClicked(QWidget *cell)
{
    if (!m_cellSelectable)
        return;
    //鼠标点击的索引
    const int pos = m_cellList.indexOf(cell);
    //没有return
    if (pos == -1)
        return;
    //设置选择的cell
    setSelectedCell(pos);
    update();
}
/**
 * @brief setSelectedCell 设置被选择的周数
 * @param index 周数所在的索引
 */
void CWeekView::setSelectedCell(int index)
{
    if (m_selectedCell == index)
        return;
    //选择的索引
    const int prevPos = m_selectedCell;
    m_selectedCell = index;
    //更新
    m_cellList.at(prevPos)->update();
    m_cellList.at(index)->update();
    //设置选择的日期
    m_selectDate = m_days[index];
    const QString dayNum = QString::number(m_days[index].addDays(3).weekNumber());
    //早于1900年第一周，return
    if (m_days[index].year() < DDECalendar::QueryEarliestYear && dayNum != "1")
        return;

    emit signalsSelectDate(m_days[index], m_days[index].addDays(m_weekAddDay));
}
/**
 * @brief updateDate 更新数据
 */
void CWeekView::updateDate()
{
    m_weekAddDay = (m_selectDate.dayOfWeek() + m_firstWeekDay) % DDEWeekCalendar::AFewDaysofWeek;
    QDate weekfirstDay = m_selectDate.addDays(-m_weekAddDay);
    m_days[4] = weekfirstDay;
    //设置前四个周数
    for (int i(0); i < 4; ++i) {
        m_days[i] = weekfirstDay.addDays(-(4 - i) * DDEWeekCalendar::AFewDaysofWeek);
    }
    //设置后五个周数
    for (int i(5); i != DDEWeekCalendar::NumWeeksDisplayed; ++i) {
        m_days[i] = weekfirstDay.addDays((i - 4) * DDEWeekCalendar::AFewDaysofWeek);
    }
    //默认设置选择第四个
    setSelectedCell(4);
    update();
}
/**
 * @brief resizeEvent 调整窗口大小
 * @param event 窗口大小调整事件
 */
void CWeekView::resizeEvent(QResizeEvent *event)
{
    int w = width() / DDEWeekCalendar::NumWeeksDisplayed;
    int h = height();
    int ww = 36;

    if (w >= ww) {
        for (int c = 0; c != DDEWeekCalendar::NumWeeksDisplayed; ++c) {
            m_cellList[c]->setFixedSize(w, h);
            m_cellList[c]->update();
        }
        for (int i = 0; i < DDEWeekCalendar::NumWeeksDisplayed; i++) {
            m_cellList[i]->setVisible(true);
            m_cellList[i]->update();
        }
    } else {
        for (int c = 0; c != DDEWeekCalendar::NumWeeksDisplayed; ++c) {
            m_cellList[c]->setFixedSize(ww, h);
            m_cellList[c]->update();
        }
        int t_num = qRound((ww * DDEWeekCalendar::NumWeeksDisplayed - width()) / ww / 2.0);
        QVector<bool> vindex;
        vindex.resize(10);
        vindex.fill(true);

        for (int i = 0; i < t_num; i++) {
            vindex[i] = false;
            vindex[9 - i] = false;
        }
        for (int i = 0; i < DDEWeekCalendar::NumWeeksDisplayed; i++) {
            m_cellList[i]->setVisible(vindex[i]);
            m_cellList[i]->update();
        }
    }
    QWidget::resizeEvent(event);
}
/**
 * @brief wheelEvent 鼠标滚轮切换上一周下一周
 * @param event 鼠标滚轮事件
 */
void CWeekView::wheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event);
    bool isDragging = false;
    //
    emit signalIsDragging(isDragging);
    //判断是否是拖拽状态
    if (!isDragging) {
        if (event->delta() < 0) {
            //上一周
            slotnext();
        } else {
            //下一周
            slotprev();
        }
    }

}
