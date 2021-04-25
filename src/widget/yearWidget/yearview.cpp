
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

#include "calendardbus.h"
#include "scheduledatamanage.h"
#include "yearschceduleview.h"
#include "yearview.h"
#include "constants.h"

#include <DArrowRectangle>
#include <DApplication>
#include <DHiDPIHelper>
#include <DPalette>

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QQueue>
#include <QSpacerItem>
#include <QLocale>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainterPath>

DGUI_USE_NAMESPACE
CYearSchceduleOutView      *CYearView::m_Scheduleview = nullptr;
CMonthDayRect               *CMonthDayRect::m_CurrentRect =nullptr;
qreal                       CMonthDayRect::m_DevicePixelRatio = 0;
QColor                      CMonthDayRect::m_ceventColor("#FF5D00");
QColor                      CMonthDayRect::m_notCurrentTextColor = "#b2b2b2";
CMonthDayRect::CellColor    CMonthDayRect::m_currentColor;
QColor                      CMonthDayRect::m_defaultTextColor;
QColor                      CMonthDayRect::m_selectedTextColor;

/**
 * @brief CYearView 构造函数
 * @param parent 父类
 */
CYearView::CYearView(QWidget *parent)
    : CustomFrame(parent)
{
    //启用鼠标跟踪
    setMouseTracking(true);
    //add separator line
    m_currentMouth = new CustomFrame();
    m_currentMouth->setFixedHeight(24);
    m_currentMouth->setContentsMargins(0, 0, 0, 0);
    m_currentMouth->setTextAlign(Qt::AlignLeft);
    //设置m_currentMouth的字体
    m_momthFont.setWeight(QFont::Medium);
    m_momthFont.setPixelSize(DDECalendar::FontSizeSixteen);
    m_currentMouth->setTextFont(m_momthFont);
    //添加年视图月份的布局
    QHBoxLayout *separatorLineLayout = new QHBoxLayout;
    separatorLineLayout->setMargin(0);
    separatorLineLayout->setSpacing(0);
    separatorLineLayout->setContentsMargins(0, 0, 0, 0);
    //将月份添加到布局中
    separatorLineLayout->addWidget(m_currentMouth);
    separatorLineLayout->addStretch();

    m_currentMouth->show();
    //为月份添加事件过滤器
    m_currentMouth->installEventFilter(this);
    //添加每个月的widget
    m_monthView = new CYearMonthView(this);
    connect(m_monthView,
            &CYearMonthView::signalPressDate,
            this,
            &CYearView::slotPressClickDate);
    connect(m_monthView,
            &CYearMonthView::signalDoubleClickDate,
            this,
            &CYearView::slotDoubleClickDate);
    //添加总体布局
    m_hhLayout = new QVBoxLayout;
    //将月份的布局添加到总体布局中
    m_hhLayout->addLayout(separatorLineLayout);
    //将每个月的widget添加到总体布局中
    m_hhLayout->addWidget(m_monthView);
    m_hhLayout->setMargin(0);
    m_hhLayout->setSpacing(0);
    m_hhLayout->setContentsMargins(13, 10, 10, 10);
    //设置总体布局
    setLayout(m_hhLayout);
    createYearSchceduleView(parent);
}
/**
 * @brief slotupdateSchedule 设置当前时间
 * @param id
 */
void CYearView::slotupdateSchedule(const int id)
{
    //设置当前的日期
    setCurrentDate(m_currentDate, id);
}

/**
 * @brief slotDoubleClickDate 隐藏日程浮框，并发出鼠标双击的信号
 * @param date 传递时间参数
 */
void CYearView::slotDoubleClickDate(const QDate &date)
{
    //如果时间早于1900年，return
    if (date.year() < DDECalendar::QueryEarliestYear)
        return;
    //隐藏日程浮框
    SchceduleViewHide();
    emit signaldoubleclickDate(date);
}

/**
 * @brief slotPressClickDate 鼠标单击日期，显示日程浮框
 * @param date 传递日程参数
 */
void CYearView::slotPressClickDate(const QDate &date)
{
    //如果时间早于1900年，return
    if (date.year() < DDECalendar::QueryEarliestYear)
        return;
    emit signalcurrentDateChanged(date);
    emit signalHideInfo();
    emit signalSelectInfo(true);
    //设置选中的时间
    m_Scheduleview->setCurrentDate(date);
    //设置当天的日程信息
    for (int i = 0; i < m_DateRangeInfo.size(); ++i) {
        if (m_DateRangeInfo.at(i).date ==date) {
            m_Scheduleview->setData(m_DateRangeInfo[i].vData);
        }
    }
    //鼠标点击的点
    QPoint pos22 = QCursor::pos();
    //屏幕信息
    QDesktopWidget *w = QApplication::desktop();
    m_Scheduleview->showWindow();

    if (pos22.x() + 10 + m_Scheduleview->width() < w->width()) {
        //日程浮框右边位置小于屏幕宽度，日程浮框在点击日期的右边
        m_Scheduleview->setArrowDirection(DArrowRectangle::ArrowLeft);
        m_Scheduleview->show(pos22.x() + 10, pos22.y());
    } else {
        //日程浮框右边位置大于等于屏幕宽度，日程浮框在点击日期的左边
        m_Scheduleview->adjustPosition(true);
        m_Scheduleview->setArrowDirection(DArrowRectangle::ArrowRight);
        m_Scheduleview->show(pos22.x() - 10, pos22.y());
    }
}

/**
 * @brief setFirstWeekday 设置日历每周是以周几开始的
 * @param weekday 星期几
 */
void CYearView::setFirstWeekday(int weekday)
{
    //设置每周以周几开始
    m_firstWeekDay = weekday;
    //更新数据
    updateDate();
}

/**
 * @brief setTheMe 根据系统主题类型，设置颜色
 * @param type 系统主题类型
 */
void CYearView::setTheMe(int type)
{
    m_themetype = type;
    //系统类型为浅色或跟随系统
    if (type == 0 || type == 1) {
        //设置背景颜色
        DPalette bpa = palette();
        bpa.setColor(DPalette::Background, Qt::white);
        setPalette(bpa);
        setBackgroundRole(DPalette::Background);
        //设置月份的颜色
        m_currentMouth->setTextColor( QColor("#CF0059"));

        m_bnormalColor = "#FFFFFF";
        m_topBorderColor = Qt::red;
        m_backgroundCircleColor = "#0081FF";
        //设置今天的颜色
        m_currentDayTextColor = "#0081FF";
        m_weekendsTextColor = Qt::black;
        m_festivalTextColor = Qt::black;
    } else if (type == 2) {
        //系统类型为深色
        m_bnormalColor = "#FFFFFF";
        m_bnormalColor.setAlphaF(0.05);
        //设置月份的颜色
        m_currentMouth->setTextColor( QColor("#BF1D63"));

        m_topBorderColor = Qt::red;
        m_backgroundCircleColor = "#0059D2";
        //设置今天的颜色
        m_currentDayTextColor = "#0059D2";
        m_weekendsTextColor = Qt::black;
        m_festivalTextColor = Qt::black;
    }
    //设置每个月的系统类型
    m_monthView->setTheMe(type);
    QColor monthcolor = Qt::white;
    monthcolor.setAlphaF(0);
    m_currentMouth->setBColor(monthcolor);
    //设置日程的系统类型
    m_Scheduleview->setTheMe(type);
}

/**
 * @brief SchceduleViewHide 隐藏日程浮框
 */
void CYearView::SchceduleViewHide()
{
    m_Scheduleview->hide();
    m_Scheduleview->clearData();
}

/**
 * @brief getInfoAndSetLineFlag 获取日程信息，设置是否有日程的标志
 */
void CYearView::getInfoAndSetLineFlag()
{
    CScheduleDataManage *tdataManage = CScheduleDataManage::getScheduleDataManage();
    //设置节假日和日程标志大小
    m_vlineflag.resize(42);
    //设置初始值
    m_vlineflag.fill(false);
    //如果这一天有节假日或日程，设置标志
    if (tdataManage->getGetAllYearScheduleInfo()->m_monthInfo.size()>0) {
        m_DateRangeInfo = tdataManage->getGetAllYearScheduleInfo()->m_monthInfo[m_currentDate.month()];
        if (m_DateRangeInfo.count() == DDEYearCalendar::RectSizeofEveyMonth) {
            for (int i = 0; i < DDEYearCalendar::RectSizeofEveyMonth; i++) {
                if (!m_DateRangeInfo.at(i).vData.isEmpty()) {
                    m_vlineflag[i] = true;
                }
            }
        }
    }
    //设置一个月的日程或节假日标志
    m_monthView->setLintFlag(m_vlineflag);
    m_monthView->update();
}

/**
 * @brief setCurrentDate 设置当前的日期
 * @param date 日期
 * @param type
 */
void CYearView::setCurrentDate(const QDate date, int type)
{
    Q_UNUSED(type);
    qDebug() << "set current date " << date;
    //设置当前日期
    m_currentDate = date;
    QLocale locale;
    //设置月份
    m_currentMouth->setTextStr(locale.monthName(date.month(), QLocale::ShortFormat));
    //更新数据
    updateDate();
    //设置一个月的日期
    m_monthView->setDate(m_days);
}

/**
 * @brief setCellSelectable
 * @param selectable
 */
void CYearView::setCellSelectable(bool selectable)
{
    if (selectable == m_cellSelectable)
        return;
    m_cellSelectable = selectable;
}

/**
 * @brief getDateIndex 根据日期找到在一个月中的位置索引
 * @param date 比较的日期
 * @return 索引
 */
int CYearView::getDateIndex(const QDate &date) const
{
    //通过日期获取点击位置的索引
    for (int i = 0; i != DDEYearCalendar::RectSizeofEveyMonth; ++i)
        if (m_days[i] == date)
            return i;

    return 0;
}

/**
 * @brief eventFilter 过滤器，双击年视图下的月份跳转到月视图。
 * @param o 事件对象
 * @param e 事件类型
 * @return false
 */
bool CYearView::eventFilter(QObject *o, QEvent *e)
{
    QWidget *cell = qobject_cast<QWidget *>(o);
    //如果选择的是月份
    if (cell == m_currentMouth) {
        //如果是鼠标双击
        if (e->type() == QEvent::MouseButtonDblClick) {
            //发送选择月份的信号
            emit signalselectMonth(m_currentDate);
        }
    }
    return false;
}

/**
 * @brief updateDate 更新日期
 */
void CYearView::updateDate()
{
    const QDate firstDay(m_currentDate.year(), m_currentDate.month(), 1);
    int offset = firstDay.dayOfWeek() % 7 - m_firstWeekDay ;
    const int day = offset <0 ?offset +7:offset;
    const int currentIndex = day + m_currentDate.day() - 1;

    if (currentIndex < 0) {
        return;
    }
    for (int i(0); i != DDEYearCalendar::RectSizeofEveyMonth; ++i) {
        m_days[i] = firstDay.addDays(i - day);
    }
    update();
}

/**
 * @brief createYearSchceduleView 创建日程浮框对象，并连接信号和槽
 * @param parent 父类
 */
void CYearView::createYearSchceduleView(QWidget *parent)
{
    if (m_Scheduleview == nullptr) {
        m_Scheduleview = new CYearSchceduleOutView(parent);
        connect(m_Scheduleview, &CYearSchceduleOutView::signalsViewSelectDate, this, &CYearView::signalselectWeekwindow);
        connect(m_Scheduleview, &CYearSchceduleOutView::signalupdateschcedule, this, &CYearView::signalupdateschcedule);
    }
}

/**
 * @brief resizeEvent 更新月份框的高度
 * @param event 窗口大小调整事件
 */
void CYearView::resizeEvent(QResizeEvent *event)
{
    int leftmagin = qFloor(width() * 0.06435 + 0.5);
    int rightmagin = leftmagin;
    int topmagin = qFloor(height() * 0.02955 + 0.5);
    int buttonmagin = qFloor(height() * 0.044 + 0.5);

    m_hhLayout->setContentsMargins(leftmagin, topmagin, rightmagin, buttonmagin);
    m_momthFont.setPixelSize(qFloor(16 + (height() - 159) / 16.75));
    m_currentMouth->setTextFont(m_momthFont);
    m_currentMouth->setFixedHeight(24 + (height() - 159) / 12);
    m_currentMouth->update();

    QFrame::resizeEvent(event);
}

/**
 * @brief mousePressEvent 单击鼠标发出隐藏日程浮框信号
 * @param event 鼠标事件
 */
void CYearView::mousePressEvent(QMouseEvent *event)
{
    if (m_selectFlag) return;

    if (event->button() == Qt::LeftButton) {
        emit signalHideInfo();
    }
    CustomFrame::mousePressEvent(event);
}

/**
 * @brief paintEvent 绘制每个月的背景
 * @param e 绘图事件
 */
void CYearView::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    int labelwidth = width() - 2 * m_borderframew;
    int labelheight = height() - 2 * m_borderframew;

    QPainter painter(this);
    if (1) {
        painter.save();
        // 反锯齿;
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(m_bnormalColor));
        painter.setPen(Qt::NoPen);
        QPainterPath painterPath;
        //oainterPath画圆角路径
        painterPath.moveTo(m_radius, m_borderframew);
        painterPath.arcTo(QRect(m_borderframew, m_borderframew, m_radius * 2, m_radius * 2), 90, 90);

        painterPath.lineTo(0, labelheight - m_radius);
        painterPath.arcTo(QRect(m_borderframew, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 180, 90);

        painterPath.lineTo(labelwidth - m_radius, labelheight);
        painterPath.arcTo(QRect(labelwidth - m_radius * 2, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 270, 90);

        painterPath.lineTo(labelwidth, m_radius);

        painterPath.arcTo(QRect(labelwidth - m_radius * 2, m_borderframew, m_radius * 2, m_radius * 2), 0, 90);

        painterPath.lineTo(m_radius, m_borderframew);
        painterPath.closeSubpath();
        painter.drawPath(painterPath);
        painter.restore();
    }
}

/**
 * @brief CYearMonthView 构造函数
 * @param parent 父类
 */
CYearMonthView::CYearMonthView(DWidget *parent)
    :DWidget (parent)
{
    //一个月添加42个rect，代表一个月的日期
    for (int i = 0; i < DDEYearCalendar::RectSizeofEveyMonth; ++i) {
        CMonthDayRect *item = new CMonthDayRect();
        m_DayItem.append(item);
    }
    setMouseTracking(true);
    CMonthDayRect::setDevicePixelRatio(devicePixelRatioF());
}

/**
  *@brief ~CYearMonthView 析构函数
  */
CYearMonthView::~CYearMonthView()
{
    for (int i = 0; i <m_DayItem.size(); ++i) {
        delete m_DayItem[i];
    }
    m_DayItem.clear();
}

/**
 * @brief setDate 设置每个月的日期
 * @param date 日期
 */
void CYearMonthView::setDate(const QDate date[42])
{
    if (date[0].day() !=1) {
        //如果第一个日期不是1号，那就是上个月的日期，这个月的月份就要+1
        m_currentMonth = date[0].addMonths(1).month();
    } else {
        //如果第一个日期正好是1号，那就是本月
        m_currentMonth = date[0].month();
    }
    //设置每天的日期
    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->setDate(date[i]);
        m_DayItem.at(i)->setIsCurrentMonth(date[i].month() == m_currentMonth);
    }
}

/**
 * @brief setTheMe 根据系统主题设置颜色
 * @param type 系统主题
 */
void CYearMonthView::setTheMe(int type)
{
    CMonthDayRect::setTheMe(type);
}

/**
 * @brief setLintFlag 设置是否有日程的标志
 * @param lineFlag 是否有日程的标志
 */
void CYearMonthView::setLintFlag(const QVector<bool> &lineFlag)
{
    m_vlineflag = lineFlag;
    //设置每天的节假日和日程标志
    if (m_vlineflag.size() == DDEYearCalendar::RectSizeofEveyMonth) {
        for (int i = 0; i < DDEYearCalendar::RectSizeofEveyMonth; i++) {
            m_DayItem.at(i)->setLineFlag(m_vlineflag.at(i));
        }
    }
}

/**
 * @brief updateSize 设置每天的日期所在矩形框的大小
 */
void CYearMonthView::updateSize()
{
    qreal w = width()/7;
    qreal h = height()/6;
    QRectF rect ;
    int w_offset = 0;
    int h_offset = 0;

    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        h_offset = i / 7;
        w_offset = i % 7;
        rect.setRect(w*w_offset,
                     h*h_offset,
                     w,
                     h);
        m_DayItem.at(i)->setRect(rect);
    }
    update();
}

/**
 * @brief getMousePosItem 获取点击日期的索引
 * @param pos 点击的点
 * @return 索引值
 */
int CYearMonthView::getMousePosItem(const QPointF &pos)
{
    int res =-1;

    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        if (m_DayItem.at(i)->rect().contains(pos)) {
            res = i;
            break;
        }
    }
    return res;
}

/**
 * @brief resizeEvent 设置每天的日期所在矩形框的大小
 * @param event 窗口大小调整函数
 */
void CYearMonthView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateSize();
}

/**
 * @brief mousePressEvent 鼠标单击事件，单击日期显示当天的日程和节日。
 * @param event 鼠标事件
 */
void CYearMonthView::mousePressEvent(QMouseEvent *event)
{
    int itemindex = getMousePosItem(event->pos());

    if (!(itemindex<0)) {
        //设置日期的点击状态
        m_DayItem.at(itemindex)->setCellEvent(CMonthDayRect::CellPress);
        m_press = true;
        if ( event->button() ==Qt::LeftButton) {
            m_pressIndex = itemindex;
            emit signalPressDate(m_DayItem.at(itemindex)->getDate());
        }
    }
    update();
}

/**
 * @brief mouseDoubleClickEvent  鼠标双击事件，双击跳转到上次打开的视图
 * @param event 鼠标事件
 */
void CYearMonthView::mouseDoubleClickEvent(QMouseEvent *event)
{
    int itemindex = getMousePosItem(event->pos());

    if (!(itemindex<0)) {
        if (m_pressIndex == itemindex) {
            //设置日期的点击状态
            m_DayItem.at(itemindex)->setCellEvent(CMonthDayRect::CellPress);
            m_press = true;
            if ( event->button() ==Qt::LeftButton) {
                emit signalDoubleClickDate(m_DayItem.at(itemindex)->getDate());
            }
        }
    }
}

/**
 * @brief mouseReleaseEvent 鼠标释放事件
 * @param event 鼠标事件
 */
void CYearMonthView::mouseReleaseEvent(QMouseEvent *event)
{
    m_press = false;
    mouseMoveEvent(event);
}

/**
 * @brief mouseMoveEvent 鼠标移动事件，设置hover状态
 * @param event 鼠标事件
 */
void CYearMonthView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_press) {
        int itemindex = getMousePosItem(event->pos());
        if (!(itemindex<0)) {
            //设置日期的hover状态
            m_DayItem.at(itemindex)->setCellEvent(CMonthDayRect::Cellhover);
        }
        update();
    }
}

/**
 * @brief paintEvent 绘制日期以及当天状态
 * @param event 绘图事件
 */
void CYearMonthView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing |QPainter::SmoothPixmapTransform);
    //绘制每天的日期
    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        m_DayItem[i]->paintItem(&painter,m_DayItem[i]->rect());
    }
    painter.end();
}

/**
 * @brief leaveEvent 离开事件，设置当前选中的日期为空
 * @param event QT事件
 */
void CYearMonthView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    CMonthDayRect::setCurrentRect(nullptr);
    update();
}


/**
 * @brief CMonthDayRect 构造函数
 */
CMonthDayRect::CMonthDayRect()
    :m_rect(0,0,0,0)
{
    //设置字体大小
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeTwelve);
    m_hightFont.setPixelSize(DDECalendar::FontSizeTwelve);
}

/**
 * @brief setTheMe 根据系统主题类型设置颜色
 * @param type 系统主题类型
 */
void CMonthDayRect::setTheMe(int type)
{
    if (type ==2) {
        //深色主题
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
        //浅色主题或跟随系统
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

/**
  * @brief ~CMonthDayRect 析构函数
  */
CMonthDayRect::~CMonthDayRect()
{

}

/**
 * @brief setDate 设置某一天的时间
 * @param date 时间
 */
void CMonthDayRect::setDate(const QDate &date)
{
    m_date = date;
}

/**
 * @brief getDate 获取某一天的时间
 * @return 时间
 */
QDate CMonthDayRect::getDate() const
{
    return  m_date;
}

/**
 * @brief setCellEvent 设置某一天的状态，CellEventType
 * @param type CellEventType类型
 */
void CMonthDayRect::setCellEvent(const CMonthDayRect::CellEventType &type)
{
    //设置当天的状态
    m_cellEventType = type;
    m_CurrentRect = this;
}

/**
 * @brief setIsCurrentMonth 设置是否是当前月
 * @param isCurrMonth 是否是当前月
 */
void CMonthDayRect::setIsCurrentMonth(const bool isCurrMonth)
{
    //是否是当前的月
    m_isCurrentMonth = isCurrMonth;
}

/**
 * @brief rect
 * @return 当天日期所在的矩形
 */
QRectF CMonthDayRect::rect() const
{
    return  m_rect;
}

/**
 * @brief setRect 设置当天日期的矩形
 * @param rect 矩形
 */
void CMonthDayRect::setRect(const QRectF &rect)
{
    m_rect = rect;
}

/**
 * @brief setRect 设置当天日期的矩形
 * @param x 设置矩形左上角的x坐标
 * @param y 设置矩形左上角的y坐标
 * @param w 设置矩形的宽度
 * @param h 设置矩形的高度
 */
void CMonthDayRect::setRect(qreal x, qreal y, qreal w, qreal h)
{
    m_rect.setRect(x,y,w,h);
}

/**
 * @brief paintItem 绘制年视图一天的所有状态并展示出来。
 * @param painter QPainter绘画
 * @param rect 年视图中每一天所在的矩形
 */
void CMonthDayRect::paintItem(QPainter *painter, const QRectF &rect)
{
    //高亮的颜色
    m_highColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    //如果日期小于1900,return
    if (m_date.year() < DDECalendar::QueryEarliestYear)
        return;
    //判断是否是今天
    const bool isCurrentDay = m_date == QDate::currentDate() && m_isCurrentMonth;
    bool isSelectedCell  = false;
    if (isCurrentDay) {
        isSelectedCell = true;
    }

    const qreal r = rect.width() > rect.height() ? rect.height() * 0.9 : rect.width() * 0.9 ;
    int fontsize = qRound(DDECalendar::FontSizeTwelve + (r - 18) * 6 / 17);

    if (fontsize < DDECalendar::FontSizeTwelve) {
        //限定font的大小
        fontsize = DDECalendar::FontSizeTwelve;
    }
    //设置font大小
    m_dayNumFont.setPixelSize(fontsize);
    m_hightFont.setPixelSize(fontsize);
    //设置矩形左上角的位置
    const qreal x = rect.x()+(rect.width() - r) / 2;
    const qreal y = rect.y()+(rect.height() - r) / 2;
    //日期所在矩形
    QRectF fillRect = QRectF(x, y, r, r);
    QColor m_cellBackgroundColor;

    if (m_CurrentRect !=this) {
        m_cellEventType = CellNormal;
    }
    if (m_cellEventType == CellPress) {
        //设置日期点击状态颜色
        m_cellBackgroundColor = m_currentColor.pressColor;
    } else if (m_cellEventType == Cellhover) {
        //设置日期hover状态颜色
        m_cellBackgroundColor = m_currentColor.hoverColor;
    } else {
        //设置日期一般状态颜色
        m_cellBackgroundColor = m_currentColor.normalColor;
    }
    //设置画刷颜色
    painter->setBrush(m_cellBackgroundColor);
    painter->setPen(Qt::NoPen);
    //绘制日期背景
    painter->drawEllipse(fillRect);

    bool highflag = false;

    if (m_isCurrentMonth) {
        //获取高亮标志
        highflag = CScheduleDataManage::getScheduleDataManage()->getSearchResult(m_date);
    }
    if (highflag) {
        painter->setBrush(QBrush(m_highColor));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(fillRect);
        painter->setPen(Qt::SolidLine);
        painter->setPen(m_highTextColor);
        painter->setFont(m_hightFont);
        //绘制每天的日期
        painter->drawText(fillRect, Qt::AlignCenter, QString::number(m_date.day()));
    } else {
        // draw selected cell background circle
        if (isSelectedCell) {
            //如果选择了该日期，则设置高亮
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
        //绘制每天的日期
        painter->drawText(fillRect, Qt::AlignCenter, QString::number(m_date.day()));

        if (m_vlineflag) {
            //如果当天有节假日或日程
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
            qreal r = rect.width() * (4 / 25);
            if (r < 4) {
                r = 4;
            } else if ( r > 7) {
                r = 7;
            }
            //绘制节假日或日程标志
            painter->drawEllipse(QRectF(rect.width() - r+rect.x(),
                                        rect.y(),
                                        r,
                                        r));
            painter->restore();
        }
    }
}

/**
 * @brief setLineFlag 设置是否有日程的标志
 * @param flag 标志
 */
void CMonthDayRect::setLineFlag(const bool flag)
{
    m_vlineflag = flag;
}

/**
 * @brief setDevicePixelRatio
 * @param pixel
 */
void CMonthDayRect::setDevicePixelRatio(const qreal pixel)
{
    m_DevicePixelRatio = pixel;
}

/**
 * @brief setCurrentRect 设置当前天
 * @param currrect 当前天
 */
void CMonthDayRect::setCurrentRect(CMonthDayRect *currrect)
{
    m_CurrentRect = currrect;
}

