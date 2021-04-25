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

#include "weekheadview.h"
#include "calendardbus.h"
#include "customframe.h"
#include "scheduledatamanage.h"

#include <DPalette>
#include <DHiDPIHelper>

#include <QHBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QQueue>
#include <QMenu>
#include <QMouseEvent>
#include <QPainterPath>

DGUI_USE_NAMESPACE
/**
 * @brief CWeekHeadView 构造函数
 * @param parent 父类
 */
CWeekHeadView::CWeekHeadView(QWidget *parent)
    : DFrame(parent)
{
    //设置边距
    setContentsMargins(0, 0, 0, 0);
    m_DBusInter = new CalendarDBus("com.deepin.api.LunarCalendar",
                                   "/com/deepin/api/LunarCalendar",
                                   QDBusConnection::sessionBus(), this);
    if (!queue)
        queue = new QQueue<int>;
    //新建字典
    if (!lunarCache)
        lunarCache = new QMap<QDate, CaLunarDayInfo>;
    //新建阴历信息
    if (!emptyCaLunarDayInfo)
        emptyCaLunarDayInfo = new CaLunarDayInfo;
    //设置周字体权重
    m_dayNumFont.setWeight(QFont::Medium);
    //设置周字体大小
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeSixteen);
    //设置月字体权重
    m_monthFont.setWeight(QFont::Medium);
    //设置月字体大小
    m_monthFont.setPixelSize(DDECalendar::FontSizeTwenty);
    // cells grid
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->setMargin(0);
    hboxLayout->setSpacing(0);
    //月label
    m_monthLabel = new CustomFrame(this);
    //设置字体大小
    m_monthLabel->setFixedWidth(DDEWeekCalendar::WMCellHeadrWidth - 5);
    //设置左上角为圆角
    m_monthLabel->setRoundState(true, false, false, false);
    QFont mlabelF;
    mlabelF.setWeight(QFont::Medium);
    mlabelF.setPixelSize(DDECalendar::FontSizeTwenty);
    m_monthLabel->setTextFont(mlabelF);
    //添加到布局
    hboxLayout->addWidget(m_monthLabel);
    hboxLayout->setStretch(0, 0);
    hboxLayout->setSpacing(0);
    //7个widget
    for (int c = 0; c != DDEWeekCalendar::AFewDaysofWeek; ++c) {
        QWidget *cell = new QWidget(this);
        //设置过滤器
        cell->installEventFilter(this);
        cell->setFocusPolicy(Qt::ClickFocus);
        //将cell添加到布局中
        hboxLayout->addWidget(cell);
        //添加到list中
        m_cellList.append(cell);
    }
    //设置布局
    setLayout(hboxLayout);
    //设置连接
    connect(this, &CWeekHeadView::dateSelected, this, &CWeekHeadView::handleCurrentDateChanged);
    setFrameRounded(false);

    m_monthList.append( "一月" );
    m_monthList.append( "二月");
    m_monthList.append( "三月" );
    m_monthList.append( "四月" );
    m_monthList.append( "五月" );
    m_monthList.append( "六月" );
    m_monthList.append( "七月" );
    m_monthList.append( "八月");
    m_monthList.append( "九月" );
    m_monthList.append( "十月" );
    m_monthList.append( "十一月");
    m_monthList.append( "十二月");
}
/**
  * @brief CWeekHeadView 析构函数
  */
CWeekHeadView::~CWeekHeadView()
{
    if (lunarCache !=nullptr) {
        delete lunarCache;
        lunarCache = nullptr;
    }
    if (emptyCaLunarDayInfo != nullptr) {
        delete  emptyCaLunarDayInfo;
        emptyCaLunarDayInfo = nullptr;
    }
    if (queue != nullptr) {
        delete queue;
        queue = nullptr;
    }
}
/**
 * @brief handleCurrentDateChanged 当前日期改变的信号
 * @param date 日期
 * @param detail unused
 */
void CWeekHeadView::handleCurrentDateChanged(const QDate date, const CaLunarDayInfo &detail)
{
    Q_UNUSED(detail);
    //设置日期
    if (date != m_currentDate) {
        setCurrentDate(date);
    }
}
/**
 * @brief setFirstWeekday 设置每周的第一天是周几
 * @param weekday 周几
 */
void CWeekHeadView::setFirstWeekday(int weekday)
{
    m_firstWeekDay = weekday;
    //更新数据
    updateDate();
}
/**
 * @brief getDateType 获取所给日期的类型CalendarWeekDayType
 * @param date 日期
 * @return 类型
 */
int CWeekHeadView::getDateType(const QDate &date)
{
    //获取日期
    const int currentIndex = getDateIndex(date);
    //获取阴历信息
    const CaLunarDayInfo info = getCaLunarDayInfo(currentIndex);
    //周几
    const int dayOfWeek = date.dayOfWeek();
    //是否是周末
    bool weekends = dayOfWeek == DDEWeekCalendar::FirstDayofWeekend || dayOfWeek == DDEWeekCalendar::AFewDaysofWeek;
    //是否是当前月
    bool isCurrentMonth = m_currentDate.month() == date.month();
    //是否是节假日
    bool isFestival = !info.mSolarFestival.isEmpty() || !info.mLunarFestival.isEmpty();
    //结果
    int resultFlag = 0;
    //不是当前月
    if (!isCurrentMonth)
        resultFlag |= SO_MNotCurrentMonth;
    //节假日
    if (isFestival)
        resultFlag |= SO_MFestival;
    //周末
    if (weekends)
        resultFlag |= SO_MWeekends;
    //结果
    return resultFlag;
}
/**
 * @brief setTheMe 根据系统主题类型设置颜色
 * @param type 系统主题类型
 */
void CWeekHeadView::setTheMe(int type)
{
    m_themetype = type;
    //根据系统类型设置颜色
    if (type == 0 || type == 1) {
        //设置月份文字颜色
        QColor textC = "#000000";
        //设置月份背景色
        QColor textbC(230, 238, 242);
        //设置月份label
        m_monthLabel->setBColor(textbC);
        m_monthLabel->setTextColor(textC);

        m_backgroundCircleColor = "#0081FF";
        m_backgroundShowColor = "#2CA7F8";
        m_backgroundShowColor.setAlphaF(0.4);
        m_Background_Weekend_Color = "#DAE4ED";
        //默认字体颜色
        m_defaultTextColor = "#6F6F6F";
        //今天字体颜色
        m_currentDayTextColor = "#FFFFFF";
        //默认阴历颜色
        m_defaultLunarColor = "#898989";
        //周末文字颜色，系统活动色
        m_weekendsTextColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        //月份颜色
        m_currentMonthColor = "#000000";
        //背景色
        m_backgroudColor = "#E6EEF2";
        //默认阴历节日颜色
        m_solofestivalLunarColor = "#4DFF7272";
    } else if (type == 2) {
        //系统主题类型为神色
        QColor textbC = "#82AEC1";
        textbC.setAlphaF(0.1);
        m_monthLabel->setBColor(textbC);
        m_monthLabel->setTextColor("#BF1D63");
        m_backgroundCircleColor = "#0059D2";
        m_backgroundShowColor = "#002AAF";
        m_backgroundShowColor.setAlphaF(0.4);
        m_Background_Weekend_Color = "#333D4A";

        m_defaultTextColor = "#C0C6D4";
        m_currentDayTextColor = "#C0C6D4";
        m_defaultLunarColor = "#6886BA";
        m_weekendsTextColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        m_currentMonthColor = "#000000";
        m_backgroudColor = "#82AEC1";
        m_backgroudColor.setAlphaF(0.1);
        m_solofestivalLunarColor = "#4DFF7272";
    }
    //更新一周的数据
    for (int i = 0; i != DDEWeekCalendar::AFewDaysofWeek; ++i)
        m_cellList.at(i)->update();
}
/**
 * @brief setWeekDay 设置一周的时间
 * @param vDays 一周代表的时间
 */
void CWeekHeadView::setWeekDay(QVector<QDate> vDays)
{
    if (vDays.count() != DDEWeekCalendar::AFewDaysofWeek)
        return;
    //设置一周的时间
    for (int i = 0; i != DDEWeekCalendar::AFewDaysofWeek; ++i)
        m_days[i] = vDays[i];
}
/**
 * @brief setMounthLabelWidth  设置
 * @param w
 * @param rw
 */
void CWeekHeadView::setMounthLabelWidth(int w, int rw)
{
    m_monthW = w + 1;
    m_fixwidth = rw;
}
/**
 * @brief setCurrentDate 设置当前的时间
 * @param date 日期
 */
void CWeekHeadView::setCurrentDate(const QDate date)
{
    qDebug() << "set current date " << date;
    //date是当前时间，return
    if (date == m_currentDate) {
        return;
    }
    //设置当前日期
    m_currentDate = date;
    //发射当前时间改变的信号
    emit signalcurrentDateChanged(m_currentDate);
    // to refresh lunar calendar
    updateCurrentLunar(getCaLunarDayInfo(getDateIndex(m_currentDate)));
    QLocale locale;
    //设置月份
    m_monthLabel->setTextStr(locale.monthName(date.month(), QLocale::ShortFormat));
}
/**
 * @brief setLunarVisible 设置是否显示阴历信息
 * @param visible 是否显示阴历信息
 */
void CWeekHeadView::setLunarVisible(bool visible)
{
    int state = int(m_showState);
    //是否显示阴历信息
    if (visible)
        state |= ShowLunar;
    else
        state &= ~ShowLunar;
    //显示状态
    m_showState = ShowState(state);
    update();
}
/**
 * @brief setCellSelectable
 * @param selectable
 */
void CWeekHeadView::setCellSelectable(bool selectable)
{
    if (selectable == m_cellSelectable)
        return;

    m_cellSelectable = selectable;

    emit cellSelectableChanged(m_cellSelectable);
}
/**
 * @brief getDateIndex 根据日期返回当天的索引
 * @param date 日期
 * @return 日期所在的索引
 */
int CWeekHeadView::getDateIndex(const QDate &date) const
{
    //返回date日期所在索引
    for (int i = 0; i != DDEWeekCalendar::AFewDaysofWeek; ++i)
        if (m_days[i] == date)
            return i;
    //没有返回0
    return 0;
}
/**
 * @brief eventFilter 过滤器
 * @param o 事件对象
 * @param e 时间类型
 * @return false
 */
bool CWeekHeadView::eventFilter(QObject *o, QEvent *e)
{
    QWidget *cell = qobject_cast<QWidget *>(o);
    //过滤事件是cell并且包含在list中
    if (cell && m_cellList.contains(cell)) {
        if (e->type() == QEvent::Paint) {
            //绘图事件
            paintCell(cell);
        } else if (e->type() == QEvent::MouseButtonPress) {
        } else if (e->type() == QEvent::MouseButtonDblClick) {
            //鼠标双击事件
            const int pos = m_cellList.indexOf(cell);
            emit signalsViewSelectDate(m_days[pos]);
        }
    }
    return false;
}
/**
 * @brief updateDate 更新数据
 */
void CWeekHeadView::updateDate()
{
    const int currentIndex = getDateIndex(m_currentDate);
    //设置选择的cell
    setSelectedCell(currentIndex);
    update();
}
/**
 * @brief updateCurrentLunar 更新当前的阴历信息
 * @param info 阴历信息
 */
void CWeekHeadView::updateCurrentLunar(const CaLunarDayInfo &info)
{
    if (!info.mLunarFestival.isEmpty()) {
        emit currentFestivalChanged(info.mLunarFestival);
    } else if (!info.mTerm.isEmpty()) {
        emit currentFestivalChanged(info.mTerm);
    } else if (!info.mSolarFestival.isEmpty()) {
        QStringList tmpFestival = info.mSolarFestival.split(" ");

        if (tmpFestival.length() >= 3) {
            emit currentFestivalChanged(QString("%1 %2").arg(tmpFestival[0]).arg(tmpFestival[1]));
        } else {
            emit currentFestivalChanged(info.mSolarFestival);
        }
    } else {
        emit currentFestivalChanged("");
    }

    QVector<QDate> vdate;
    QVector<CaLunarDayInfo> vdetail;

    for (int i = 0; i < DDEWeekCalendar::AFewDaysofWeek; i++) {
        vdate.append(m_days[i]);
        vdetail.append(getCaLunarDayInfo(getDateIndex(m_days[i])));
    }

    emit signalcurrentLunarDateChanged(vdate, vdetail, 1);
    updateDate();
}
/**
 * @brief getCellDayNum 根据索引值获取当天是在一个月中的第几天
 * @param pos 索引
 * @return 日期的字符串
 */
const QString CWeekHeadView::getCellDayNum(int pos)
{
    return QString::number(m_days[pos].day());
}
/**
 * @brief getCellDate 根据索引获取当天的日期
 * @param pos 索引值
 * @return 日期
 */
const QDate CWeekHeadView::getCellDate(int pos)
{
    return m_days[pos];
}
/**
 * @brief getLunar 根据索引值获取当天的阴历信息
 * @param pos 索引值
 * @return 阴历信息字符串
 */
const QString CWeekHeadView::getLunar(int pos)
{
    CaLunarDayInfo info = getCaLunarDayInfo(pos);
    //若是初一，设置月份
    if (info.mLunarDayName == "初一") {
        info.mLunarDayName =  info.mLunarMonthName;
    }

    if (info.mTerm.isEmpty())
        return info.mLunarDayName;

    return info.mTerm;
}
/**
 * @brief getCaLunarDayInfo 根据索引值获取阴历信息
 * @param pos 索引值
 * @return 阴历信息
 */
const CaLunarDayInfo CWeekHeadView::getCaLunarDayInfo(int pos)
{
    const QDate date = m_days[pos];
    //map中包含date，返回date对应的阴历信息
    if (lunarCache->contains(date)) {
        return lunarCache->value(date);
    }

    if (lunarCache->size() > 300)
        lunarCache->clear();

    queue->push_back(pos);

    QTimer::singleShot(300, this, SLOT(getDbusData()));

    return *emptyCaLunarDayInfo;
}
/**
 * @brief getDbusData 获取dbus日期
 */
void CWeekHeadView::getDbusData()
{
    //queue为空，return
    if (queue->isEmpty())
        return;
    //queue头
    const int pos = queue->head();
    queue->pop_front();
    const QDate date = m_days[pos];
    //日期不合法
    if (!date.isValid()) {
        return;
    }
    //阴历信息
    CaLunarDayInfo currentDayInfo;
    //map中不包含date，添加
    if (!lunarCache->contains(date)) {
        bool o1 = true;
        //获取阴历信息
        QDBusReply<CaLunarMonthInfo> reply = m_DBusInter->GetLunarMonthCalendar(date.year(), date.month(), false, o1);
        QDate cacheDate;
        //设置时间
        cacheDate.setDate(date.year(), date.month(), 1);
        foreach (const CaLunarDayInfo &dayInfo, reply.value().mCaLunarDayInfo) {
            lunarCache->insert(cacheDate, dayInfo);

            if (date == m_currentDate) {
                currentDayInfo = dayInfo;
            }
            cacheDate = cacheDate.addDays(1);
        }
    } else {
        //date的阴历信息
        currentDayInfo = lunarCache->value(date);
    }
    //更新数据
    m_cellList.at(pos)->update();
    // refresh   lunar info
    if (date == m_currentDate) {
        updateCurrentLunar(currentDayInfo);
    }
}
/**
 * @brief paintCell 绘制周信息
 * @param cell 每天所在的widget
 */
void CWeekHeadView::paintCell(QWidget *cell)
{
    //系统活动色
    m_weekendsTextColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    //cell矩形
    const QRect rect(0, 0, cell->width(), cell->height());
    //cell所在索引
    const int pos = m_cellList.indexOf(cell);
    //是否是今天
    const bool isCurrentDay = getCellDate(pos) == QDate::currentDate();
    //是否是选择的cell
    const bool isSelectedCell = isCurrentDay;
    //周几
    int d = m_days[pos].dayOfWeek();
    //painter cell
    QPainter painter(cell);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    //根据周几设置不一样的背景色
    if (d == DDEWeekCalendar::FirstDayofWeekend || d == DDEWeekCalendar::AFewDaysofWeek) {
        painter.setBrush(QBrush(m_Background_Weekend_Color));
    } else {
        painter.setBrush(QBrush(m_backgroudColor));
    }
    if (d != 6) {
        //画矩形
        painter.drawRect(rect);
    } else {
        //绘制带圆角的矩形
        int labelwidth = cell->width();
        int labelheight = cell->height();
        //绘制路径
        QPainterPath painterPath;
        painterPath.moveTo(m_radius, 0);

        painterPath.lineTo(0, 0);
        painterPath.lineTo(0, m_radius);
        painterPath.lineTo(0, labelheight - m_radius);
        painterPath.lineTo(0, labelheight);
        painterPath.lineTo(m_radius, labelheight);
        painterPath.lineTo(labelwidth - m_radius, labelheight);
        painterPath.lineTo(labelwidth, labelheight);
        painterPath.lineTo(labelwidth, labelheight - m_radius);
        painterPath.lineTo(labelwidth, m_radius);
        //绘制右上角圆角
        painterPath.arcTo(QRect(labelwidth - m_radius * 2, 0, m_radius * 2, m_radius * 2), 0, 90);
        painterPath.lineTo(m_radius, 0);
        painterPath.closeSubpath();
        painter.drawPath(painterPath);
    }
    //绘制分割线
    if (d != DDEWeekCalendar::FirstDayofWeekend && d != DDEWeekCalendar::AFewDaysofWeek) {
        QPoint point_begin(cell->width(), 0);
        QPoint point_end(cell->width(), cell->height());
        painter.save();
        QColor m_linecolor = "#000000";
        m_linecolor.setAlphaF(0.1);
        painter.setPen(m_linecolor);
        //绘制直线
        painter.drawLine(point_begin, point_end);
        painter.restore();
    }

    int bw = (cell->width() - 104) / 2;
    int bh = (cell->height() - 26) / 2;
    //设置矩形左上角点的最小值
    if (bw < 0) {
        bw = 2;
    }
    if (bh < 0) {
        bh = 2;
    }
    if (isSelectedCell) {
        //选择的cell
        if (m_showState & ShowLunar) {
            //绘制有阴历信息的当天背景圆
            QRect fillRect(bw - 2, bh, 26, 26);
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            //设置颜色为系统活动色
            painter.setBrush(QBrush(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor()));
            painter.setPen(Qt::NoPen);
            //绘制圆
            painter.drawEllipse(fillRect);
            painter.restore();
        } else {
            //绘制没有阴历信息的当天背景圆
            QRect fillRect(cell->width() - (cell->width()/2) + 1, bh - 1, 26, 26);
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            //设置颜色为系统活动色
            painter.setBrush(QBrush(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor()));
            painter.setPen(Qt::NoPen);
            //绘制圆
            painter.drawEllipse(fillRect);
            painter.restore();
        }
    }
    QLocale locale;
    //每天的数字
    const QString dayNum = getCellDayNum(pos);
    //每天的阴历
    const QString dayLunar = getLunar(pos);
    //在一周中是第几天
    QString dayWeek = locale.dayName(d ? d : DDEWeekCalendar::AFewDaysofWeek, QLocale::ShortFormat);

    painter.save();
    painter.setPen(Qt::SolidLine);
    // draw text of day
    if (isSelectedCell) {
        //设置今天字体颜色
        painter.setPen(m_currentDayTextColor);
    } else if (isCurrentDay) {
        //设置周末字体颜色
        painter.setPen(m_weekendsTextColor);
    } else {
        if (d == DDEWeekCalendar::FirstDayofWeekend || d == DDEWeekCalendar::AFewDaysofWeek)
            //设置周末字体颜色
            painter.setPen(m_weekendsTextColor);
        else
            //设置一般字体颜色
            painter.setPen(m_defaultTextColor);
    }
    //设置字体
    painter.setFont(m_dayNumFont);
    //根据是否显示阴历，在不同位置绘制日期和周数
    if (m_showState & ShowLunar) {
        painter.drawText(QRect(bw - 1, bh, 24, 24), Qt::AlignCenter, dayNum);
        if (d == DDEWeekCalendar::FirstDayofWeekend || d == DDEWeekCalendar::AFewDaysofWeek)
            painter.setPen(m_weekendsTextColor);
        else
            painter.setPen(m_defaultTextColor);
        painter.drawText(QRect(bw + 24, bh, 30, 25), Qt::AlignCenter, dayWeek);
    } else {
        QFontMetrics fm1 = painter.fontMetrics();
        //绘制day
        painter.drawText(QRect(cell->width() - (cell->width()/2) - 4, bh - 1, 36, 26), Qt::AlignCenter, dayNum);
        //设置不同的颜色
        if (d == DDEWeekCalendar::FirstDayofWeekend || d == DDEWeekCalendar::AFewDaysofWeek)
            painter.setPen(m_weekendsTextColor);
        else
            painter.setPen(m_defaultTextColor);
        //查看字体度量
        QFontMetrics fm = painter.fontMetrics();
        //文字太宽，减少显示的文字
        while (fm.width(dayWeek) > cell->width()/2)
            dayWeek.chop(1);
        //绘制周几
        painter.drawText(QRect(0, bh, (cell->width()/2), 26), Qt::AlignRight, dayWeek);
    }
    // draw text of day type
    if (m_showState & ShowLunar) {
        if (cell->width() > 100) {
            if (d == DDEWeekCalendar::FirstDayofWeekend || d == DDEWeekCalendar::AFewDaysofWeek)
                painter.setPen(m_weekendsTextColor);
            else
                painter.setPen(m_defaultLunarColor);
            //根据cell宽度绘制阴历信息
            if (cell->width() < 132) {
                //cell宽度较小
                QString str_dayLunar = nullptr;
                //阴历信息显示两个字，多余显示...
                if (dayLunar.count() > 2) {
                    for (int i = 0; i < 2; i++) {
                        str_dayLunar.append(dayLunar.at(i));
                    }
                    str_dayLunar.append("...");
                } else {
                    str_dayLunar = dayLunar;
                }
                //绘制文字
                painter.drawText(QRect(bw + 52 + 10, bh, 50, 25), Qt::AlignLeft, str_dayLunar);
            } else {
                //cell宽度较大
                painter.drawText(QRect(bw + 52 + 10, bh, 50, 25), Qt::AlignLeft, dayLunar);
            }
            //获取阴历信息
            CaLunarDayInfo dayInfo = getCaLunarDayInfo(pos);
        }
    }
    painter.restore();
    painter.end();
}
/**
 * @brief cellClicked
 * @param cell
 */
void CWeekHeadView::cellClicked(QWidget *cell)
{
    if (!m_cellSelectable)
        return;
    //cell在list中的位置
    const int pos = m_cellList.indexOf(cell);
    if (pos == -1)
        return;
    //设置选择的cell
    setSelectedCell(pos);
    // my gift eggs
    static int gift = 0;

    if (m_days[pos] == QDate(1993, 7, 28))
        if (++gift == 10)
            QMessageBox::about(this, "LinuxDeepin", "by shibowen <sbw@sbw.so> :P");
}
/**
 * @brief setSelectedCell 设置选择的时间
 * @param index 索引值
 */
void CWeekHeadView::setSelectedCell(int index)
{
    if (m_selectedCell == index)
        return;

    const int prevPos = m_selectedCell;
    m_selectedCell = index;

    m_cellList.at(prevPos)->update();
    m_cellList.at(index)->update();
    emit dateSelected(m_days[index], getCaLunarDayInfo(index));
}
/**
 * @brief checkDay 检查周数，在定制中计算正确的顺序。
 * @param weekday 周数
 * @return 周数
 */
int CWeekHeadView::checkDay(int weekday)
{
    // check the week, calculate the correct order in the custom.
    if (weekday <= 0)
        return weekday += DDEWeekCalendar::AFewDaysofWeek;

    if (weekday > DDEWeekCalendar::AFewDaysofWeek)
        return weekday -= DDEWeekCalendar::AFewDaysofWeek;

    return weekday;
}
/**
 * @brief mousePressEvent 鼠标单击事件
 * @param event 鼠标事件
 */
void CWeekHeadView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    //发送隐藏日程浮框的信号
    emit signaleSchedulHide();
}
