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
#include "yearwindow.h"
#include "yearview.h"
#include "constants.h"
#include "schcedulesearchview.h"
#include "calendardbus.h"

#include <DPalette>
#include <DHiDPIHelper>

#include <QMessageBox>
#include <QMenuBar>
#include <QMouseEvent>
#include <QWheelEvent>

DGUI_USE_NAMESPACE
/**
 * @brief CYearWindow::CYearWindow 在构造函数中进行一些初始化设置
 * @param parent 父类
 */
CYearWindow::CYearWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //初始化UI
    initUI();
    //初始化信号槽连接
    initConnection();
    //去掉标题
    setWindowFlags(Qt::FramelessWindowHint);
    //设置中心边距
    setContentsMargins(0, 0, 0, 0);
}

CYearWindow::~CYearWindow()
{

}

/**
 * @brief eventFilter 过滤器，过滤返回今天的按钮事件
 * @param watched 事件对象
 * @param event 事件类型
 * @return false
 */
bool CYearWindow::eventFilter(QObject *watched, QEvent *event)
{
    //事件对象为：返回今天的按钮
    if (watched == m_today) {
        //对应的事件为：鼠标单击事件
        if (event->type() == QEvent::MouseButtonPress) {
            //返回今天
            slottoday();
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

/**
 * @brief mousePressEvent 鼠标单击事件
 * @param event 鼠标事件
 */
void CYearWindow::mousePressEvent(QMouseEvent *event)
{
    //单击日期区域外，隐藏日程浮框
    m_YearWidget->slotHideInfo();
    QMainWindow::mousePressEvent(event);
}

/**
 * @brief resizeEvent 窗口大小调整事件，搜索时，调整边框大小
 * @param event 窗口大小调整事件
 */
void CYearWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_topWidget->setGeometry(0, 0, this->width(), DDEMonthCalendar::M_YTopHeight);

    if (m_searchfalg) {
        //展示搜索结果时的边距
        m_tmainLayout->setContentsMargins(10, 0, 2, 0);
        m_topWidget->setContentsMargins(10, 0, 2, 0);
    } else {
        //没有搜索结果时的边距
        m_tmainLayout->setContentsMargins(10, 0, 10, 0);
        m_topWidget->setContentsMargins(10, 0, 10, 0);
    }
}

/**
 * @brief setDate 设置年视图当前显示的时间
 * @param date 年视图当前显示的时间
 */
void CYearWindow::setDate(QDate date)
{
    //无效数据
    if (!date.isValid()) return;
    //设置YearFram的日期
    m_YearWidget->setDate(date);
    //当前日期改变，重新设置日期
    m_YearWidget->slotcurrentDateChanged(date);
    //发送当前日期的信号
    emit signalCurrentDate(date);
    //更新当前日期，并设置年视图的时间
    slotUpdateCurrentDate(date);
}

/**
 * @brief initUI 初始化年视图的界面显示
 */
void CYearWindow::initUI()
{
    //设置背景色
    //创建背景Frame
    m_contentBackground = new DFrame(this);
    DPalette anipa = m_contentBackground->palette();
    anipa.setColor(DPalette::Background, "#F8F8F8");
    //是否可以设置背景色
    m_contentBackground->setAutoFillBackground(true);
    m_contentBackground->setPalette(anipa);
    //新建“返回今天”的label
    m_today = new QLabel(this);
    //设置过滤器
    m_today->installEventFilter(this);
    QFont todayfont;
    //设置字体大小
    todayfont.setPixelSize(DDECalendar::FontSizeSixteen);
    //“返回今天”的字体
    m_today->setFont(todayfont);
    //设置对齐方式
    m_today->setAlignment(Qt::AlignCenter);
    //设置字体
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    //设置宽度
    m_today->setFixedWidth(88);
    //是否可以设置背景色
    m_today->setAutoFillBackground(true);
    //设置高度
    m_today->setFixedHeight(DDEYearCalendar::Y_MLableHeight - 4);
    //新建上一年按钮
    m_prevButton = new DIconButton(DStyle::SP_ArrowLeft, this);
    //设置对象名称和辅助显示名称
    m_prevButton->setObjectName("PrevButton");
    m_prevButton->setAccessibleName("PrevButton");
    //设置按钮宽度
    m_prevButton->setFixedWidth(DDEYearCalendar::Y_MLableHeight);
    //设置按钮高度
    m_prevButton->setFixedHeight(DDEYearCalendar::Y_MLableHeight);

    m_nextButton = new DIconButton(DStyle::SP_ArrowRight, this);
    //设置对象名称和辅助显示名称
    m_nextButton->setObjectName("NextButton");
    m_nextButton->setAccessibleName("NextButton");
    //设置按钮宽度
    m_nextButton->setFixedWidth(DDEYearCalendar::Y_MLableHeight);
    //设置按钮高度
    m_nextButton->setFixedHeight(DDEYearCalendar::Y_MLableHeight);
    //新建年份的label
    m_YearLabel = new QLabel(this);
    //设置高度
    m_YearLabel->setFixedHeight(DDEYearCalendar::Y_YLableHeight);
    QFont t_labelF;
    t_labelF.setWeight(QFont::Medium);//设置字体权重
    //设置字体大小
    t_labelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    //为年份label设置字体
    m_YearLabel->setFont(t_labelF);
    DPalette pa = m_YearLabel->palette();
    pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    //设置颜色
    m_YearLabel->setPalette(pa);
    //新建阴历年label
    m_YearLunarLabel = new QLabel(m_contentBackground);
    //设置大小
    m_YearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunatLabelWindth, DDEMonthCalendar::M_YLunatLabelHeight);
    QFont ylabelF;
    //设置字体权重
    ylabelF.setWeight(QFont::Medium);
    //设置字体大小
    ylabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    //为阴历年label设置字体
    m_YearLunarLabel->setFont(ylabelF);
    DPalette Lunapa = m_YearLunarLabel->palette();
    Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    //设置颜色
    m_YearLunarLabel->setPalette(Lunapa);
    //新建阴历日label
    m_YearLunarDayLabel = new QLabel(m_contentBackground);
    //设置阴历日的大小
    m_YearLunarDayLabel->setFixedSize(108, DDEMonthCalendar::M_YLunatLabelHeight);
    //设置字体
    m_YearLunarDayLabel->setFont(ylabelF);
    //设置颜色
    m_YearLunarDayLabel->setPalette(Lunapa);
    //设置对齐方式
    m_YearLunarDayLabel->setAlignment(Qt::AlignRight);
    //新建title布局
    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->setContentsMargins(11, 12, 8, 10);
    //将年份label添加到布局中
    yeartitleLayout->addWidget(m_YearLabel);
    //新建title1布局
    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setMargin(0);
    yeartitleLayout1->setSpacing(0);
    yeartitleLayout1->setContentsMargins(4, 9, 0, 7);
    //将阴历年label添加到title1布局中
    yeartitleLayout1->addWidget(m_YearLunarLabel);
    yeartitleLayout1->addSpacing(390);
    yeartitleLayout1->addStretch();
    //将阴历日label添加到布局中，并设置对齐方式
    yeartitleLayout1->addWidget(m_YearLunarDayLabel, 0, Qt::AlignVCenter);
    //新建返回今天的Frame
    m_todayframe = new CustomFrame(this);
    m_todayframe->setContentsMargins(0, 0, 0, 0);
    m_todayframe->setRoundState(true, true, true, true);
    //设置颜色
    m_todayframe->setBColor(Qt::white);
    //设置高度
    m_todayframe->setFixedHeight(DDEYearCalendar::Y_MLableHeight);
    m_todayframe->setboreder(1);
    //新建返回今天的布局
    QHBoxLayout *todaylayout = new QHBoxLayout;
    todaylayout->setMargin(0);
    todaylayout->setSpacing(0);
    //将上一年添加到布局中
    todaylayout->addWidget(m_prevButton);
    //将返回今天label添加到布局中，并设置对齐方式
    todaylayout->addWidget(m_today, 0, Qt::AlignCenter);
    //将下一年添加到布局中
    todaylayout->addWidget(m_nextButton);
    //设置为Frame布局
    m_todayframe->setLayout(todaylayout);
    yeartitleLayout1->addSpacing(10);
    //将Frame添加到title1布局中
    yeartitleLayout1->addWidget(m_todayframe);
    //将title1布局添加到title布局中
    yeartitleLayout->addLayout(yeartitleLayout1);
    //为title新建一个widget
    m_topWidget = new DWidget(this);
    //设置布局为title布局
    m_topWidget->setLayout(yeartitleLayout);
    //新建两个YearFrame
    //第一个YearFrame
    YearWidget_First = new YearFrame();
    //第二个YearFrame
    YearWidget_Second = new YearFrame();
    //新建动画
    m_StackedWidget = new  AnimationStackedWidget(AnimationStackedWidget::TB);
    //将第一个YearFrame添加到动画中
    m_StackedWidget->addWidget(YearWidget_First);
    //将第二个YearFrame添加到动画中
    m_StackedWidget->addWidget(YearWidget_Second);
    m_StackedWidget->setContentsMargins(0, 0, 0, 0);
    //设置动画时间
    m_StackedWidget->setDuration(600);
    //获得初始的页面
    m_YearWidget = qobject_cast<YearFrame *>(m_StackedWidget->widget(0));
    //设置动画的布局
    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setMargin(0);
    hhLayout->setSpacing(0);
    hhLayout->setContentsMargins(0, 0, 0, 0);
    //将动画添加到动画的布局总
    hhLayout->addWidget(m_StackedWidget);
    //设置最终的布局
    m_tmainLayout = new QHBoxLayout;
    m_tmainLayout->setMargin(0);
    m_tmainLayout->setSpacing(0);
    m_tmainLayout->setContentsMargins(10, 0, 10, 0);
    //将动画的布局添加到最终布局中
    m_tmainLayout->addLayout(hhLayout);

    m_contentBackground->setContentsMargins(0, 0, 0, 0);
    //设置布局为最终布局
    m_contentBackground->setLayout(m_tmainLayout);

    //将Frame设置为中心部件
    setCentralWidget(m_contentBackground);
}

/**
 * @brief initConnection 初始化信号和槽的连接
 */
void CYearWindow::initConnection()
{
    //上/下一年clicked的信号和槽连接
    connect(m_prevButton, &DIconButton::clicked, this, &CYearWindow::slotprev);
    connect(m_nextButton, &DIconButton::clicked, this, &CYearWindow::slotnext);
    //和YearFrame之间的信号槽的连接
    connect(YearWidget_First,
            &YearFrame::signaldoubleclickDate,
            this,
            &CYearWindow::signaldoubleclickDate);
    connect(YearWidget_Second,
            &YearFrame::signaldoubleclickDate,
            this,
            &CYearWindow::signaldoubleclickDate);

    connect(YearWidget_First,
            &YearFrame::signalselectMonth,
            this,
            &CYearWindow::signalselectMonth);

    connect(YearWidget_Second,
            &YearFrame::signalselectMonth,
            this,
            &CYearWindow::signalselectMonth);
    connect(YearWidget_First,
            &YearFrame::signalselectWeekwindow,
            this,
            &CYearWindow::signalselectWeekwindow);

    connect(YearWidget_Second,
            &YearFrame::signalselectWeekwindow,
            this,
            &CYearWindow::signalselectWeekwindow);

    connect(YearWidget_First,
            &YearFrame::signalUpdateYearDate,
            this,
            &CYearWindow::slotUpdateCurrentDate);
    connect(YearWidget_Second,
            &YearFrame::signalUpdateYearDate,
            this,
            &CYearWindow::slotUpdateCurrentDate);
    connect(YearWidget_First,
            &YearFrame::signalupdateschcedule,
            this,
            &CYearWindow::signalupdateschcedule);

    connect(m_StackedWidget,
            &AnimationStackedWidget::signalIsFinished,
            this,
            &CYearWindow::setYearData);
}

/**
 * @brief setLunarVisible 设置年视图是否显示阴历信息
 * @param state 是否显示阴历信息
 */
void CYearWindow::setLunarVisible(bool state)
{
    //设置年份label是否可见
    m_YearLunarLabel->setVisible(state);
    //设置阴历年label是否可见
    m_YearLunarDayLabel->setVisible(state);
}

/**
 * @brief setTheMe 设置系统主题
 * @param type 主题类型
 */
void CYearWindow::setTheMe(int type)
{
    //系统类型为浅色或者跟随系统
    if (type == 0 || type == 1) {
        //设置背景颜色
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#F8F8F8");
        m_contentBackground->setPalette(anipa);
        //设置背景色
        m_contentBackground->setBackgroundRole(DPalette::Background);
        //设置返回今天的背景和字体颜色
        DPalette todaypa = m_today->palette();
        todaypa.setColor(DPalette::WindowText, QColor("#000000"));
        todaypa.setColor(DPalette::Background, Qt::white);
        m_today->setPalette(todaypa);
        m_today->setForegroundRole(DPalette::WindowText);
        m_today->setBackgroundRole(DPalette::Background);
        m_todayframe->setBColor(Qt::white);
        //设置年份的字体颜色
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
        m_YearLunarDayLabel->setPalette(Lunapa);
        m_YearLunarDayLabel->setForegroundRole(DPalette::WindowText);
    } else if (type == 2) {
        //系统类型为深色
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#252525");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);
        //设置返回今天按钮的背景和字体颜色
        DPalette todaypa = m_today->palette();
        todaypa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        QColor tbcolor = "#414141";
        tbcolor.setAlphaF(0.0);
        todaypa.setColor(DPalette::Background, tbcolor);
        m_today->setPalette(todaypa);
        m_today->setForegroundRole(DPalette::WindowText);
        m_today->setBackgroundRole(DPalette::Background);
        QColor tbcolor2 = "#414141";
        tbcolor2.setAlphaF(0.3);
        m_todayframe->setBColor(tbcolor2);
        //设置年份字体颜色
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历年字体颜色
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历日的颜色
        m_YearLunarDayLabel->setPalette(Lunapa);
        m_YearLunarDayLabel->setForegroundRole(DPalette::WindowText);
    }
    //设置YearFrame的系统类型
    YearWidget_First->setTheMe(type);
    YearWidget_Second->setTheMe(type);
    //设置title部分的背景颜色
    DPalette palette = m_topWidget->palette();
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::Window));
    m_topWidget->setAutoFillBackground(true);
    m_topWidget->setPalette(palette);
}
/**
 * @brief setSearchWFlag 设置是否在进行搜索
 * @param flag 是否所搜的标志
 */
void CYearWindow::setSearchWFlag(bool flag)
{
    //是否在查询日程的标志
    m_searchfalg = flag;
    //YearFram设置查询日程标志
    m_YearWidget->setSearchWFlag(flag);
}

/**
 * @brief clearSearch
 */
void CYearWindow::clearSearch()
{
}
/**
 * @brief getScheduleInfo 判断一年中每一天是否有日程信息
 */
void CYearWindow::getScheduleInfo()
{
    //获取信息并设置日程标志
    m_YearWidget->getInfoAndSetLineFlag();
}

/**
 * @brief slotTransitSearchSchedule
 * @param id
 */
void CYearWindow::slotTransitSearchSchedule(int id)
{
    emit signalsWUpdateShcedule(this, id);
}

/**
 * @brief slotSetSchceduleHide 隐藏日程浮框
 */
void CYearWindow::slotSetSchceduleHide()
{
    //隐藏日程浮框
    m_YearWidget->slotHideInfo();
}

/**
 * @brief slotprev 通过鼠标点击左箭头切换到上一年，刷新年视图下的所有内容，并隐藏日程展示浮窗。
 */
void CYearWindow::slotprev()
{
    //隐藏日程浮框
    m_YearWidget->slotHideInfo();
    //如果早于1900年，return
    if (m_currentdate.year() == DDECalendar::QueryEarliestYear)
        return;
    //如果正在翻页，return
    if (m_StackedWidget->IsRunning()) return;
    //上一年的日期
    QDate tcurrent = QDate(m_currentdate.year() - 1, m_currentdate.month(), m_currentdate.day());

    if (!tcurrent.isValid()) {
        //如果日期无效，
        //设置为上一年这个月的1号
        m_currentdate = QDate(m_currentdate.year() - 1, m_currentdate.month(), 1);
    } else {
        //日期有效
        m_currentdate = tcurrent;
    }
    if (m_currentdate.year() >= DDECalendar::QueryEarliestYear) {
        //当前页面的索引
        int index = m_StackedWidget->currentIndex();
        //获取上一页的索引值
        index = qAbs(index - 1) % 2;
        //通过索引值获得当前页面
        m_YearWidget = qobject_cast<YearFrame *>(m_StackedWidget->widget(index));
        //设置日期
        m_YearWidget->setDate(m_currentdate);
        //进行翻页动画
        m_StackedWidget->setPre();
        //发送当前日期的信号
        emit signalCurrentDate(m_currentdate);
    } else {
    }
}

/**
 * @brief slotnext 通过鼠标点击右箭头切换到下一年，并刷新年视图下的所有内容，并隐藏日程展示浮窗。
 */
void CYearWindow::slotnext()
{
    //隐藏日程浮框
    m_YearWidget->slotHideInfo();
    //如果正在进行动画，return
    if (m_StackedWidget->IsRunning()) return;
    //明年的今天
    QDate tcurrent = QDate(m_currentdate.year() + 1, m_currentdate.month(), m_currentdate.day());

    if (!tcurrent.isValid()) {
        //如果日期无效，
        //设置为明年这个月的1号
        m_currentdate = QDate(m_currentdate.year() + 1, m_currentdate.month(), 1);
    } else {
        //日期有效
        m_currentdate = tcurrent;
    }
    //当前页面的索引
    int index = m_StackedWidget->currentIndex();
    //明年的widget索引
    index = (index + 1) % 2;
    //通过索引值获取widget
    m_YearWidget = qobject_cast<YearFrame *>(m_StackedWidget->widget(index));
    //设置明年的日期
    m_YearWidget->setDate(m_currentdate);
    //开始翻页动画
    m_StackedWidget->setNext();
    emit signalCurrentDate(m_currentdate);
}

/**
 * @brief slottoday 通过鼠标点击返回今天按钮返回到今天，并刷新年视图下的所有内容，并隐藏日程展示浮窗。
 */
void CYearWindow::slottoday()
{
    //隐藏日程浮框
    m_YearWidget->slotHideInfo();
    //发送返回今天的信号
    emit signalsReturnTodayUpdate(this);
    //设置日期为今天
    setDate(QDate::currentDate());
}

/**
 * @brief slotReturnTodayUpdate
 */
void CYearWindow::slotReturnTodayUpdate()
{
}

/**
 * @brief slotupdateSchedule 更新当前时间
 * @param id
 */
void CYearWindow::slotupdateSchedule(const int id)
{
    //更新日期
    m_YearWidget->slotupdateSchedule(id);
}

/**
 * @brief slotUpdateCurrentDate 设置年视图当前时间的信息
 * @param date 时间
 */
void CYearWindow::slotUpdateCurrentDate(const QDate &date)
{
    //设置日期
    m_currentdate = date;
    //设置年视图信息
    setYearData();
}

/**
 * @brief setYearData 设置年视图阴历和按钮信息
 */
void CYearWindow::setYearData()
{
    if (m_currentdate == QDate::currentDate()) {
        //如果当天日期是今天，
        //按钮现实“今天”
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        //如果当前日期不是今天，
        //按钮现实“返回今天”
        m_today->setText(QCoreApplication::translate("Return", "Today", "Return"));
    }
    QLocale locale;

    if (locale.language() == QLocale::Chinese) {
        //如果系统语言为中文
        //显示XXXX年
        m_YearLabel->setText(QString::number(m_currentdate.year()) + tr("Y"));
    } else {
        //如果系统语言为其他语言，
        //只显示XXXX
        m_YearLabel->setText(QString::number(m_currentdate.year()));
    }

    //设置阴历年
    m_YearLunarLabel->setText(m_YearWidget->getLunarYear());
    //设置阴历日
    m_YearLunarDayLabel->setText(m_YearWidget->getLunarDay());
}

/**
 * @brief slotsearchDateSelect 设置选择的时间
 * @param date 时间
 */
void CYearWindow::slotsearchDateSelect(QDate date)
{
    //设置日期
    setDate(date);
}

/**
 * @brief wheelEvent 通过鼠标中间的滚轮滚动切换年份，并刷新年视图下的所有内容。
 * @param event 鼠标滚轮事件
 */
void CYearWindow::wheelEvent(QWheelEvent *event)
{
    if (event->delta() < 0) {
        //下一年
        slotnext();
    } else {
        //上一年
        slotprev();
    }
}

/**
 * @brief YearFrame 构造函数
 * @param parent 父类
 */
YearFrame::YearFrame(DWidget *parent)
    : DFrame(parent)
{
    m_DBusInter = new CalendarDBus("com.deepin.api.LunarCalendar",
                                   "/com/deepin/api/LunarCalendar",
                                   QDBusConnection::sessionBus(), this);
    //设置12个月的布局为网格布局
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setMargin(0);
    gridLayout->setSpacing(8);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    //3行4列，代表12个月份
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            //创建一个新的月份
            CYearView *view = new CYearView(this);
            //对信号和槽进行连接
            connect(view, &CYearView::signalcurrentDateChanged, this, &YearFrame::slotcurrentDateChanged);
            connect(view, &CYearView::signaldoubleclickDate, this, &YearFrame::signaldoubleclickDate);
            connect(view, &CYearView::signalselectWeekwindow, this, &YearFrame::signalselectWeekwindow);
            connect(view, &CYearView::signalupdateschcedule, this, &YearFrame::signalupdateschcedule);
            connect(view, &CYearView::signalselectMonth, this, &YearFrame::signalselectMonth);
            connect(view, &CYearView::signalHideInfo, this, &YearFrame::slotHideInfo);
            connect(view, &CYearView::signalSelectInfo, this, &YearFrame::slotSelectInfo);
            //将每个月份添加到布局中
            gridLayout->addWidget(view, i, j);
            //将每个月添加到list中
            m_monthViewList.append(view);
        }
    }
    //新建显示年份的label
    m_YearLabel = new QLabel(this);
    //设置年份label的高度
    m_YearLabel->setFixedHeight(DDEYearCalendar::Y_YLableHeight);
    QFont t_labelF;
    //设置字体的权重
    t_labelF.setWeight(QFont::Medium);
    //设置字体的大小
    t_labelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    //设置年份label的字体
    m_YearLabel->setFont(t_labelF);
    DPalette pa = m_YearLabel->palette();
    pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    //设置年份label的颜色
    m_YearLabel->setPalette(pa);

    //新建显示阴历年份的label
    m_YearLunarLabel = new QLabel();
    //设置阴历年份label的大小
    m_YearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunatLabelWindth, DDEMonthCalendar::M_YLunatLabelHeight);
    QFont ylabelF;
    //设置字体的权重
    ylabelF.setWeight(QFont::Medium);
    //设置字体的大小
    ylabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    //设置年份label的字体
    m_YearLunarLabel->setFont(ylabelF);
    DPalette Lunapa = m_YearLunarLabel->palette();
    Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    //设置年份label的颜色
    m_YearLunarLabel->setPalette(Lunapa);

    //设置title部分的布局
    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->setContentsMargins(11, 12, 8, 10);
    //将年份label添加到布局中
    yeartitleLayout->addWidget(m_YearLabel);
    //设置阴历年份的布局
    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setMargin(0);
    yeartitleLayout1->setSpacing(0);
    yeartitleLayout1->setContentsMargins(4, 9, 0, 7);
    //将阴历年label添加到布局中
    yeartitleLayout1->addWidget(m_YearLunarLabel);
    yeartitleLayout1->addSpacing(390);
    yeartitleLayout1->addStretch();
    yeartitleLayout1->addSpacing(10);
    //将阴历年份的布局添加到title的布局中
    yeartitleLayout->addLayout(yeartitleLayout1);

    //为title部分新建一个widget
    m_topWidget = new DWidget();
    //widget设置布局为title的布局
    m_topWidget->setLayout(yeartitleLayout);
    //设置widget的高度
    m_topWidget->setFixedHeight(DDEMonthCalendar::M_YTopHeight);
    //总体布局
    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setMargin(0);
    hhLayout->setSpacing(0);
    hhLayout->setContentsMargins(0, 0, 0, 0);
    hhLayout->addWidget(m_topWidget);
    //将title的widget添加到总体布局中
    //将12个月份的布局添加到总体布局中

    hhLayout->addLayout(gridLayout);
    //YearFrame设置布局为总体布局
    this->setLayout(hhLayout);
    setContentsMargins(0, 0, 0, 10);
    //设置圆角
    setFrameRounded(true);
}

/**
  *@brief ~YearFrame 析构函数
  */
YearFrame::~YearFrame()
{

}

/**
 * @brief setDate 设置当前时间
 * @param date 时间
 */
void YearFrame::setDate(QDate &date)
{
    //日期无效，return
    if (!date.isValid()) return;
    //设置当前日期
    m_currentdate = date;

    for (int i = 0; i < DDEYearCalendar::FrameSizeofEveryYear; i++) {
        //初始化时间为今年每个月的1号
        QDate tdate(m_currentdate.year(), i + 1, 1);
        //设置时间
        m_monthViewList.at(i)->setCurrentDate(tdate, 0);
    }
    for (int i = 0; i < DDEYearCalendar::FrameSizeofEveryYear; i++) {
        //初始化时间为今年每个月的1号
        QDate tdate(m_currentdate.year(), i + 1, 1);
        //如果时间为当前年当前月
        if (date.year() == tdate.year() && date.month() == tdate.month()) {
            //设置时间
            m_monthViewList.at(i)->setCurrentDate(date, 1);
            break;
        }
    }
    //获取阴历信息
    getLunarData();
}

/**
 * @brief getInfoAndSetLineFlag 获取日程信息，设置是否有日程的标志
 */
void YearFrame::getInfoAndSetLineFlag()
{
    for (int i = 0; i < DDEYearCalendar::FrameSizeofEveryYear; i++) {
        //获取每个月的日程
        m_monthViewList.at(i)->getInfoAndSetLineFlag();
    }
}

/**
 * @brief setTheMe 根据系统主题类型设置颜色
 * @param type 系统主题类型
 */
void YearFrame::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        //系统类型为浅色或自适应
        //设置背景颜色
        DPalette gpa = palette();
        gpa.setColor(DPalette::Background, "#F8F8F8");
        setPalette(gpa);
        setBackgroundRole(DPalette::Background);
        //设置阳历年的字体颜色
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历年的字体颜色
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
    } else if (type == 2) {
        //系统类型为深色
        //设置背景颜色
        DPalette gpa = palette();
        gpa.setColor(DPalette::Background, "#252525");
        setPalette(gpa);
        setBackgroundRole(DPalette::Background);
        //设置阳历年的字体颜色
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历年的字体颜色
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
    }
    //设置每个月的系统类型
    for (int i = 0; i < DDEYearCalendar::FrameSizeofEveryYear; i++) {
        m_monthViewList.at(i)->setTheMe(type);
    }
}

/**
 * @brief setSearchWFlag 设置搜索的标志
 * @param flag 是否进行了搜索
 */
void YearFrame::setSearchWFlag(bool flag)
{
    //搜索标志
    m_searchfalg = flag;
}

/**
 * @brief getLunarData 更新当天的农历年份数据。
 */
void YearFrame::getLunarData()
{
    bool o1 = true;
    //通过dbus服务，获取当天阴历信息的应答
    QDBusReply<CaLunarDayInfo> replydd = m_DBusInter->GetLunarInfoBySolar(m_currentdate.year(), m_currentdate.month(), m_currentdate.day(), o1);
    //获取阴历信息
    CaLunarDayInfo currentDayInfo = replydd.value();
    //设置阴历年
    m_LunarYear = QString("-%0%1年-").arg(currentDayInfo.mGanZhiYear).arg(currentDayInfo.mZodiac);
    //设置阴历日
    m_LunarDay = QString("-农历%0%1-").arg(currentDayInfo.mLunarMonthName).arg(currentDayInfo.mLunarDayName);
    QLocale locale;

    if (locale.language() == QLocale::Chinese) {
        //如果系统语言为中文
        //显示XXXX年
        m_YearLabel->setText(QString::number(m_currentdate.year()) + tr("Y"));
        //显示阴历年
        m_YearLunarLabel->setText(m_LunarYear);
    } else {
        //如果系统语言不是中文
        //显示XXXX
        m_YearLabel->setText(QString::number(m_currentdate.year()));
        //不显示阴历年
        m_YearLunarLabel->setText("");
    }

}

/**
 * @brief mousePressEvent 鼠标单击日期区域外隐藏日程浮框
 * @param event 鼠标单击事件
 */
void YearFrame::mousePressEvent(QMouseEvent *event)
{
    if (m_selectFlag) return;
    //鼠标左键
    if (event->button() == Qt::LeftButton) {
        //隐藏日程浮框
        slotHideInfo();
    }
}

/**
 * @brief slotcurrentDateChanged 设置当前时间
 * @param date 时间
 */
void YearFrame::slotcurrentDateChanged(QDate date)
{
    //设置当前时间
    m_currentdate = date;
    //获取阴历信息
    getLunarData();
    //发送更新年日期的信号
    emit signalUpdateYearDate(date);
}

/**
 * @brief slotHideInfo 隐藏日程浮框
 */
void YearFrame::slotHideInfo()
{
    //隐藏日程信息
    CYearView::SchceduleViewHide();
}

/**
 * @brief slotSelectInfo 设置是否选择了日期的标志
 * @param flag 是否选择了如期
 */
void YearFrame::slotSelectInfo(bool flag)
{
    //搜索标志
    m_selectFlag = flag;
}

/**
 * @brief slotupdateSchedule 更新日程
 * @param id
 */
void YearFrame::slotupdateSchedule(const int id)
{
    //设置每个月的时间
    for (int i =0; i < m_monthViewList.size(); ++i) {
        m_monthViewList.at(i)->slotupdateSchedule(id);
    }
}

/**
 * @brief slotSetSchceduleHide 隐藏日程浮框
 */
void YearFrame::slotSetSchceduleHide()
{
    //隐藏日程浮框
    CYearView::SchceduleViewHide();
}
