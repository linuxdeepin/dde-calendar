/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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
#include "yearschceduleview.h"
#include "schcedulectrldlg.h"
#include "myschceduleview.h"
#include "schceduledlg.h"
#include "scheduledatamanage.h"
#include "constants.h"

#include <DMessageBox>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DPalette>

#include <QAction>
#include <QMenu>
#include <QListWidget>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>
#include <QMouseEvent>

DGUI_USE_NAMESPACE

const QString fontfamily = QStringLiteral("SourceHanSansSC-Medium");
/**
 * @brief CYearSchceduleItem 构造函数
 * @param parent 父类
 */
CYearSchceduleItem::CYearSchceduleItem(QWidget *parent)
    : DLabel(parent)
{

}
/**
 * @brief setBackgroundColor 设置背景颜色
 * @param color1 背景颜色
 */
void CYearSchceduleItem::setBackgroundColor(QColor color1)
{
    m_Backgroundcolor = color1;
}
/**
 * @brief setStateColor 设置圆点的颜色
 * @param color1 圆点颜色
 */
void CYearSchceduleItem::setStateColor(QColor color1)
{
    m_Statecolor = color1;
}
/**
 * @brief setText 设置文字颜色和字体
 * @param tcolor 文字颜色
 * @param font 文字字体
 */
void CYearSchceduleItem::setText( QColor tcolor, QFont font)
{
    m_ttextcolor = tcolor;
    m_tfont = font;
}
/**
 * @brief setTimeC 设置时间的颜色和字体
 * @param tcolor 时间的颜色
 * @param font 时间的字体
 */
void CYearSchceduleItem::setTimeC(QColor tcolor, QFont font)
{
    m_timecolor = tcolor;
    m_timefont = font;
}
/**
 * @brief setData 设置日程信息
 * @param vScheduleInfo 日程信息
 */
void CYearSchceduleItem::setData( ScheduleDtailInfo vScheduleInfo)
{
    m_ScheduleInfo = vScheduleInfo;
    update();
}
/**
 * @brief paintEvent
 * @param e
 */
void CYearSchceduleItem::paintEvent( QPaintEvent *e )
{
    Q_UNUSED(e);
    int labelwidth = width();
    int labelheight = height();

    QPainter painter(this);
    QRect fillRect = QRect(0, 0, labelwidth, labelheight);
    painter.setRenderHints(QPainter::HighQualityAntialiasing);
    QColor bcolor = m_Backgroundcolor;
    painter.save();
    // 反锯齿;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(bcolor));
    painter.setPen(Qt::NoPen);
    painter.drawRect(fillRect);
    painter.restore();
    //圆点
    painter.save();
    // 反锯齿;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(m_Statecolor));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QRect(10, (labelheight - 8) / 2, 8, 8));
    painter.restore();

    QString str;
    //左边文字
    painter.save();
    // 反锯齿;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(m_ttextcolor);
    painter.setFont(m_tfont);
    QFontMetrics fm = painter.fontMetrics();
    QString tStitlename = m_ScheduleInfo.titleName;
    tStitlename.replace("\n", "");
    str = tStitlename;
    int tilenameW = labelwidth - 90;
    QString tstr;

    for (int i = 0; i < str.count(); i++) {
        tstr.append(str.at(i));
        int widthT = fm.width(tstr) + 5;
        if (widthT >= tilenameW) {
            tstr.chop(1);
            break;
        }
    }
    if (tstr != str) {
        tstr = tstr + "...";
    }

    painter.drawText(QRect(28, 0, tilenameW, labelheight - 2), Qt::AlignLeft | Qt::AlignVCenter, tstr);
    painter.restore();
    //右边时间
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setPen(m_timecolor);
    painter.setFont(m_timefont);
    QLocale locale;

    if (locale.language() == QLocale::Chinese) {
        if (m_ScheduleInfo.allday) {
            str = tr("All Day");
        } else {
            str = m_ScheduleInfo.beginDateTime.time().toString("ap h") + ("时");
        }
    } else {
        if (m_ScheduleInfo.allday) {
            str = tr("All Day");
        } else {
            str = m_ScheduleInfo.beginDateTime.time().toString("ap h:mm");
        }
    }
    QFontMetrics fm2 = painter.fontMetrics();
    painter.drawText(QRect(labelwidth - 60, 0, 50, labelheight - 2), Qt::AlignRight | Qt::AlignVCenter, str);
    painter.restore();
}
/**
 * @brief CYearSchceduleView::CYearSchceduleView 构造函数
 * @param parent 父类
 */
CYearSchceduleView::CYearSchceduleView(QWidget *parent)
    : DWidget(parent)
{
    //设置边距
    setContentsMargins(10, 10, 10, 10);
    //设置windowflags
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    //设置属性
    setAttribute(Qt::WA_TranslucentBackground);
}
/**
 * @brief CYearSchceduleView::~CYearSchceduleView 析构函数
 */
CYearSchceduleView::~CYearSchceduleView()
{

}
/**
 * @brief CYearSchceduleView::setSoloDay
 * @param soloday
 */
void CYearSchceduleView::setSoloDay(QString soloday)
{
    m_soloDay = soloday;
}
/**
 * @brief YScheduleDateThan 比较日程的开始时间
 * @param s1 第一个日程
 * @param s2 第二个日程
 * @return 比较结果
 */
bool YScheduleDateThan(const ScheduleDtailInfo &s1, const ScheduleDtailInfo &s2)
{
    QDate bdate1 = s1.beginDateTime.date();
    QDate edate1 = s1.endDateTime.date();
    QDate bdate2 = s2.beginDateTime.date();
    QDate edate2 = s2.endDateTime.date();

    if (bdate1 != edate1 && bdate2 == edate2) {
        return true;
    } else if (bdate1 == edate1 && bdate2 != edate2) {
        return false;
    } else if (bdate1 != edate1 && bdate2 != edate2) {
        return bdate1 < bdate2;
    } else {
        if (s1.beginDateTime == s2.beginDateTime) {
            return s1.titleName < s2.titleName;
        } else {
            return s1.beginDateTime < s2.beginDateTime;
        }
    }
}
/**
 * @brief YScheduleDaysThan 比较日程跨越的天数
 * @param s1 第一个日程
 * @param s2 第二个日程
 * @return 比较结果
 */
bool YScheduleDaysThan(const ScheduleDtailInfo &s1, const ScheduleDtailInfo &s2)
{
    return s1.beginDateTime.date().daysTo(s1.endDateTime.date()) > s2.beginDateTime.date().daysTo(s2.endDateTime.date());
}
/**
 * @brief CYearSchceduleView::setData 设置数据
 * @param vListData 日程信息
 */
void CYearSchceduleView::setData(QVector<ScheduleDtailInfo> &vListData)
{
    QVector<ScheduleDtailInfo> valldayListData, vDaylistdata;

    for (int i = 0; i < vListData.count(); i++) {
        if (vListData.at(i).allday) {
            //全天日程
            valldayListData.append(vListData.at(i));
        } else {
            //非全天日程
            vDaylistdata.append(vListData.at(i));
        }
    }
    //排序
    std::sort(valldayListData.begin(), valldayListData.end(), YScheduleDaysThan);
    std::sort(valldayListData.begin(), valldayListData.end(), YScheduleDateThan);
    std::sort(vDaylistdata.begin(), vDaylistdata.end(), YScheduleDaysThan);
    std::sort(vDaylistdata.begin(), vDaylistdata.end(), YScheduleDateThan);

    for (int i = 0; i < valldayListData.count(); i++) {
        QVector<ScheduleDtailInfo>::iterator iter = valldayListData.begin();
        if (valldayListData.at(i).type.ID == DDECalendar::FestivalTypeID) {
            //节日信息放在最前面
            ScheduleDtailInfo moveDate;
            moveDate = valldayListData.at(i);
            valldayListData.removeAt(i);
            valldayListData.insert(iter, moveDate);
        }
    }
    //清空日程容器
    m_vlistData.clear();
    //将全天日程添加到日程容器中
    m_vlistData.append(valldayListData);
    //将非全天日程添加到日程容器中
    m_vlistData.append(vDaylistdata);

    if (m_vlistData.size() > DDEYearCalendar::YearScheduleListMaxcount) {
        //如果节日和日程个数总数超过了5个
        QVector<ScheduleDtailInfo> vTlistData;
        for (int i = 0; i < 4; i++) {
            //取容器中的前四个进行展示
            if (m_vlistData.at(i).beginDateTime.date() != m_vlistData.at(i).endDateTime.date() && !m_vlistData.at(i).allday) {
                if (m_vlistData.at(i).beginDateTime.date() != m_currentDate) {
                    //跨天日程的中间部分设置为全天
                    m_vlistData[i].allday = true;
                }
            }
            //将前四个日程添加到容器中
            vTlistData.append(m_vlistData.at(i));
        }
        ScheduleDtailInfo info;
        //剩下的日程显示为...
        info.titleName = "......";
        info.id = -1;
        vTlistData.append(info);
        m_vlistData = vTlistData;
    }
}
/**
 * @brief clearData 清除日程信息
 */
void CYearSchceduleView::clearData()
{
    m_vlistData.clear();
    m_soloDay.clear();
    return;
}
/**
 * @brief showWindow 设置日程浮框的大小
 * @return 日程浮框的宽度
 */
int CYearSchceduleView::showWindow()
{
    if (m_soloDay.isEmpty() && m_vlistData.isEmpty()) {
        //无日程
        setFixedSize(130, 45);
    } else {
        //有日程
        setFixedSize(240, 180);
    }
    //更新展示
    updateDateShow();

    return this->width();
}
/**
 * @brief setTheMe 根据系统主题类型设置颜色
 * @param type 系统主题类型
 */
void CYearSchceduleView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        //系统主题类型为浅色或跟随系统
        //设置背景色
        m_bBackgroundcolor = "#FFFFFF";
        m_bBackgroundcolor.setAlphaF(0.0);
        //设置时间的字体颜色
        m_btimecolor = "#414D68";
        m_btimecolor.setAlphaF(0.7);
        //设置文字的字体颜色
        m_bttextcolor = "#414D68";
        m_lBackgroundcolor = "#EBEBEB";
        m_lBackgroundcolor.setAlphaF(0.0);
        m_ltextcolor = "#001A2E";
        m_solocolor = "#FF7272";
        m_TBcolor = "#EBEBEB";
        m_borderColor = "#000000";
        m_borderColor.setAlphaF(0.05);
    } else if (type == 2) {
        //系统主题类型为深色
        m_bBackgroundcolor = "#FFFFFF";
        m_bBackgroundcolor.setAlphaF(0.0);
        m_btimecolor = "#C0C6D4";
        m_btimecolor.setAlphaF(0.7);
        m_bttextcolor = "#C0C6D4";
        m_lBackgroundcolor = "#191919";
        m_lBackgroundcolor.setAlphaF(0.00);
        m_ltextcolor = "#C0C6D4";
        m_solocolor = "#FF7272";
        m_solocolor.setAlphaF(0.8);
        m_TBcolor = "#191919";
        m_borderColor = "#FFFFFF";
        m_borderColor.setAlphaF(0.05);
    }
}
/**
 * @brief setDtype
 * @param type
 * @param arrowheight
 */
void CYearSchceduleView::setDtype(int type, int arrowheight)
{
    m_dtype = type;
    m_arrowheight = arrowheight;
    update();
}
/**
 * @brief setCurrentDate 设置日程所在当天的日期
 * @param cdate 日期
 */
void CYearSchceduleView::setCurrentDate(QDate cdate)
{
    m_currentDate = cdate;
}
/**
 * @brief getCurrentDate 获取日程所在当天的日期
 * @return 日期
 */
QDate CYearSchceduleView::getCurrentDate()
{
    return  m_currentDate;
}
/**
 * @brief adjustPosition 根据日程浮框左右朝向不同，日程显示位置不同
 * @param ad 是否调整显示位置
 */
void CYearSchceduleView::adjustPosition(bool ad)
{
    adjustPos = ad;
}
/**
 * @brief updateDateShow 调整最多展示日程为五个，并设置浮框大小
 */
void CYearSchceduleView::updateDateShow()
{
    //展示日程的数量
    int sviewNum = 0;

    if (!m_soloDay.isEmpty() || !m_vlistData.isEmpty()) {
        //节假日或日程不为空
        if (!m_soloDay.isEmpty()) {
            if (m_vlistData.size() > 4) {
                //节假日和日程总数大于4时，设置最多显示为5
                sviewNum = DDEYearCalendar::YearScheduleListMaxcount;
            } else {
                //展示日程的数量+1
                sviewNum = m_vlistData.size() + 1;
            }
        } else {
            if (m_vlistData.size() > DDEYearCalendar::YearScheduleListMaxcount) {
                sviewNum = DDEYearCalendar::YearScheduleListMaxcount;
            } else {
                sviewNum = m_vlistData.size();
            }
        }
    }
    //设置日程浮框显示大小
    if (!m_soloDay.isEmpty() || !m_vlistData.isEmpty())
        setFixedSize(240, 45 + (sviewNum - 1) * 29);
    update();

    return;
}
/**
 * @brief createItemWidget 设置每条日程的具体信息
 * @param info 日程信息
 * @param type 系统主题类型
 */
void CYearSchceduleView::createItemWidget(ScheduleDtailInfo info, int type)
{
    //日程信息
    ScheduleDtailInfo &gd = info;
    //日程颜色
    CSchedulesColor gdcolor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(gd.type.ID);
    //日程item
    CYearSchceduleItem *gwi = new CYearSchceduleItem();
    //字体
    QFont font;
    //设置字体的权重
    font.setWeight(QFont::Medium);
    //设置字体的大小
    font.setPixelSize(DDECalendar::FontSizeFourteen);
    //设置背景色
    gwi->setBackgroundColor(m_bBackgroundcolor);
    QColor scolor = gdcolor.Purecolor;
    //设置透明度
    scolor.setAlphaF(1.0);
    //根据系统主题类型设置颜色
    if (type == 0)
        gwi->setStateColor(gdcolor.splitColor);
    else {
        gwi->setStateColor(m_solocolor);
    }
    //设置日程item的字体
    gwi->setText(m_bttextcolor, font);
    font.setPixelSize(DDECalendar::FontSizeTwelve);
    gwi->setTimeC(m_btimecolor, font);
    //设置日程item大小
    gwi->setFixedSize(m_gradientItemList->width() - 3, 28);
    //设置日程信息
    gwi->setData(gd);

    QListWidgetItem *listItem = new QListWidgetItem;
    listItem->setSizeHint(QSize(m_gradientItemList->width() - 2, 29)); //每次改变Item的高度
    listItem->setFlags(Qt::ItemIsTristate );
    m_gradientItemList->addItem(listItem);
    m_gradientItemList->setItemWidget(listItem, gwi);
    //将日程item添加到容器中
    m_labellist.append(gwi);
}
/**
 * @brief paintEvent 绘制日程
 * @param event 绘图事件
 */
void CYearSchceduleView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    for (int i = 0; i < m_vlistData.size(); ++i) {
        //绘制有节假日或日程的item
        paintItem(m_vlistData.at(i), i, 0);
    }
    if (m_soloDay.isEmpty() && m_vlistData.isEmpty()) {
        //绘制无日程的item
        paintItem();
    }
    adjustPos = false;
}
/**
 * @brief paintItem 绘制日程
 * @param info 日程信息
 * @param index 日程的索引
 * @param type 系统主题类型
 */
void CYearSchceduleView::paintItem(ScheduleDtailInfo info, int index, int type)
{
    int labelwidth = width() - 30;
    int bheight = index * 29 + 10;
    int labelheight = 28;
    //日程信息
    ScheduleDtailInfo &gd = info;
    //日程颜色
    CSchedulesColor gdcolor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(gd.type.ID);
    //字体
    QFont font;
    //设置字体的权重
    font.setWeight(QFont::Medium);
    font.setPixelSize(DDECalendar::FontSizeTwelve);
    QColor scolor = gdcolor.Purecolor;
    //设置透明度
    scolor.setAlphaF(1.0);

    QPainter painter(this);
    //日程所在矩形
    QRect fillRect = QRect(0, 0, width(), height());
    painter.setRenderHints(QPainter::HighQualityAntialiasing);
    QColor bcolor = m_bBackgroundcolor;

    painter.save();
    // 反锯齿;
    painter.setRenderHint(QPainter::Antialiasing);
    //设置画刷
    painter.setBrush(QBrush(bcolor));
    painter.setPen(Qt::NoPen);
    //绘制日程所在的矩形
    painter.drawRect(fillRect);
    painter.restore();

    if (gd.id == -1) {
        QString str = "...";

        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        //设置画笔颜色
        painter.setPen(m_btimecolor);
        //设置字体
        painter.setFont(font);
        //如果日程个数多于5个时，绘制...
        if (adjustPos) {
            //日程浮框在左边时，将日程显示内容向左移动，保证整体效果不变
            painter.drawText(QRect(25 - 18, bheight, labelwidth - 80, labelheight - 2), Qt::AlignLeft | Qt::AlignVCenter, str);
        } else {
            painter.drawText(QRect(25, bheight, labelwidth - 80, labelheight - 2), Qt::AlignLeft | Qt::AlignVCenter, str);
        }
        painter.restore();
    } else {
        if (info.id != -1) {
            //圆点
            painter.save();
            // 反锯齿;
            painter.setRenderHint(QPainter::Antialiasing);
            //根据系统类型设置画刷的颜色
            if (type == 0)
                painter.setBrush(QBrush(gdcolor.splitColor));
            else {
                painter.setBrush(QBrush(m_solocolor));
            }
            painter.setPen(Qt::NoPen);
            //绘制圆点
            if (adjustPos) {
                painter.drawEllipse(QRect(25 - 18, bheight + (labelheight - 8) / 2, 8, 8));
            } else {
                painter.drawEllipse(QRect(25, bheight + (labelheight - 8) / 2, 8, 8));
            }
            painter.restore();
        }

        QString str;
        //左边文字
        painter.save();
        // 反锯齿;
        painter.setRenderHint(QPainter::Antialiasing);
        //设置绘制文字的画笔的颜色
        painter.setPen(m_bttextcolor);
        painter.setFont(font);
        //根据字体类型，获得字体大小
        QFontMetrics fm = painter.fontMetrics();
        QString tStitlename = gd.titleName;
        //替换到换行
        tStitlename.replace("\n", "");
        str = tStitlename;
        int tilenameW = labelwidth - 80;
        QString tstr;

        for (int i = 0; i < str.count(); i++) {
            tstr.append(str.at(i));
            int widthT = fm.width(tstr) + 5;
            if (widthT >= tilenameW) {
                tstr.chop(1);
                break;
            }
        }
        if (tstr != str) {
            //日程标题过长用...代替
            tstr = tstr + "...";
        }
        //绘制左边的文字
        if (adjustPos) {
            painter.drawText(QRect(41 - 18, bheight, tilenameW, labelheight - 2), Qt::AlignLeft | Qt::AlignVCenter, tstr);
        } else {
            painter.drawText(QRect(41, bheight, tilenameW, labelheight - 2), Qt::AlignLeft | Qt::AlignVCenter, tstr);
        }
        painter.restore();

        if (info.id != -1) {
            //右边时间
            painter.save();
            // 反锯齿;
            painter.setRenderHint(QPainter::Antialiasing);
            //设置绘制时间的画笔颜色
            painter.setPen(m_btimecolor);
            //设置字体
            painter.setFont(font);
            QLocale locale;

            if (info.allday) {
                //如果是全天日程，时间设置为全天
                str = tr("All Day");
            } else {
                if (m_currentDate > info.beginDateTime.date()) {
                    //如果是跨天中间的日程，时间设置为全天
                    str = tr("All Day");
                } else {
                    //时间设置为日程的开始时间
                    str = info.beginDateTime.time().toString("hh:mm");
                }
            }

            QFontMetrics fm2 = painter.fontMetrics();
            //绘制右边时间
            if (adjustPos) {
                painter.drawText(QRect(width() - 70 - 18, bheight, 57, labelheight - 2), Qt::AlignRight | Qt::AlignVCenter, str);
            } else {
                painter.drawText(QRect(width() - 70, bheight, 57, labelheight - 2), Qt::AlignRight | Qt::AlignVCenter, str);
            }
            painter.restore();
        }
    }
}

/**
 * @brief paintItem
 */
void CYearSchceduleView::paintItem()
{
    QFont font;
    //设置字体大小
    font.setPixelSize(DDECalendar::FontSizeTwelve);
    QPainter painter(this);
    //无日程所在矩形
    QRect fillRect = QRect(0, 0, width(), height());
    painter.setRenderHints(QPainter::HighQualityAntialiasing);
    QColor bcolor = m_bBackgroundcolor;
    painter.save();
    // 反锯齿;
    painter.setRenderHint(QPainter::Antialiasing);
    //设置画刷颜色
    painter.setBrush(QBrush(bcolor));
    painter.setPen(Qt::NoPen);
    //绘制矩形
    painter.drawRect(fillRect);
    painter.restore();

    QString str;
    //左边文字
    painter.save();
    // 反锯齿;
    painter.setRenderHint(QPainter::Antialiasing);
    //设置画笔颜色
    painter.setPen(m_bttextcolor);
    painter.setFont(font);
    QFontMetrics fm = painter.fontMetrics();
    //设置内容
    QString tStitlename = tr("No event");
    //绘制文字
    painter.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, tStitlename);
    painter.restore();
}

/**
 * @brief CYearSchceduleOutView 构造函数
 * @param parent 父类
 */
CYearSchceduleOutView::CYearSchceduleOutView(QWidget *parent)
    : DArrowRectangle(DArrowRectangle::ArrowLeft, parent)
{
    yearschceduleview = new CYearSchceduleView ();
    this->setContent(yearschceduleview);
}

/**
 * @brief setSoloDay
 * @param soloday
 */
void CYearSchceduleOutView::setSoloDay(QString soloday)
{
    yearschceduleview->setSoloDay(soloday);
}

/**
 * @brief setData 设置日程信息
 * @param vListData 日程信息
 */
void CYearSchceduleOutView::setData(QVector<ScheduleDtailInfo> &vListData)
{
    //设置日程个数
    list_count = vListData.size();
    yearschceduleview->setData(vListData);
    //获取日程信息
    scheduleinfoList = yearschceduleview->getlistdate();
}

/**
 * @brief clearData 清除数据
 */
void CYearSchceduleOutView::clearData()
{
    yearschceduleview->clearData();
}

/**
 * @brief showWindow 设置日程浮框外边框大小
 */
void CYearSchceduleOutView::showWindow()
{
    int w = yearschceduleview->showWindow();
    //设置日程浮框大小
    this->setFixedSize(w, yearschceduleview->height());
}

/**
 * @brief setTheMe 设置系统主题颜色
 * @param type 系统主题
 */
void CYearSchceduleOutView::setTheMe(int type)
{
    yearschceduleview->setTheMe(type);
}

/**
 * @brief setDtype
 * @param type
 * @param arrowheight
 */
void CYearSchceduleOutView::setDtype(int type, int arrowheight)
{
    yearschceduleview->setDtype(type, arrowheight);
}

/**
 * @brief setCurrentDate 设置日程所在当天的日期
 * @param cdate 日期
 */
void CYearSchceduleOutView::setCurrentDate(QDate cdate)
{
    currentdate = cdate;
    yearschceduleview->setCurrentDate(cdate);
}

/**
 * @brief adjustPosition 根据日程浮框左右朝向不同，日程显示位置不同
 * @param ad 是否调整显示位置
 */
void CYearSchceduleOutView::adjustPosition(bool ad)
{
    yearschceduleview->adjustPosition(ad);
}

/**
 * @brief mousePressEvent 鼠标单击事件，单击非节日日程进行编辑，单击”...”区域跳转到周视图。
 * @param event 鼠标事件
 */
void CYearSchceduleOutView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    QPoint pos = QCursor::pos();
    //点击位置
    pos = this->mapFromGlobal(pos);
    //日程点击区域容器
    QVector<QRect> rect_press;
    QRect rect(35,50,width() - 50,20);
    //日程展示个数
    int listShow = 0;

    if (!scheduleinfoList.isEmpty()) {
        //日程信息不为空
        if (scheduleinfoList.size() < DDEYearCalendar::YearScheduleListMaxcount)
            //日程个数小于5,日程展示个数为具体日程个数
            listShow = scheduleinfoList.size();
        else
            //日程个数大于5,日程展示个数为5
            listShow = DDEYearCalendar::YearScheduleListMaxcount;
    }
    for (int i = 0; i < listShow; i++) {
        //根据日程个数，设置每个日程的点击区域
        if (this->arrowDirection() == DArrowRectangle::ArrowLeft)
            rect_press.append(QRect(35, 20 + i * 30, width() - 50, 20));
        else
            rect_press.append(QRect(20, 20 + i * 30, width() - 50, 20));
    }
    for (int i = 0; i < listShow; i++) {
        if (rect_press.at(i).contains(pos)) {
            if (i > 3 && list_count > DDEYearCalendar::YearScheduleListMaxcount) {
                emit signalsViewSelectDate(currentdate);
                this->hide();
                //跳转到周视图
            } else {
                if (scheduleinfoList.at(i).type.ID != DDECalendar::FestivalTypeID) {
                    //打开编辑日程页面
                    CSchceduleDlg dlg(0);
                    //设置日程信息
                    dlg.setData(scheduleinfoList.at(i));
                    if (dlg.exec() == DDialog::Accepted)
                        emit signalupdateschcedule();
                }
            }
        }
    }
}
