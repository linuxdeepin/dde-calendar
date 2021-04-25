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
#include "monthschceduleview.h"
#include "SchecduleRemindWidget.h"
#include "monthview.h"
#include "schcedulectrldlg.h"
#include "schceduledlg.h"
#include "myschceduleview.h"

#include <DMessageBox>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DPalette>

#include <QAction>
#include <QListWidget>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>
#include <QShortcut>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QSequentialAnimationGroup>
#include <QDebug>

DGUI_USE_NAMESPACE
/**
 * @brief CMonthSchceduleWidgetItem 构造函数
 * @param rect 日程所在矩形
 * @param parent 父类
 * @param edittype unuse
 */
CMonthSchceduleWidgetItem::CMonthSchceduleWidgetItem(QRect rect, QGraphicsItem *parent, int edittype)
    : DragInfoItem(rect, parent)
    , m_pos(13, 5)
{
    Q_UNUSED(edittype);
}
/**
  * @brief ~CMonthSchceduleWidgetItem 析构函数
  */
CMonthSchceduleWidgetItem::~CMonthSchceduleWidgetItem()
{

}
/**
 * @brief getPixmap 获取日程的图像
 * @return 日程的图像
 */
QPixmap CMonthSchceduleWidgetItem::getPixmap()
{
    //设置图像
    QPixmap pixmap(this->rect().size().toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    //绘制图像
    paintBackground(&painter,pixmap.rect(),true);
    painter.end();

    return  pixmap;
}
/**
 * @brief paintBackground 绘制日程
 * @param painter 绘制函数
 * @param rect 日程所在矩形
 * @param isPixMap 是否为图像
 */
void CMonthSchceduleWidgetItem::paintBackground(QPainter *painter, const QRectF &rect, const int isPixMap)
{
    qreal labelwidth = rect.width();
    qreal labelheight = rect.height();
    //获取字体
    m_font = DFontSizeManager::instance()->get(m_sizeType, m_font);
    //系统主题
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();
    //根据日程类型获取颜色
    CSchedulesColor gdcolor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(m_vScheduleInfo.type.ID);
    //高亮标志
    m_vHighflag = CScheduleDataManage::getScheduleDataManage()->getSearchResult(m_vScheduleInfo);
    //渐变画笔
    QLinearGradient linearGradient(rect.topLeft().x(), 0, rect.topRight().x(), 0);
    //渐变开始颜色
    QColor color1 = gdcolor.gradientFromC;
    //渐变结束颜色
    QColor color2 = gdcolor.gradientToC;
    //文字颜色
    QColor textcolor = gdcolor.textColor;
    //选择日程
    if (CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo() == m_vScheduleInfo ) {
        if (m_vScheduleInfo.IsMoveInfo ==
                CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo().IsMoveInfo) {
            //高亮移动信息
            m_vHighflag = true;
        } else {
            painter->setOpacity(0.4);
            textcolor.setAlphaF(0.4);
        }
        //选择标志
        m_vSelectflag = m_press;
    }

    if (isPixMap) {
        //设置图像透明度
        painter->setOpacity(0.6);
        textcolor.setAlphaF(0.8);
    }

    if (m_vHoverflag) {
        //hover渐变开始颜色
        color1 = gdcolor.hovergradientFromC;
        //hover渐变结束颜色
        color2 = gdcolor.hovergradientToC;
    } else if (m_vHighflag) {
        //高亮渐变开始颜色
        color1 = gdcolor.hightlightgradientFromC;
        //高亮渐变结束颜色
        color2 = gdcolor.hightlightgradientToC;
    }
    if (m_vSelectflag) {
        //选中渐变开始颜色
        color1 = gdcolor.pressgradientFromC;
        //选中渐变结束颜色
        color2 = gdcolor.pressgradientToC;
        //设置颜色透明度
        textcolor.setAlphaF(0.4);
    }
    //设置开始渐变色
    linearGradient.setColorAt(0, color1);
    //设置结束渐变色
    linearGradient.setColorAt(1, color2);
    //矩形
    QRectF fillRect = QRectF(rect.x()+2,
                             rect.y()+2,
                             labelwidth - 2,
                             labelheight - 2);
    painter->save();
    //将直线开始点设为0，终点设为1，然后分段设置颜色
    painter->setBrush(linearGradient);
    painter->setPen(Qt::NoPen);
    //绘制矩形
    painter->drawRoundedRect(fillRect,
                             rect.height() / 3,
                             rect.height() / 3);
    painter->restore();
    //设置字体
    painter->setFont(m_font);
    //设置颜色
    painter->setPen(textcolor);
    QFontMetrics fm = painter->fontMetrics();
    //日程标题
    QString tStitlename = m_vScheduleInfo.titleName;
    tStitlename.replace("\n", "");
    QString str = tStitlename;
    qreal textWidth = labelwidth - m_pos.x() - m_offset*2;
    QString tstr;
    //分析字符串
    for (int i = 0; i < str.count(); i++) {
        tstr.append(str.at(i));
        int widthT = fm.width(tstr) + 5;
        if (widthT >= textWidth) {
            tstr.chop(2);
            break;
        }
    }
    //内容较长显示...
    if (tstr != str) {
        tstr = tstr + "...";
    }
    //绘制日程文字
    painter->drawText(QRectF(rect.x()+m_pos.x(),
                             rect.y()+1,
                             textWidth,
                             labelheight - m_pos.y() + 3  ),
                      Qt::AlignLeft | Qt::AlignVCenter, tstr);
    //高亮但不点击
    if (m_vHoverflag && !m_vSelectflag) {
        //矩形
        QRectF trect = QRectF(rect.x()+2.5, rect.y()+2.5, labelwidth - 3, labelheight - 3);
        painter->save();
        //抗锯齿
        painter->setRenderHints(QPainter::Antialiasing);
        QPen pen;
        QColor selcolor;
        //根据系统主题设置颜色
        if (themetype == 2) {
            selcolor = "#FFFFFF";
        } else {
            selcolor = "#000000";
        }
        //设置透明度
        selcolor.setAlphaF(0.08);
        //设置画笔颜色
        pen.setColor(selcolor);
        pen.setWidthF(1);
        pen.setStyle(Qt::SolidLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);
        painter->drawRoundedRect(trect, rect.height() / 3, rect.height() / 3);
        painter->restore();
    }
    //点击选择
    if (m_vSelectflag) {
        QColor selcolor = "#000000";
        selcolor.setAlphaF(0.05);
        painter->setBrush(selcolor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(fillRect, rect.height() / 3, rect.height() / 3);
    }
}
/**
 * @brief CMonthSchceduleNumButton 构造函数
 * @param parent 父类
 */
CMonthSchceduleNumButton::CMonthSchceduleNumButton(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{

}
/**
 * @brief ~CMonthSchceduleNumButton 析构函数
 */
CMonthSchceduleNumButton::~CMonthSchceduleNumButton()
{

}
/**
 * @brief setColor 设置颜色
 * @param color1 第一种颜色
 * @param color2 第二种颜色
 * @param GradientFlag 是否设置渐变色
 */
void CMonthSchceduleNumButton::setColor(QColor color1, QColor color2, bool GradientFlag)
{
    m_color1 = color1;
    m_color2 = color2;
    m_GradientFlag = GradientFlag;
}
/**
 * @brief setText 字体设置
 * @param tcolor 字体颜色
 * @param font 字体
 * @param pos unuse
 */
void CMonthSchceduleNumButton::setText( QColor tcolor, QFont font, QPoint pos)
{
    Q_UNUSED(pos);
    m_textcolor = tcolor;
    m_font = font;
}
/**
 * @brief setSizeType 设置字体类型大小
 * @param sizeType 字体类型大小
 */
void CMonthSchceduleNumButton::setSizeType(DFontSizeManager::SizeType sizeType)
{
    m_SizeType = sizeType;
}
/**
 * @brief setTransparentB
 * @param t
 * @param tcolor
 */
void CMonthSchceduleNumButton::setTransparentB(bool t, QColor tcolor)
{
    m_transparentcolor = tcolor;
    m_transparentf = t;
}
/**
 * @brief paint 绘制“还有n项”
 * @param painter 绘制函数
 * @param option
 * @param widget
 */
void CMonthSchceduleNumButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    qreal labelwidth = this->rect().width();
    qreal labelheight = this->rect().height() - 6;
    qreal rectx = this->rect().x();
    qreal recty = this->rect().y();
    //字体
    m_font = DFontSizeManager::instance()->get(m_SizeType, m_font);
    //设置渐变色
    if (m_GradientFlag) {
        QLinearGradient linearGradient(0, 0, labelwidth, 0);
        //设置渐变色1
        linearGradient.setColorAt(0, m_color1);
        //设置渐变色2
        linearGradient.setColorAt(1, m_color2);
        //矩形
        QRectF fillRect = QRectF(rectx+2, recty+1, labelwidth - 2, labelheight - 1);
        //将直线开始点设为0，终点设为1，然后分段设置颜色
        painter->setRenderHints(QPainter::HighQualityAntialiasing);
        //设置画刷为渐变色
        painter->setBrush(linearGradient);
        painter->setPen(Qt::NoPen);
        //绘制圆角矩形
        painter->drawRoundedRect(fillRect, 3, 2);
        painter->setFont(m_font);
        painter->setPen(m_textcolor);
        //文字
        QString str =  QString(tr("%1 more")).arg(m_num) + "...";
        QFont tfont = m_font;
        QFontMetrics fm = painter->fontMetrics();
        QString tstr;
        //调整文字显示
        for (int i = 0; i < str.count(); i++) {
            tstr.append(str.at(i));
            int widthT = fm.width(tstr) + 5;
            if (widthT >= labelwidth - m_pos.x()) {
                tstr.chop(2);
                break;
            }
        }
        //文字过长显示...
        if (tstr != str) {
            tstr = tstr + "...";
        }
        //绘制文字
        painter->drawText(QRectF(rectx+m_pos.x(), recty+m_pos.y(), labelwidth - m_pos.x(), labelheight + 4), Qt::AlignCenter, tstr);
    } else {
        //矩形
        QRectF fillRect = QRectF(rectx+2, recty+1, labelwidth - 2, labelheight - 1);
        //将直线开始点设为0，终点设为1，然后分段设置颜色
        painter->setRenderHints(QPainter::HighQualityAntialiasing);
        //设置画刷
        painter->setBrush(m_color1);
        painter->setPen(Qt::NoPen);
        //绘制圆角矩形
        painter->drawRoundedRect(fillRect, 3, 2);
        //设置字体
        painter->setFont(m_font);
        painter->setPen(m_textcolor);
        //绘制文字
        painter->drawText(QRectF(rectx+m_pos.x(),
                                 recty+m_pos.y(),
                                 labelwidth - m_pos.x(),
                                 labelheight + 4),
                          Qt::AlignLeft,
                          QString(tr("%1 more")).arg(m_num));
    }
}
/**
 * @brief setTheMe 根据系统类型设置颜色
 * @param type 系统类型
 */
void CMonthSchceduleView::setTheMe(int type)
{
    Q_UNUSED(type);
    updateData();
}
/**
 * @brief CMonthSchceduleView 构造函数
 * @param parent 父类
 * @param scene 场景
 */
CMonthSchceduleView::CMonthSchceduleView(QWidget *parent, QGraphicsScene *scene)
    : QObject(parent)
    , m_Scene(scene)
{
    //每月6行，代表6周
    for (int i = 0; i < 6; ++i) {
        CWeekScheduleView *weekSchedule = new CWeekScheduleView(this);
        m_weekSchedule.append(weekSchedule);
    }
    //字体大小改变
    slotFontChange();
}
/**
  * @brief ~CMonthSchceduleView 析构函数
  */
CMonthSchceduleView::~CMonthSchceduleView()
{

}
/**
 * @brief setallsize 设置日程周为边距，和日程高度
 * @param w 视图的宽度
 * @param h 视图的高度
 * @param left 日程的左边距
 * @param top 日程的上边距
 * @param buttom 日程的下边距
 * @param itemHeight 日程的高度
 */
void CMonthSchceduleView::setallsize(int w, int h, int left, int top, int buttom, int itemHeight)
{
    m_width = w;
    m_height = h;
    m_buttommagin = buttom;
    m_leftMagin = left;
    m_topMagin = top;
    //最多可以显示日程数量
    m_cNum = static_cast<int>(((m_height - m_topMagin - m_buttommagin) / 6.0 + 0.5  - 27) / (itemHeight + 1));
}
/**
 * @brief setData 设置日期
 * @param data 日期
 * @param currentMonth 当前月份
 */
void CMonthSchceduleView::setData(QVector<ScheduleDateRangeInfo> &data, int currentMonth)
{
    m_data = data;
    m_currentMonth = currentMonth;
    updateData();
}
/**
 * @brief slotdeleteitem 发送更新日程的信号
 * @param item 日程item
 */
void CMonthSchceduleView::slotdeleteitem( CMonthSchceduleWidgetItem *item)
{
    emit signalsUpdateShcedule(item->getData().id);
}
/**
 * @brief slotedititem 发送更新日程的信号
 * @param item 日程
 * @param type
 */
void CMonthSchceduleView::slotedititem(CMonthSchceduleWidgetItem *item, int type)
{
    Q_UNUSED(type);
    emit signalsUpdateShcedule(item->getData().id);
}
/**
 * @brief slotFontChange 字体大小改变，更新显示
 */
void CMonthSchceduleView::slotFontChange()
{
    QFont font;
    DFontSizeManager::instance()->setFontGenericPixelSize(
        static_cast<quint16>(DFontSizeManager::instance()->fontPixelSize(qGuiApp->font())));
    font= DFontSizeManager::instance()->t8(font);
    QFontMetrics fm(font);
    int h = fm.height();

    if (m_ItemHeight != h) {
        m_ItemHeight = h;
        updateData();
    }
}
/**
 * @brief MScheduleDateThan 日程开始时间的比较
 * @param s1 第一个日程
 * @param s2 第二个日程
 * @return bool
 */
bool MScheduleDateThan(const MScheduleDateRangeInfo &s1, const MScheduleDateRangeInfo &s2)
{
    if (s1.bdate != s1.edate && s2.bdate == s2.edate) {
        return true;
    } else if (s1.bdate == s1.edate && s2.bdate != s2.edate) {
        return false;
    } else if (s1.bdate != s1.edate && s2.bdate != s2.edate) {
        if (s1.tData.beginDateTime == s2.tData.beginDateTime) {
            return s1.bdate.daysTo(s1.edate) > s2.bdate.daysTo(s2.edate);
        } else {
            return s1.bdate < s2.bdate;
        }
    } else {
        if (s1.tData.beginDateTime == s2.tData.beginDateTime) {
            return s1.tData.titleName < s2.tData.titleName;
        } else {
            return s1.tData.beginDateTime < s2.tData.beginDateTime;
        }
    }
}
/**
 * @brief MScheduleDaysThan 日程跨越天数的比较
 * @param s1 第一个日程
 * @param s2 第二个日程
 * @return bool
 */
bool MScheduleDaysThan(const MScheduleDateRangeInfo &s1, const MScheduleDateRangeInfo &s2)
{
    return s1.bdate.daysTo(s1.edate) > s2.bdate.daysTo(s2.edate);
}
/**
 * @brief updateData 更新数据
 */
void CMonthSchceduleView::updateData()
{
    //保护数据防止越界
    if (m_data.count() != DDEMonthCalendar::ItemSizeofMonthDay || m_cNum < 1)
        return;
    //开始时间
    QDate begindate = m_data.begin()->date;
    //结束时间
    QDate enddate = m_data[m_data.count() - 1].date;
    m_beginDate = begindate;
    m_endDate  = enddate;
    //设置每周的数据
    for (int i = 0 ; i < m_weekSchedule.size(); ++i) {
        m_weekSchedule[i]->clearItem();
        //设置高度
        m_weekSchedule[i]->setHeight(m_ItemHeight,qRound((m_height - m_topMagin - m_buttommagin) / 6.0-27));
        //设置日期
        m_weekSchedule[i]->setData(m_data,i*7,7);
        //日程信息
        QVector<QVector<MScheduleDateRangeInfo> > mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        //展示日程
        updateDateShow(mSchedule,m_weekSchedule[i]->getScheduleShowItem());
    }
    return;
}
/**
 * @brief updateHigh 更新数据
 */
void CMonthSchceduleView::updateHigh()
{
    for (int j = 0 ; j < m_weekSchedule.size(); ++j) {
        for (int i = 0; i < m_weekSchedule[j]->getScheduleShowItem().count(); i++) {
            m_weekSchedule[j]->getScheduleShowItem().at(i)->update();
        }
    }
}
/**
 * @brief getScheduleShowItem 获取展示的日程item
 * @return
 */
QVector<QGraphicsRectItem *> CMonthSchceduleView::getScheduleShowItem() const
{
    QVector<QGraphicsRectItem *>   m_scheduleShowItem;
    //将展示的日程item添加到容器
    for (int j = 0 ; j < m_weekSchedule.size(); ++j) {
        for (int i = 0; i < m_weekSchedule[j]->getScheduleShowItem().count(); i++) {
            m_scheduleShowItem.append(m_weekSchedule[j]->getScheduleShowItem().at(i));
        }
    }

    return m_scheduleShowItem;
}
/**
 * @brief updateDate 更新数据
 * @param info 日程信息
 */
void CMonthSchceduleView::updateDate(const ScheduleDtailInfo &info)
{
    for (int i = 0 ; i < m_weekSchedule.size(); ++i) {
        if (m_weekSchedule.at(i)->addData(info)) {
        } else {
            m_weekSchedule[i]->clearItem();
            m_weekSchedule[i]->updateSchedule(true);
        };
        //获取日程信息
        QVector<QVector<MScheduleDateRangeInfo> > mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        //展示日程信息
        updateDateShow(mSchedule,m_weekSchedule[i]->getScheduleShowItem());
    }
}
/**
 * @brief changeDate 更改数据
 * @param info 日程信息
 */
void CMonthSchceduleView::changeDate(const ScheduleDtailInfo &info)
{
    for (int i = 0 ; i < m_weekSchedule.size(); ++i) {
        m_weekSchedule.at(i)->changeDate(info);
        //获取日程信息
        QVector<QVector<MScheduleDateRangeInfo> > mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        //展示日程信息
        updateDateShow(mSchedule,m_weekSchedule[i]->getScheduleShowItem());
    }
}
/**
 * @brief updateDate 更新数据
 * @param row 日程在一个月中第几行
 * @param info 日程信息
 */
void CMonthSchceduleView::updateDate(const int row, const ScheduleDtailInfo &info)
{
    for (int i = 0 ; i < m_weekSchedule.size(); ++i) {
        if ( row ==i) {
            m_weekSchedule.at(i)->addData(info);
        } else {
            m_weekSchedule[i]->clearItem();
            m_weekSchedule[i]->updateSchedule(true);
        };
        //获取日程信息
        QVector<QVector<MScheduleDateRangeInfo> > mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        //展示日程信息
        updateDateShow(mSchedule,m_weekSchedule[i]->getScheduleShowItem());
    }
}
/**
 * @brief updateDateShow 更新数据展示
 * @param vCMDaySchedule 日程信息
 * @param schudeleShowItem 日程item
 */
void CMonthSchceduleView::updateDateShow(QVector<QVector<MScheduleDateRangeInfo> > &vCMDaySchedule, QVector<QGraphicsRectItem *> &schudeleShowItem)
{
    for (int i = 0; i < vCMDaySchedule.count(); i++) {
        for (int j = 0; j < vCMDaySchedule[i].count(); j++) {
            if (vCMDaySchedule[i].at(j).state) {
                //创建还有n项item
                createScheduleNumWidget(vCMDaySchedule[i].at(j), i + 1,schudeleShowItem);
            } else {
                //创建日程item
                createScheduleItemWidget(vCMDaySchedule[i].at(j), i + 1,schudeleShowItem);
            }
        }
    }
}
/**
 * @brief createScheduleItemWidget 新建日程item
 * @param info 日程信息
 * @param cnum 日程个数
 * @param schudeleShowItem 日程展示的item
 */
void CMonthSchceduleView::createScheduleItemWidget(MScheduleDateRangeInfo info, int cnum,QVector<QGraphicsRectItem *> &schudeleShowItem)
{
    ScheduleDtailInfo gd = info.tData;
    QPoint pos;
    int fw;
    int fh;
    //设置日程宽度高度和坐标
    computePos(cnum, info.bdate, info.edate, pos, fw, fh);
    //日程item
    CMonthSchceduleWidgetItem *gwi = new CMonthSchceduleWidgetItem(QRect(pos.x(),pos.y(),fw,fh),nullptr);
    //将item添加到场景中
    m_Scene->addItem(gwi);
    //字体
    QFont font;
    //设置日程信息
    gwi->setData(gd);
    //颜色
    QColor TransparentC = "#000000";
    //透明度
    TransparentC.setAlphaF(0.05);
    schudeleShowItem.append(gwi);
}
/**
 * @brief createScheduleNumWidget 新建“还有n项”item
 * @param info 日程信息
 * @param cnum 日程个数
 * @param schudeleShowItem 展示的item
 */
void CMonthSchceduleView::createScheduleNumWidget(MScheduleDateRangeInfo info, int cnum,QVector<QGraphicsRectItem *> &schudeleShowItem)
{
    int type = CScheduleDataManage::getScheduleDataManage()->getTheme();
    CMonthSchceduleNumButton *gwi = new CMonthSchceduleNumButton(nullptr);
    QPoint pos;
    int fw;
    int fh;
    //设置宽度高度和坐标
    computePos(cnum, info.bdate, info.edate, pos, fw, fh);
    //颜色
    QColor gradientFromC = "#000000";
    //透明度
    gradientFromC.setAlphaF(0.00);
    //设置颜色
    gwi->setColor(gradientFromC, gradientFromC, true);
    QFont font;
    gwi->setSizeType(DFontSizeManager::T8);
    //根据系统主题色绘制颜色
    if (type == 0 || type == 1) {
        //颜色
        QColor tc("#5E5E5E");
        //透明度
        tc.setAlphaF(0.9);
        //设置文字
        gwi->setText(tc, font, QPoint(13, 3));
    } else {
        QColor tc("#798190");
        tc.setAlphaF(1);
        gwi->setText(tc, font, QPoint(13, 3));
    }
    m_Scene->addItem(gwi);
    //设置矩形
    gwi->setRect(pos.x(),
                 pos.y(),
                 fw,
                 fh);
    gwi->setData(info.num);
    gwi->setDate(info.bdate);

    if (m_currentMonth != info.bdate.month() && m_currentMonth != info.edate.month()) {
        QColor TransparentC = "#000000";
        TransparentC.setAlphaF(0.05);
        gwi->setTransparentB(true, TransparentC);
    }
    connect(gwi, &CMonthSchceduleNumButton::signalsCurrentScheduleDate, this, &CMonthSchceduleView::signalsCurrentScheduleDate);
    connect(gwi, &CMonthSchceduleNumButton::signalPressScheduleShow, this, &CMonthSchceduleView::signalPressScheduleShow);

    schudeleShowItem.append(gwi);
}
/**
 * @brief computePos 设置item
 * @param cnum 日程个数
 * @param bgeindate 日程开始时间
 * @param enddate 日程结束时间
 * @param pos 日程的位置
 * @param fw 日程宽度
 * @param fh 日程高度
 */
void CMonthSchceduleView::computePos(int cnum, QDate bgeindate, QDate enddate, QPoint &pos, int &fw, int &fh)
{
    int brow = static_cast<int>((m_beginDate.daysTo(bgeindate)) / DDEMonthCalendar::AFewDaysofWeek);
    int bcol = (m_beginDate.daysTo(bgeindate)) % DDEMonthCalendar::AFewDaysofWeek;
    int ecol = (m_beginDate.daysTo(enddate)) % DDEMonthCalendar::AFewDaysofWeek;
    //设置日程宽度
    fw = static_cast<int>((ecol - bcol + 1) * ((m_width - m_leftMagin ) / 7.0) - 11);
    //设置日程高度
    fh = m_ItemHeight;
    int x =static_cast<int>(m_leftMagin + bcol * ((m_width - m_leftMagin )  / 7.0) + 5);
    int y = static_cast<int>(m_topMagin + ((m_height - m_topMagin - m_buttommagin) * brow / 6.0 + 0.5)  + 27 + (cnum - 1) * fh + 2.9);
    //设置日程坐标
    pos = QPoint(x, y);
}
/**
 * @brief CWeekScheduleView 构造函数
 * @param parent 父类
 */
CWeekScheduleView::CWeekScheduleView(QObject *parent)
    : QObject(parent)
    , m_ScheduleHeight(22)
    , m_DayHeight(47)
{
    setMaxNum();
}
/**
 * @brief ~CWeekScheduleView 析构函数
 */
CWeekScheduleView::~CWeekScheduleView()
{

}
/**
 * @brief setData 设置数据
 * @param data 日程信息
 * @param position 日程开始位置
 * @param count 设置日程个数
 */
void CWeekScheduleView::setData(QVector<ScheduleDateRangeInfo> &data, const int position, const int count)
{
    int endPos = position+count;
    Q_ASSERT(!(endPos>data.size()));
    m_ScheduleInfo.clear();

    for (int i = position; i<endPos; ++i) {
        for (int j = 0 ; j < data.at(i).vData.size(); ++j) {
            if (!m_ScheduleInfo.contains(data.at(i).vData.at(j))) {
                m_ScheduleInfo.append(data.at(i).vData.at(j));
            }
        }
    }
    beginDate  = data.at(position).date;
    endDate = data.at(position+count -1).date;
    m_colum = count;
    updateSchedule(true);
}
/**
 * @brief addData 添加日程
 * @param info 日程信息
 * @return 是否添加日程
 */
bool CWeekScheduleView::addData(const ScheduleDtailInfo &info)
{
    if (info.beginDateTime.date().daysTo(endDate)>=0 &&beginDate.daysTo(info.endDateTime.date())>=0) {
        clearItem();
        updateSchedule(false,info);
        return  true;
    }

    return false;
}
/**
 * @brief changeDate 修改数据
 * @param info 日程信息
 */
void CWeekScheduleView::changeDate(const ScheduleDtailInfo &info)
{
    int index = m_ScheduleInfo.indexOf(info);

    if (index <0) {
        m_ScheduleInfo.append(info);
    } else {
        m_ScheduleInfo[index] = info;
    }
    clearItem();
    updateSchedule(true);
}
/**
 * @brief setHeight 设置高度
 * @param ScheduleHeight 日程高度
 * @param DayHeigth 展示日程区域高度
 */
void CWeekScheduleView::setHeight(const int ScheduleHeight, const int DayHeigth)
{
    m_ScheduleHeight = ScheduleHeight;
    m_DayHeight = DayHeigth;
    setMaxNum();
}
/**
 * @brief updateSchedule 更新日程
 * @param isNormalDisplay 是否为正常展示
 * @param info 日程信息
 */
void CWeekScheduleView::updateSchedule(const bool isNormalDisplay, const ScheduleDtailInfo &info)
{
    QVector<ScheduleDtailInfo> schedulev;
    schedulev.clear();
    schedulev = m_ScheduleInfo;

    if (isNormalDisplay) {
        Q_UNUSED(info);
    } else {
        schedulev.append(info);
    }
    QDate   tbegindate,tenddate;
    QVector<MScheduleDateRangeInfo> vMDaySchedule;
    m_ColumnScheduleCount.clear();
    m_ColumnScheduleCount.fill(0,m_colum);

    for (int i = 0 ; i < schedulev.size(); ++i) {
        //日程时间重新标定
        tbegindate = schedulev.at(i).beginDateTime.date();
        tenddate = schedulev.at(i).endDateTime.date();

        if (tenddate<beginDate ||tbegindate>endDate)
            continue;
        if (tbegindate <  beginDate) tbegindate = beginDate;
        if (tenddate > endDate) tenddate = endDate;
        //日程信息
        MScheduleDateRangeInfo info;
        info.bdate = tbegindate;
        info.edate = tenddate;
        info.tData = schedulev.at(i);
        info.state = false;
        vMDaySchedule.append(info);
        qint64 pos = beginDate.daysTo(info.bdate);
        qint64 count = info.bdate.daysTo(info.edate);
        int j = static_cast<int>(pos);

        for (; j < (pos+ count+1); ++j) {
            ++m_ColumnScheduleCount[j];
        }
    }
    std::sort(vMDaySchedule.begin(),vMDaySchedule.end());
    sortAndFilter(vMDaySchedule);
}
/**
 * @brief clearItem 清空m_scheduleShowItem
 */
void CWeekScheduleView::clearItem()
{
    for (int i = 0; i < m_scheduleShowItem.count(); i++) {

        delete m_scheduleShowItem[i];
    }
    m_scheduleShowItem.clear();
}
/**
 * @brief setMaxNum 设置可以展示的最多日程
 */
void CWeekScheduleView::setMaxNum()
{
    m_MaxNum = m_DayHeight/(m_ScheduleHeight+1);
}
/**
 * @brief mScheduleClear 清空日程
 */
void CWeekScheduleView::mScheduleClear()
{
    for (int i = 0; i < m_MScheduleInfo.size(); ++i) {
        m_MScheduleInfo[i].clear();
    }
    m_MScheduleInfo.clear();
}
/**
 * @brief sortAndFilter 对日程排序并过滤
 * @param vMDaySchedule 日程
 */
void CWeekScheduleView::sortAndFilter(QVector<MScheduleDateRangeInfo> &vMDaySchedule)
{
    QVector<QVector<bool> > scheduleFill;
    QVector<bool> scheduf;
    //初始化
    //m_colum列
    scheduf.fill(false,m_colum);
    //m_MaxNum 行
    scheduleFill.fill(scheduf,m_MaxNum);
    //标签起始位置
    int postion = 0;
    //标签结束位置
    int end = 0;
    mScheduleClear();
    for (int i = 0 ; i < vMDaySchedule.size(); ++i) {
        postion = static_cast<int>(beginDate.daysTo(vMDaySchedule.at(i).bdate));
        end     = static_cast<int>(beginDate.daysTo(vMDaySchedule.at(i).edate));
        //初始化当前行
        int row = 0;
        int pos = postion;
        int count = 0;
        int scheduleRow = row;

        for (; postion<end+1; ++postion) {
            //如果当前行等于最大显示行
            if (row == m_MaxNum) {
                //初始化当前行
                row =0;
                //初始化当前位置
                pos = postion;
            }
            while (row<m_MaxNum) {
                if (m_MScheduleInfo.size()<(row+1)) {
                    RowScheduleInfo ms;
                    m_MScheduleInfo.append(ms);
                }
                //如果该位置没有被占用
                if (!scheduleFill[row][postion]) {
                    //如果该列日程总数大于最大显m_MScheduleInfo示数且该显示行没有超过最大显示行
                    if ((m_ColumnScheduleCount[postion]>m_MaxNum) &&(row>=m_MaxNum-1)) {
                        //占用该位置
                        scheduleFill[row][postion] = true;
                        //如果该位置不为起始位置
                        if (pos !=postion) {
                            MScheduleDateRangeInfo scheduleInfo;
                            scheduleInfo.bdate = beginDate.addDays(pos);
                            scheduleInfo.edate = beginDate.addDays(postion -1);
                            scheduleInfo.state = false;
                            scheduleInfo.tData = vMDaySchedule.at(i).tData;
                            m_MScheduleInfo[row].append(scheduleInfo);
                        }
                        //设置还有xxx项
                        MScheduleDateRangeInfo info;
                        info.bdate = beginDate.addDays(postion);
                        info.edate = info.bdate;
                        info.num = m_ColumnScheduleCount[postion] -m_MaxNum +1;
                        info.state = true;
                        m_MScheduleInfo[row].append(info);
                        pos = postion +1;
                        row = 0;
                        count = 0;
                    } else {
                        scheduleFill[row][postion] = true;
                        ++count;
                        scheduleRow = row;
                    }
                    break;
                } else {
                    ++row;
                }
            }
        }
        if (pos>6||count==0) {
        } else {
            MScheduleDateRangeInfo scheduleInfo;
            scheduleInfo.bdate = beginDate.addDays(pos);
            scheduleInfo.edate = beginDate.addDays(postion -1);
            scheduleInfo.state = false;
            scheduleInfo.tData = vMDaySchedule.at(i).tData;
            m_MScheduleInfo[scheduleRow].append(scheduleInfo);
        }
    }
}
