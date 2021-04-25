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
#include "todybutton.h"
#include "scheduledatamanage.h"
#include "daymonthview.h"
#include "constants.h"
#include "dayhuangliview.h"

#include <DPalette>
#include <DHorizontalLine>
#include <DHiDPIHelper>

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QQueue>
#include <QSpacerItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainterPath>

DGUI_USE_NAMESPACE
/**
 * @brief CDayMonthView::CDayMonthView 构造函数
 * @param parent 父类
 */
CDayMonthView::CDayMonthView(QWidget *parent)
    : CustomFrame(parent)
{
    m_DBusInter = new CalendarDBus("com.deepin.api.LunarCalendar",
                                   "/com/deepin/api/LunarCalendar",
                                   QDBusConnection::sessionBus(), this);
    //队列
    if (!queue)
        queue = new QQueue<int>;
    //map
    if (!lunarCache)
        lunarCache = new QMap<QDate, CaHuangLiDayInfo>;
    //黄历信息
    if (!emptyCaHuangLiDayInfo)
        emptyCaHuangLiDayInfo = new CaHuangLiDayInfo;
    //周列表
    m_weeklist.append(tr("Monday"));
    m_weeklist.append(tr("Tuesday"));
    m_weeklist.append(tr("Wednesday"));
    m_weeklist.append(tr("Thursday"));
    m_weeklist.append(tr("Friday"));
    m_weeklist.append(tr("Saturday"));
    m_weeklist.append(tr("Sunday"));
    //初始化界面
    initUI();
    //初始化connect
    initConnection();
}
/**
 * @brief CDayMonthView::~CDayMonthView 析构函数
 */
CDayMonthView::~CDayMonthView()
{
    //删除map，并置空
    if (lunarCache !=nullptr) {
        delete lunarCache;
        lunarCache = nullptr;
    }
    //删除队列并置空
    if ( queue !=nullptr) {
        delete queue;
        queue = nullptr;
    }
    //删除黄历信息并置空
    if (emptyCaHuangLiDayInfo != nullptr) {
        delete emptyCaHuangLiDayInfo;
        emptyCaHuangLiDayInfo = nullptr;
    }
}
/**
 * @brief CDayMonthView::handleCurrentDateChanged 当前日期改变
 * @param date 日期
 * @param detail
 */
void CDayMonthView::handleCurrentDateChanged(const QDate date, const CaHuangLiDayInfo &detail)
{
    Q_UNUSED(detail);
    if (date != m_currentDate) {
        //设置当天时间
        setCurrentDate(date);
    }
}
/**
 * @brief CDayMonthView::setFirstWeekday 设置每周以周几开始
 * @param weekday 周几
 */
void CDayMonthView::setFirstWeekday(int weekday)
{
    m_firstWeekDay = weekday;
    //更新数据
    updateDate();
}
/**
 * @brief CDayMonthView::setLunarVisible 是否显示阴历信息
 * @param visible 使能
 */
void CDayMonthView::setLunarVisible(bool visible)
{
    m_huanglistate = visible;
    m_yiLabel->setVisible(visible);
    m_jiLabel->setVisible(visible);
    m_currentLuna->setVisible(visible);
    m_splitline->setVisible(visible);
    update();
}
/**
 * @brief CDayMonthView::setTheMe 设置系统主题类型
 * @param type 系统主题类型
 */
void CDayMonthView::setTheMe(int type)
{
    m_themetype = type;
    //根据系统主题类型设置颜色
    if (type == 0 || type == 1) {
        //系统主题类型为
        //设置背景色
        DPalette anipa = this->palette();
        QColor tbcolor = "#FFFFFF";
        anipa.setColor(DPalette::Background, tbcolor);
        setPalette(anipa);
        setBackgroundRole(DPalette::Background);
        setBColor(tbcolor);
        //设置返回今天按钮的颜色
        DPalette todaypa = m_today->palette();
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        todaypa.setColor(DPalette::ButtonText, todayColor);
        todaypa.setColor(DPalette::Dark, Qt::white);
        todaypa.setColor(DPalette::Light, Qt::white);
        QColor sbcolor("#002A57");
        sbcolor.setAlphaF(0.05);
        todaypa.setColor(DPalette::Shadow, sbcolor);
        m_today->setPalette(todaypa);
        QColor todayhover = "#000000";
        todayhover.setAlphaF(0.1);
        QColor todaypress = "#000000";
        todaypress.setAlphaF(0.2);
        m_today->setBColor("#FFFFFF", todayhover, todaypress, "#FFFFFF", todayhover, todaypress);
        m_today->setTColor(todayColor, "#001A2E", "#0081FF");
        m_today->setshadowColor(sbcolor);
        //设置上一个月按钮颜色
        DPalette prevpa = m_prevButton->palette();
        prevpa.setColor(DPalette::Dark, QColor("#E6E6E6"));
        prevpa.setColor(DPalette::Light, QColor("#E3E3E3"));
        //设置下一个月按钮颜色
        DPalette nextvpa = m_nextButton->palette();
        nextvpa.setColor(DPalette::Dark, QColor("#E6E6E6"));
        nextvpa.setColor(DPalette::Light, QColor("#E3E3E3"));
        //当前月文字颜色
        m_currentMouth->setTextColor(QColor("#3B3B3B"));
        //当前天文字颜色
        m_currentDay->setTextColor(todayColor);
        //当前周文字颜色
        m_currentWeek->setTextColor(QColor("#414D68"));
        //当前阴历文字颜色
        m_currentLuna->setTextColor(QColor("#414D68"));
        //今年的文字颜色
        m_currentYear->setTextColor(QColor("#414D68"));
        //字体
        QFont hlabelF;
        hlabelF.setPixelSize(DDECalendar::FontSizeFourteen);
        //设置宜颜色
        QColor yicolor = QColor("#75C18E");
        yicolor.setAlphaF(0.1);
        m_yiLabel->setbackgroundColor(yicolor);
        m_yiLabel->setTextInfo(QColor("#7B7B7B"), hlabelF);
        m_yiLabel->update();
        //设置忌颜色
        QColor jicolor = QColor("#C17575");
        jicolor.setAlphaF(0.1);
        m_jiLabel->setbackgroundColor(jicolor);
        m_jiLabel->setTextInfo(QColor("#7B7B7B"), hlabelF);
        m_jiLabel->update();

        m_topBorderColor = Qt::red;
        m_backgroundCircleColor = "#0081FF";
        //默认文字颜色
        m_defaultTextColor = Qt::black;
        //返回今天按钮文字颜色
        m_currentDayTextColor = todayColor;
        //周末文字颜色
        m_weekendsTextColor = Qt::black;
        //选择的文字颜色
        m_selectedTextColor = Qt::white;
        //节假日文字颜色
        m_festivalTextColor = Qt::black;
        //不是当前的文字颜色
        m_notCurrentTextColor = "#b2b2b2";
        m_ceventColor = QColor(255, 93, 0);
    } else if (type == 2) {
        //系统主题类型为神色
        //设置背景色
        DPalette anipa = this->palette();
        QColor tbcolor = "#282828";
        anipa.setColor(DPalette::Background, tbcolor);
        setPalette(anipa);
        setBackgroundRole(DPalette::Background);
        setBColor(tbcolor);
        //设置返回今天按钮颜色
        DPalette todaypa = m_today->palette();
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        todaypa.setColor(DPalette::ButtonText, todayColor);
        todaypa.setColor(DPalette::Light, "#484848");
        todaypa.setColor(DPalette::Dark, "#414141");
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.05);
        todaypa.setColor(DPalette::Shadow, sbcolor);
        m_today->setPalette(todaypa);
        m_today->setBColor("#484848", "#727272", "#242424", "#414141", "#535353", "#282828");
        m_today->setTColor(todayColor, "#FFFFFF", "#0081FF");
        m_today->setshadowColor(sbcolor);
        //设置上一个月按钮颜色
        DPalette prevpa = m_prevButton->palette();
        prevpa.setColor(DPalette::Dark, QColor("#484848"));
        prevpa.setColor(DPalette::Light, QColor("#414141"));
        //设置下一个月颜色
        DPalette nextvpa = m_nextButton->palette();
        nextvpa.setColor(DPalette::Dark, QColor("#484848"));
        nextvpa.setColor(DPalette::Light, QColor("#414141"));
        //当前月份文字颜色
        m_currentMouth->setTextColor(QColor("#C0C6D4"));
        //当前天文字颜色
        m_currentDay->setTextColor(todayColor);
        //当前周文字颜色
        m_currentWeek->setTextColor(QColor("#C0C6D4"));
        //当天阴历文字颜色
        m_currentLuna->setTextColor(QColor("#C0C6D4"));
        //今天文字颜色
        m_currentYear->setTextColor(QColor("#C0C6D4"));
        QFont hlabelF;
        hlabelF.setPixelSize(DDECalendar::FontSizeFourteen);
        //宜颜色
        QColor yicolor = QColor("#2F8C4D");
        yicolor.setAlphaF(0.2);
        m_yiLabel->setbackgroundColor(yicolor);
        m_yiLabel->setTextInfo(QColor("#C0C6D4"), hlabelF);
        m_yiLabel->update();
        //忌颜色
        QColor jicolor = QColor("#A43B3B");
        jicolor.setAlphaF(0.2);
        m_jiLabel->setbackgroundColor(jicolor);
        m_jiLabel->setTextInfo(QColor("#C0C6D4"), hlabelF);
        m_jiLabel->update();

        m_topBorderColor = Qt::red;
        m_backgroundCircleColor = "#0059D2";

        m_defaultTextColor = "#C0C6D4";
        m_currentDayTextColor = todayColor;
        m_weekendsTextColor = Qt::black;
        m_selectedTextColor = "#B8D3FF";
        m_festivalTextColor = Qt::black;
        m_notCurrentTextColor = "#C0C6D4";
        m_notCurrentTextColor.setAlphaF(0.5);
        m_ceventColor = QColor(204, 77, 3);
    }
    //设置每一个cell
    for (int i(0); i != DDEDayCalendar::PainterCellNum; ++i) {
        m_cellList.at(i)->update();
    }
}
/**
 * @brief CDayMonthView::setSearchFlag 设置搜索标志
 * @param flag 搜索标志
 */
void CDayMonthView::setSearchFlag(bool flag)
{
    m_searchflag = flag;
    update();
}
/**
 * @brief CDayMonthView::updateFlag 更新日程标志
 */
void CDayMonthView::updateFlag()
{
    //获取日程标志
    getlineflag();
    update();
}
/**
 * @brief CDayMonthView::setCurrentDate 设置当前天
 * @param date 日期
 * @param type unused
 */
void CDayMonthView::setCurrentDate(const QDate date, int type)
{
    Q_UNUSED(type);
    //小于1900年，return
    if (date.year() < DDECalendar::QueryEarliestYear)
        return;
    //date等于当前时间，return
    if (date == m_currentDate) {
        return;
    }
    //设置返回今天按钮文字
    if (date == QDate::currentDate()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }
    //设置当天
    m_currentDate = date;
    //更新阴历
    updateCurrentLunar();
}
/**
 * @brief CDayMonthView::getDateIndex 获取日期的索引
 * @param date 日期
 * @return 索引
 */
int CDayMonthView::getDateIndex(const QDate &date) const
{
    //遍历所有cell
    for (int i = 0; i != DDEDayCalendar::PainterCellNum; ++i)
        if (m_days[i] == date)
            return i;

    return 0;
}
/**
 * @brief CDayMonthView::eventFilter 过滤器
 * @param o 过滤widget
 * @param e 过滤事件
 * @return false
 */
bool CDayMonthView::eventFilter(QObject *o, QEvent *e)
{
    QWidget *cell = qobject_cast<QWidget *>(o);
    //过滤widget
    if (cell && m_cellList.contains(cell)) {
        const int pos = m_cellList.indexOf(cell);
        QDate date = m_days[pos];
        //最早时间
        if (date.year() < DDECalendar::QueryEarliestYear)
            return false;
        //过滤事件
        if (e->type() == QEvent::Paint) {
            //绘制事件
            paintCell(cell);
        } else if (e->type() == QEvent::MouseButtonPress) {
            //鼠标点击事件
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
            if (mouseEvent->button() ==Qt::LeftButton) {
                //左击cell
                cellClicked(cell);
            }
        }
    }
    return false;
}
/**
 * @brief CDayMonthView::updateDate 更新日期
 */
void CDayMonthView::updateDate()
{
    //这个月的1号
    const QDate firstDay(m_currentDate.year(), m_currentDate.month(), 1);
    //第一天的索引
    const int day = (firstDay.dayOfWeek() + m_firstWeekDay) % 7;
    //今天的索引
    const int currentIndex = day + m_currentDate.day() - 1;
    //如果今天的索引小于0,return
    if (currentIndex < 0) {
        return;
    }
    //设置42天的日期
    for (int i(0); i != DDEDayCalendar::PainterCellNum; ++i) {
        m_days[i] = firstDay.addDays(i - day);
    }
    //设置选择的cell索引
    setSelectedCell(currentIndex);
    //获取日程标志
    getlineflag();
    //更新
    update();
}
/**
 * @brief CDayMonthView::getlineflag 获取日程或节假日标志
 */
void CDayMonthView::getlineflag()
{
    QLocale locale;
    CScheduleDataManage *tdataManage = CScheduleDataManage::getScheduleDataManage();
    //设置size大小
    m_vlineflag.resize(DDEDayCalendar::PainterCellNum);
    //初始化
    m_vlineflag.fill(false);
    QVector<ScheduleDateRangeInfo> out;
    //设置42天的日程标志
    if (tdataManage->getscheduleDataCtrl()->getScheduleInfo(m_days[0], m_days[41], out)) {
        if (out.count() == DDEDayCalendar::PainterCellNum)
            for (int i = 0; i < DDEDayCalendar::PainterCellNum; i++) {
                if (!out.at(i).vData.isEmpty()) {
                    m_vlineflag[i] = true;
                }
            }
    }
}
/**
 * @brief CDayMonthView::initUI 初始化界面
 */
void CDayMonthView::initUI()
{
    //设置daynum字体大小
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeTwelve);
    //返回今天按钮
    m_today = new CTodyButton;
    //设置文字
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    //设置按钮大小
    m_today->setFixedSize(100, DDEDayCalendar::D_MLableHeight);
    //设置返回按钮颜色
    DPalette todaypa = m_today->palette();
    QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    todaypa.setColor(DPalette::ButtonText, todayColor);
    todaypa.setColor(DPalette::Dark, Qt::white);
    todaypa.setColor(DPalette::Light, Qt::white);
    QColor sbcolor("#002A57");
    sbcolor.setAlphaF(0.05);
    todaypa.setColor(DPalette::Shadow, sbcolor);
    QFont todayfont;
    todayfont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_today->setFont(todayfont);
    m_today->setPalette(todaypa);
    //设置前一个月按钮
    m_prevButton = new DIconButton(DStyle::SP_ArrowLeft, this);
    m_prevButton->setFixedSize(36, 36);
    //设置后一个月按钮
    m_nextButton = new DIconButton(DStyle::SP_ArrowRight, this);
    m_nextButton->setFixedSize(36, 36);
    //title布局
    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 3);
    //add separator line
    m_currentMouth = new CustomFrame(this);
    m_currentMouth->setFixedSize(74, DDEDayCalendar::D_MLableHeight);
    QFont mlabelF;
    mlabelF.setWeight(QFont::Medium);
    mlabelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_currentMouth->setTextFont(mlabelF);
    m_currentMouth->setTextAlign(Qt::AlignCenter);
    //将上一个月按钮添加到title布局
    titleLayout->addWidget(m_prevButton);
    //将本月文字添加到title布局
    titleLayout->addWidget(m_currentMouth);
    //将下一个月按钮添加到title布局
    titleLayout->addWidget(m_nextButton);
    //弹簧
    titleLayout->addStretch();
    //将返回今天按钮添加到title布局
    titleLayout->addWidget(m_today, 0, Qt::AlignRight);
    // cells grid
    m_gridLayout = new QGridLayout;
    m_gridLayout->setMargin(0);
    m_gridLayout->setSpacing(0);
    //一个月，42个cell
    for (int r = 0; r != 6; ++r) {
        for (int c = 0; c != 7; ++c) {
            QWidget *cell = new QWidget;
            cell->setFixedSize(DDEDayCalendar::DCellWidth, DDEDayCalendar::DCellHeight);
            cell->installEventFilter(this);
            cell->setFocusPolicy(Qt::ClickFocus);
            //添加到布局
            m_gridLayout->addWidget(cell, r, c);
            //添加到list
            m_cellList.append(cell);
        }
    }
    //上半部分
    m_upLayout = new QVBoxLayout;
    m_upLayout->setMargin(0);
    m_upLayout->setSpacing(0);
    m_upLayout->setContentsMargins(22, 9, 0, 7);
    //添加title布局
    m_upLayout->addLayout(titleLayout);
    //添加cell grid
    m_upLayout->addLayout(m_gridLayout);
    //中间部分
    QVBoxLayout *midLayout = new QVBoxLayout;
    midLayout->setMargin(0);
    midLayout->setSpacing(0);
    midLayout->setContentsMargins(0, 0, 0, 20);
    //今天
    m_currentDay = new CustomFrame(this);
    m_currentDay->setFixedHeight(DDEDayCalendar::DDLableHeight);
    m_currentDay->setMinimumWidth(width());
    m_currentDay->setTextAlign(Qt::AlignCenter);
    QFont daylabelF;
    daylabelF.setWeight(QFont::Medium);
    daylabelF.setPixelSize(DDECalendar::FontSizeOneHundred);
    m_currentDay->setTextFont(daylabelF);
    midLayout->addWidget(m_currentDay);
    //本周
    m_currentWeek = new CustomFrame(this);
    m_currentWeek->setFixedHeight(DDEDayCalendar::DWLableHeight);
    m_currentWeek->setTextAlign(Qt::AlignCenter);
    QFont wlabelF;
    wlabelF.setPixelSize(DDECalendar::FontSizeSixteen);
    m_currentWeek->setTextFont(wlabelF);
    midLayout->addWidget(m_currentWeek);
    midLayout->addSpacing(2);
    //今天
    m_currentYear = new CustomFrame(this);
    m_currentYear->setFixedHeight(DDEDayCalendar::DWLableHeight);
    m_currentYear->setTextAlign(Qt::AlignCenter);
    m_currentYear->setTextFont(wlabelF);
    midLayout->addWidget(m_currentYear);
    midLayout->addSpacing(2);
    //阴历信息
    m_currentLuna = new CustomFrame(this);
    m_currentLuna->setFixedHeight(DDEDayCalendar::DHualiInfoLableHeight);
    m_currentLuna->setTextAlign(Qt::AlignCenter);
    QFont hlabelF;
    hlabelF.setPixelSize(DDECalendar::FontSizeTwelve);
    m_currentLuna->setTextFont(hlabelF);
    midLayout->addWidget(m_currentLuna);
    //宜
    m_yidownLayout = new QVBoxLayout;
    m_yidownLayout->setMargin(0);
    m_yidownLayout->setSpacing(0);
    m_yidownLayout->setContentsMargins(10, 5, 10, 0);
    hlabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_yiLabel = new CDayHuangLiLabel(this);
    m_yiLabel->setbackgroundColor(QColor("#75C18E"));
    m_yiLabel->setTextInfo(QColor("#7B7B7B "), hlabelF);
    m_yiLabel->setFixedSize(DDEDayCalendar::DHualiDtailLableWidth, DDEDayCalendar::DHualiDtailLableHeight);
    m_yidownLayout->addWidget(m_yiLabel);
    //忌
    m_jidownLayout = new QVBoxLayout;
    m_jidownLayout->setMargin(0);
    m_jidownLayout->setSpacing(0);
    m_jidownLayout->setContentsMargins(10, 10, 10, 10);
    m_jiLabel = new CDayHuangLiLabel(this);
    m_jiLabel->setbackgroundColor(QColor("#C17575"));
    m_jiLabel->setTextInfo(QColor("#7B7B7B "), hlabelF);
    m_jiLabel->setFixedSize(DDEDayCalendar::DHualiDtailLableWidth, DDEDayCalendar::DHualiDtailLableHeight);
    m_jidownLayout->addWidget(m_jiLabel);
    //总体布局
    m_hhLayout = new QVBoxLayout;
    m_hhLayout->setMargin(0);
    m_hhLayout->setSpacing(0);
    //添加上部分布局
    m_hhLayout->addLayout(m_upLayout);
    //添加中间布局
    m_hhLayout->addLayout(midLayout);
    //中间分割线
    m_splitline = new DHorizontalLine;
    m_splitline->setFixedSize(241, 2);
    QHBoxLayout *hlineLayout = new QHBoxLayout;
    hlineLayout->setMargin(0);
    hlineLayout->setSpacing(0);
    hlineLayout->setContentsMargins(0, 0, 0, 3);
    hlineLayout->addStretch(1);
    hlineLayout->addWidget(m_splitline);
    hlineLayout->addStretch(1);
    //添加分割线
    m_hhLayout->addLayout(hlineLayout);
    //添加宜布局
    m_hhLayout->addLayout(m_yidownLayout);
    //添加忌布局
    m_hhLayout->addLayout(m_jidownLayout);
    //弹簧
    m_hhLayout->addStretch();
    //设置布局
    setLayout(m_hhLayout);
}
/**
 * @brief CDayMonthView::initConnection 初始化connect
 */
void CDayMonthView::initConnection()
{
    connect(m_prevButton, &DIconButton::clicked, this, &CDayMonthView::slotprev);
    connect(m_today, &CTodyButton::clicked, this, &CDayMonthView::slottoday);
    connect(m_nextButton, &DIconButton::clicked, this, &CDayMonthView::slotnext);
    connect(this, &CDayMonthView::dateSelected, this, &CDayMonthView::handleCurrentDateChanged);
}
/**
 * @brief CDayMonthView::updateDateShow 更新数据显示
 */
void CDayMonthView::updateDateShow()
{
    QLocale locale;
    //set month text
    m_currentMouth->setTextStr(locale.monthName(m_currentDate.month(), QLocale::ShortFormat));
    //set day text
    m_currentDay->setTextStr(QString::number(m_currentDate.day()));
    //set week text
    if (m_currentDate.dayOfWeek() > 0)
        m_currentWeek->setTextStr(m_weeklist.at(m_currentDate.dayOfWeek() - 1));
    //set yearAndMonth text
    m_currentYear->setTextStr(m_currentDate.toString("yyyy/M"));
}
/**
 * @brief CDayMonthView::updateDateLunarDay 更新阴历信息
 */
void CDayMonthView::updateDateLunarDay()
{
    if (!m_huanglistate) return;
    //黄历信息
    CaHuangLiDayInfo info = getCaHuangLiDayInfo(getDateIndex(m_currentDate));
    //设置干支年月日
    m_currentLuna->setTextStr(info.mGanZhiYear + "年 " + "【" + info.mZodiac + "年】" + info.mGanZhiMonth + "月 " + info.mGanZhiDay + "日 ");
    //宜列表
    QStringList yilist = info.mSuit.split(".", QString::SkipEmptyParts);
    //忌列表
    QStringList jilist = info.mAvoid.split(".", QString::SkipEmptyParts);
    //设置文字
    m_yiLabel->setHuangLiText(yilist);
    m_jiLabel->setHuangLiText(jilist, 1);
}
/**
 * @brief CDayMonthView::getCellDayNum 获取cell的day
 * @param pos 索引
 * @return number
 */
const QString CDayMonthView::getCellDayNum(int pos)
{
    return QString::number(m_days[pos].day());
}
/**
 * @brief CDayMonthView::getCellDate 获取cell的日期
 * @param pos 索引
 * @return 日期
 */
const QDate CDayMonthView::getCellDate(int pos)
{
    return m_days[pos];
}
/**
 * @brief CDayMonthView::paintCell 绘制cell
 * @param cell widget
 */
void CDayMonthView::paintCell(QWidget *cell)
{
    //矩形
    const QRect rect(0, 0, cellwidth, cellheight);
    //索引
    const int pos = m_cellList.indexOf(cell);
    //是否选择cell
    const bool isSelectedCell = pos == m_selectedCell;
    //是否是今天
    const bool isCurrentDay = getCellDate(pos) == QDate::currentDate();
    //painter cell
    QPainter painter(cell);

    // draw selected cell background circle
    if (isSelectedCell) {
        int min = 0;
        //高度和宽度最小的一个
        min = cell->height() < cell->width() ? cell->height():cell->width();
        //最终rect
        QRectF fillRect;
        //宽度小于高度时的rect
        QRectF rectByWidth = QRectF(cell->width() * 0.15, (cell->height() - cell->width() * 0.75) / 2.0, min * 0.7, min * 0.7);
        //高度小于宽度时的rect
        QRectF rectByHeight = QRectF((cell->width() - cell->height() * 0.75) / 2.0, cell->height() * 0.15, min * 0.7, min * 0.7);
        //判断最终rect是哪一个
        fillRect = cell->height() < cell->width() ? rectByHeight : rectByWidth;
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        //设置系统活动色
        painter.setBrush(QBrush(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor()));
        painter.setPen(Qt::NoPen);
        //绘制圆形
        painter.drawEllipse(fillRect);
        painter.restore();
    }

    painter.setPen(Qt::SolidLine);
    //cell day
    const QString dayNum = getCellDayNum(pos);
    // draw text of day
    if (isSelectedCell) {
        //选择的颜色
        painter.setPen(m_selectedTextColor);
    } else if (isCurrentDay) {
        //今天的颜色
        painter.setPen(m_currentDayTextColor);
    } else {
        if (m_currentDate.month() == getCellDate(pos).month())
            //本月默认颜色
            painter.setPen(m_defaultTextColor);
        else
            //不属于本月
            painter.setPen(m_notCurrentTextColor);
    }

    QRect test;
    painter.setFont(m_dayNumFont);
    //绘制文字
    painter.drawText(rect, Qt::AlignCenter, dayNum, &test);
    //日程标志
    if (m_vlineflag.count() == DDEDayCalendar::PainterCellNum) {
        //有日程或节假日
        if (m_vlineflag[pos]) {
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::HighQualityAntialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            QPen pen;
            pen.setWidth(2);
            pen.setColor(m_ceventColor);
            painter.setPen(pen);
            //设置颜色
            painter.setBrush(QBrush(m_ceventColor));
            painter.setPen(Qt::NoPen);
            int r = cell->width() * (4 / 25);
            //设置半径
            if (r < 4) {
                r = 4;
            } else if ( r > 7) {
                r = 7;
            }
            //绘制圆
            painter.drawEllipse(cell->width() - r - 6, 4, r, r);
            painter.restore();
        }
    }
    painter.end();
}
/**
 * @brief CDayMonthView::cellClicked 点击cell
 * @param cell widget
 */
void CDayMonthView::cellClicked(QWidget *cell)
{
    if (!m_cellSelectable)
        return;
    //cell索引
    const int pos = m_cellList.indexOf(cell);
    //无效cell
    if (pos == -1)
        return;
    //设置选择的cell
    setSelectedCell(pos);
}
/**
 * @brief CDayMonthView::setSelectedCell 设置选择的cell
 * @param index 索引
 */
void CDayMonthView::setSelectedCell(int index)
{
    if (m_selectedCell == index)
        return;

    const int prevPos = m_selectedCell;
    m_selectedCell = index;

    updateDate();
    m_cellList.at(prevPos)->update();
    m_cellList.at(index)->update();
    //早于1900,return
    if (m_currentDate.year() < DDECalendar::QueryEarliestYear)
        return;
    emit signalcurrentDateChanged(m_days[index]);
}
/**
 * @brief CDayMonthView::updateCurrentLunar 更新当前的阴历
 */
void CDayMonthView::updateCurrentLunar()
{
    //更新数据
    updateDate();
    //更新数据显示
    updateDateShow();
    //更新阴历信息
    updateDateLunarDay();
    //发送信号
    emit signalcurrentLunarDateChanged(m_currentDate, getCaHuangLiDayInfo(getDateIndex(m_currentDate)), 1);
}
/**
 * @brief CDayMonthView::getLunar 获取阴历
 * @param pos 索引
 * @return 阴历
 */
const QString CDayMonthView::getLunar(int pos)
{
    //黄历信息
    CaHuangLiDayInfo info = getCaHuangLiDayInfo(pos);
    //初一显示为几月初一
    if (info.mLunarDayName == "初一") {
        info.mLunarDayName = info.mLunarMonthName + info.mLunarDayName;
    }

    if (info.mTerm.isEmpty())
        return info.mLunarDayName;

    return info.mTerm;
}
/**
 * @brief CDayMonthView::getCaHuangLiDayInfo 获取黄历信息
 * @param pos 索引
 * @return 黄历信息
 */
const CaHuangLiDayInfo CDayMonthView::getCaHuangLiDayInfo(int pos)
{
    //获取date
    const QDate date = m_days[pos];
    //包含日期，返回当天黄历信息
    if (lunarCache->contains(date)) {
        return lunarCache->value(date);
    }

    if (lunarCache->size() > 40)
        lunarCache->clear();
    //在队列中添加索引
    queue->push_back(pos);

    QTimer::singleShot(300, this, SLOT(getDbusData()));

    return *emptyCaHuangLiDayInfo;
}
/**
 * @brief CDayMonthView::getDbusData 获取dbus数据
 */
void CDayMonthView::getDbusData()
{
    //队列为空，返回
    if (queue->isEmpty())
        return;
    //队列头
    const int pos = queue->head();
    queue->pop_front();
    const QDate date = m_days[pos];
    if (!date.isValid()) {
        return;
    }
    //当天的黄历信息
    CaHuangLiDayInfo currentDayInfo;
    //设置当天的黄历信息
    if (!lunarCache->contains(date)) {
        CaHuangLiDayInfo scurrentDayinfo;
        if (m_DBusInter->GetHuangLiDayCalendar(date.year(), date.month(), date.day(), scurrentDayinfo)) {
            lunarCache->insert(date, scurrentDayinfo);
            currentDayInfo = scurrentDayinfo;
        }
    } else {
        currentDayInfo = lunarCache->value(date);
    }
    //更新数据
    m_cellList.at(pos)->update();
    // refresh   lunar info
    if (date == m_currentDate) {
        //更新当天阴历信息
        updateCurrentLunar();
    }
}
/**
 * @brief CDayMonthView::resizeEvent 调整窗口大小
 * @param event 窗口大小调整事件
 */
void CDayMonthView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    cellwidth = qRound(width() * 0.1005 + 0.5);
    cellheight = qRound(height() * 0.0496 + 0.5);
    m_gridLayout->setHorizontalSpacing(qRound(width() * 0.0287 + 0.5));
    m_gridLayout->setVerticalSpacing(0);
    int leftmagin = qRound(width() * 0.0332 + 0.5);
    int rightmagin = leftmagin;
    int topmagin = qRound(height() * 0.0164 + 0.5);
    int buttonmagin = topmagin;
    m_upLayout->setContentsMargins(leftmagin, topmagin, rightmagin, buttonmagin);
    m_dayNumFont.setPixelSize(qRound(12 + (width() - 347) / 71.66));

    for (int i(0); i != DDEDayCalendar::PainterCellNum; ++i) {
        m_cellList.at(i)->setFixedSize(cellwidth, cellheight);
        m_cellList.at(i)->update();
    }
    m_splitline->setFixedWidth(qRound(0.6925 * width() + 0.5));

    int hleftmagin = qRound(width() * 0.026 + 0.5);
    int hrightmagin = hleftmagin;
    int htopmagin = qRound(height() * 0.01773 + 0.5);
    int hbuttonmagin = htopmagin;
    int lw = width() - hleftmagin * 2;
    int lh = qRound(height() * 0.0992);
    m_yiLabel->setFixedSize(lw, lh);
    m_yidownLayout->setContentsMargins(hleftmagin, qRound(htopmagin * 0.5), hrightmagin, 0);
    m_jiLabel->setFixedSize(lw, lh);
    m_jidownLayout->setContentsMargins(hleftmagin, htopmagin, hrightmagin, hbuttonmagin);
}
/**
 * @brief CDayMonthView::wheelEvent 鼠标滑轮滚动
 * @param event 滑轮滚动事件
 */
void CDayMonthView::wheelEvent(QWheelEvent *event)
{
    //设置拖拽为false
    bool isDragging = false;
    //发送拖拽信号
    emit signalIsDragging(isDragging);
    //如果是拖拽则退出
    if (isDragging)
        return;
    if (event->delta() < 0) {
        //明天
        m_currentDate = m_currentDate.addDays(1);
        //设置返回今天按钮文字
        if (m_currentDate == QDate::currentDate()) {
            //今天
            m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
        } else {
            //返回今天
            m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
        }
        //发送datechange信号
        emit signalcurrentDateChanged(m_currentDate);
        //更新当天的阴历信息
        updateCurrentLunar();
    } else {
        //昨天
        QDate t_curret = m_currentDate.addDays(-1);
        //早于1900,return
        if (t_curret.year() < DDECalendar::QueryEarliestYear)
            return;
        //设置当前时间
        if (m_currentDate.year() >= DDECalendar::QueryEarliestYear) {
            m_currentDate = t_curret;
            //设置返回今天按钮文字
            if (m_currentDate == QDate::currentDate()) {
                //今天
                m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
            } else {
                //返回今天
                m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
            }
            emit signalcurrentDateChanged(m_currentDate);
            updateCurrentLunar();
        } else {
        }
    }
}
/**
 * @brief CDayMonthView::paintEvent 绘制圆角
 * @param e 绘图事件
 */
void CDayMonthView::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    int labelwidth = width();
    int labelheight = height();
    DPalette anipa = this->palette();
    QPainter painter(this);
    painter.save();
    // 反锯齿;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(anipa.background());
    painter.setPen(Qt::NoPen);
    //painter路径
    QPainterPath painterPath;
    painterPath.moveTo(m_radius, 0);
    painterPath.lineTo(0, 0);
    painterPath.lineTo(0, m_radius);
    painterPath.lineTo(0, labelheight - m_radius);
    painterPath.lineTo(0, labelheight);
    painterPath.lineTo(m_radius, labelheight);
    painterPath.lineTo(labelwidth - m_radius, labelheight);
    //绘制右下角
    if (!m_searchflag) {
        //圆角
        painterPath.arcTo(QRect(labelwidth - m_radius * 2, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 270, 90);
    } else {
        //直角
        painterPath.lineTo(labelwidth, labelheight);
        painterPath.lineTo(labelwidth, labelheight - m_radius);
    }
    painterPath.lineTo(labelwidth, m_radius);
    //绘制右上角
    if (!m_searchflag) {
        //圆角
        painterPath.arcTo(QRect(labelwidth - m_radius * 2, 0, m_radius * 2, m_radius * 2), 0, 90);
    } else {
        //直角
        painterPath.lineTo(labelwidth, 0);
        painterPath.lineTo(labelwidth - m_radius, 0);
    }
    painterPath.lineTo(m_radius, 0);
    painterPath.closeSubpath();
    //绘制路径
    painter.drawPath(painterPath);
    painter.restore();
}
/**
 * @brief CDayMonthView::slotprev 上一个月按钮
 */
void CDayMonthView::slotprev()
{
    emit signalSchedulHide();
    //早于1900年1月，return
    if (m_currentDate.year() == DDECalendar::QueryEarliestYear && m_currentDate.month() == 1)
        return;
    //下一个月时间
    if (m_currentDate.year() >= DDECalendar::QueryEarliestYear) {
        m_currentDate = m_currentDate.addMonths(-1);
        //设置返回今天按钮文字
        if (m_currentDate == QDate::currentDate()) {
            //今天
            m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
        } else {
            //返回今天
            m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
        }
        emit signalcurrentDateChanged(m_currentDate);
        updateCurrentLunar();
    } else {
    }
}
/**
 * @brief CDayMonthView::slotnext 下一个月
 */
void CDayMonthView::slotnext()
{
    emit signalSchedulHide();
    //下一个月时间
    m_currentDate = m_currentDate.addMonths(1);
    //设置返回今天按钮文字
    if (m_currentDate == QDate::currentDate()) {
        //今天
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        //返回今天
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }
    emit signalcurrentDateChanged(m_currentDate);
    //更新阴历
    updateCurrentLunar();
}
/**
 * @brief CDayMonthView::slottoday 返回今天
 */
void CDayMonthView::slottoday()
{
    //发送隐藏日程浮框信号
    emit signalSchedulHide();
    emit signalsReturnDay();
    //设置今天时间
    setCurrentDate(QDate::currentDate());
}
