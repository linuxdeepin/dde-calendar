// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "yearscheduleview.h"
#include "commondef.h"

#include "scheduledlg.h"
#include "scheduledatamanage.h"
#include "constants.h"
#include "schedulemanager.h"
#include "cscheduleoperation.h"

#include <QPainter>
#include <QRect>
#include <QMouseEvent>

DGUI_USE_NAMESPACE

CYearScheduleView::CYearScheduleView(QWidget *parent)
    : DWidget(parent)
{
    qCDebug(ClientLogger) << "CYearScheduleView constructed";
    m_textfont.setWeight(QFont::Medium);
    m_textfont.setPixelSize(DDECalendar::FontSizeTwelve);
}

CYearScheduleView::~CYearScheduleView()
{
    qCDebug(ClientLogger) << "CYearScheduleView destroyed";
}

bool YScheduleDateThan(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2)
{
    // qCDebug(ClientLogger) << "Comparing schedule dates";
    QDate bDate1 = s1->dtStart().date();
    QDate eDate1 = s1->dtEnd().date();
    QDate bDate2 = s2->dtStart().date();
    QDate eDate2 = s2->dtEnd().date();

    if (bDate1 != eDate1 && bDate2 == eDate2) {
        return true;
    } else if (bDate1 == eDate1 && bDate2 != eDate2) {
        return false;
    } else if (bDate1 != eDate1 && bDate2 != eDate2) {
        return bDate1 < bDate2;
    } else {
        if (s1->dtStart() == s2->dtStart()) {
            return s1->summary() < s2->summary();
        } else {
            return s1->dtStart() < s2->dtStart();
        }
    }
}
bool YScheduleDaysThan(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2)
{
    // qCDebug(ClientLogger) << "Comparing schedule days";
    return s1->dtStart().date().daysTo(s1->dtEnd().date()) > s2->dtStart().date().daysTo(s2->dtEnd().date());
}

void CYearScheduleView::setData(DSchedule::List &vListData)
{
    qCDebug(ClientLogger) << "Setting data with" << vListData.size() << "schedules";
    DSchedule::List valldayListData, vDaylistdata;

    for (int i = 0; i < vListData.count(); i++) {
        DSchedule::Ptr ptr = vListData.at(i);
        if (ptr.isNull()) {
            continue;
        }
        if (ptr->allDay()) {
            valldayListData.append(ptr);
        } else {
            vDaylistdata.append(ptr);
        }
    }

    std::sort(valldayListData.begin(), valldayListData.end(), YScheduleDaysThan);
    std::sort(valldayListData.begin(), valldayListData.end(), YScheduleDateThan);
    std::sort(vDaylistdata.begin(), vDaylistdata.end(), YScheduleDaysThan);
    std::sort(vDaylistdata.begin(), vDaylistdata.end(), YScheduleDateThan);

    // Fixed: Use a safer approach to move festival schedules to the front
    // Collect festival schedules first, then reorganize the list
    DSchedule::List festivalSchedules;
    DSchedule::List nonFestivalSchedules;

    qCDebug(ClientLogger) << "Processing" << valldayListData.count() << "all-day schedules for festival reorganization";

    for (int i = 0; i < valldayListData.count(); i++) {
        DSchedule::Ptr schedule = valldayListData.at(i);
        if (CScheduleOperation::isFestival(schedule)) {
            //如果为节假日日程
            festivalSchedules.append(schedule);
            qCDebug(ClientLogger) << "Found festival schedule, i=" << i;
        } else {
            nonFestivalSchedules.append(schedule);
            qCDebug(ClientLogger) << "Found non-festival schedule, i=" << i;
        }
    }

    // Rebuild the list with festival schedules at the front
    valldayListData.clear();
    valldayListData.append(festivalSchedules);
    valldayListData.append(nonFestivalSchedules);

    qCDebug(ClientLogger) << "Reorganized schedules:" << festivalSchedules.count() << "festival schedules moved to front," << nonFestivalSchedules.count() << "regular schedules follow";

    m_vlistData.clear();
    m_vlistData.append(valldayListData);
    m_vlistData.append(vDaylistdata);

    if (m_vlistData.size() > DDEYearCalendar::YearScheduleListMaxcount) {
        qCDebug(ClientLogger) << "Schedule list exceeds max count, truncating to" << DDEYearCalendar::YearScheduleListMaxcount;
        DSchedule::List vTListData;
        for (int i = 0; i < 4; i++) {
            if (m_vlistData.at(i)->dtStart().date() != m_vlistData.at(i)->dtEnd().date() && !m_vlistData.at(i)->allDay()) {
                if (m_vlistData.at(i)->dtStart().date() != m_currentDate) {
                    m_vlistData[i]->setAllDay(true);
                }
            }
            vTListData.append(m_vlistData.at(i));
        }
        DSchedule::Ptr info;
        info.reset(new  DSchedule());
        info->setSummary("......");
        info->setUid("-1");
        vTListData.append(info);
        m_vlistData = vTListData;
    }
}

void CYearScheduleView::clearData()
{
    qCDebug(ClientLogger) << "Clearing schedule data";
    m_vlistData.clear();
}

void CYearScheduleView::showWindow()
{
    qCDebug(ClientLogger) << "Showing window with" << m_vlistData.size() << "schedules";
    if (m_vlistData.isEmpty()) {
        // qCDebug(ClientLogger) << "Setting fixed size to: 130x45";
        setFixedSize(130, 45);
    } else {
        // qCDebug(ClientLogger) << "Updating date show";
        updateDateShow();
    }
}

void CYearScheduleView::setTheMe(int type)
{
    qCDebug(ClientLogger) << "Setting theme to type:" << type;
    if (type == 0 || type == 1) {
        qCDebug(ClientLogger) << "Setting theme to light";
        m_btimecolor = "#414D68";
        m_btimecolor.setAlphaF(0.7);
        m_btTextColor = "#414D68";
    } else if (type == 2) {
        qCDebug(ClientLogger) << "Setting theme to dark";
        m_btimecolor = "#C0C6D4";
        m_btimecolor.setAlphaF(0.7);
        m_btTextColor = "#C0C6D4";
    }
}

void CYearScheduleView::setCurrentDate(const QDate &cdate)
{
    // qCDebug(ClientLogger) << "Setting current date to:" << cdate.toString();
    m_currentDate = cdate;
}

QDate CYearScheduleView::getCurrentDate()
{
    // qCDebug(ClientLogger) << "Getting current date";
    return m_currentDate;
}

void CYearScheduleView::setTimeFormat(const QString &format)
{
    // qCDebug(ClientLogger) << "Setting time format to:" << format;
    m_timeFormat = format;
    update();
}

int CYearScheduleView::getPressScheduleIndex()
{
    // qCDebug(ClientLogger) << "Getting press schedule index";
    int resutle = -1;
    //获取全局坐标
    QPoint currentPos = QCursor::pos();
    //转换为当前坐标
    currentPos = this->mapFromGlobal(currentPos);
    for (int i = 0; i < m_drawRect.size(); ++i) {
        //若坐标在绘制标签中则更新返回值
        if (m_drawRect.at(i).contains(currentPos)) {
            resutle = i;
            break;
        }
    }
    qCDebug(ClientLogger) << "Press schedule index:" << resutle;
    return resutle;
}

void CYearScheduleView::updateDateShow()
{
    qCDebug(ClientLogger) << "Updating date show";
    int sViewNum = 0;
    if (!m_vlistData.isEmpty()) {
        if (m_vlistData.size() > DDEYearCalendar::YearScheduleListMaxcount) {
            sViewNum = DDEYearCalendar::YearScheduleListMaxcount;
        } else {
            sViewNum = m_vlistData.size();
        }
    }
    if (!m_vlistData.isEmpty())
        setFixedSize(240, 45 + (sViewNum - 1) * 29);
    update();
    m_drawRect.clear();
    //对绘制标签矩阵进行缓存
    for (int i = 0; i < m_vlistData.size(); i++) {
        m_drawRect.append(QRect(30, 10 + i * 29, width() - 50, 26));
    }
    return;
}

void CYearScheduleView::paintEvent(QPaintEvent *event)
{
    // qCDebug(ClientLogger) << "Paint event received";
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    if (m_vlistData.isEmpty()) {
        // qCDebug(ClientLogger) << "Painting empty schedule";
        paintItem(painter);
    } else {
        // qCDebug(ClientLogger) << "Painting schedule";
        for (int i = 0; i < m_vlistData.size(); ++i) {
            paintItem(painter, m_vlistData.at(i), i);
        }
    }
}

void CYearScheduleView::paintItem(QPainter &painter, DSchedule::Ptr info, int index)
{
    // qCDebug(ClientLogger) << "Painting schedule item";
    int labelwidth = width() - 30;
    int bHeight = index * 29 + 10;
    int labelheight = 28;
    DSchedule::Ptr &gd = info;

    if (info->uid() == "-1") {
        // qCDebug(ClientLogger) << "Painting '...' item";
        QString str = "...";
        painter.save();
        painter.setPen(m_btimecolor);
        painter.setFont(m_textfont);
        painter.drawText(QRect(25, bHeight, labelwidth - 80, labelheight - 2), Qt::AlignLeft | Qt::AlignVCenter, str);
        painter.restore();
    } else {
        // qCDebug(ClientLogger) << "Painting schedule item";
        if (info->uid() != "-1") {
            //圆点m_solocolor
            QColor gdColor;
            DScheduleType::Ptr type = gScheduleManager->getScheduleTypeByScheduleId(info->scheduleTypeID());
            if (nullptr != type) {
                gdColor = type->getColorCode();
            }
            painter.save();
            painter.setBrush(QBrush(gdColor));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QRect(25, bHeight + (labelheight - 8) / 2, 8, 8));
            painter.restore();
        }

        QString str;
        //左边文字
        painter.save();
        painter.setPen(m_btTextColor);
        painter.setFont(m_textfont);
        QFontMetrics fm = painter.fontMetrics();
        QString tSTitleName = gd->summary();
        tSTitleName.replace("\n", "");
        str = tSTitleName;
        int tilenameW = labelwidth - 80;
        QString tStr;

        for (int i = 0; i < str.count(); i++) {
            tStr.append(str.at(i));
            int widthT = fm.horizontalAdvance(tStr) + 5;
            if (widthT >= tilenameW) {
                tStr.chop(1);
                break;
            }
        }
        if (tStr != str) {
            tStr = tStr + "...";
        }
        painter.drawText(QRect(41, bHeight, tilenameW, labelheight - 2), Qt::AlignLeft | Qt::AlignVCenter, tStr);

        painter.restore();

        if (info->uid() != "-1") {
            //右边时间
            painter.save();
            painter.setPen(m_btimecolor);
            painter.setFont(m_textfont);
            if (info->allDay()) {
                str = tr("All Day");
            } else {
                if (m_currentDate > info->dtStart().date()) {
                    str = tr("All Day");
                } else {
                    str = info->dtStart().time().toString(m_timeFormat);
                }
            }
            painter.drawText(QRect(width() - 70, bHeight, 57, labelheight - 2), Qt::AlignRight | Qt::AlignVCenter, str);
            painter.restore();
        }
    }
}

void CYearScheduleView::paintItem(QPainter &painter)
{
    // qCDebug(ClientLogger) << "Painting empty schedule";
    //左边文字
    painter.save();
    painter.setPen(m_btTextColor);
    painter.setFont(m_textfont);
    QString tSTitleName = tr("No event");
    painter.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, tSTitleName);
    painter.restore();
}

CYearScheduleOutView::CYearScheduleOutView(QWidget *parent)
    : DArrowRectangle(DArrowRectangle::ArrowLeft, DArrowRectangle::FloatWidget, parent)
{
    qCDebug(ClientLogger) << "CYearScheduleOutView constructed";
    //如果dtk版本为5.3以上则使用新接口
#if (DTK_VERSION > DTK_VERSION_CHECK(5, 3, 0, 0))
    //设置显示圆角
    setRadiusArrowStyleEnable(true);
    //设置圆角
    setRadius(DARROWRECT::DRADIUS);
#endif
    yearscheduleview = new CYearScheduleView();
    this->setContent(yearscheduleview);
}

void CYearScheduleOutView::setData(DSchedule::List &vListData)
{
    qCDebug(ClientLogger) << "Setting data with" << vListData.size() << "schedules in CYearScheduleOutView";
    list_count = vListData.size();
    yearscheduleview->setData(vListData);
    yearscheduleview->showWindow();
    //根据widget的高度调整圆角
    int value = yearscheduleview->height() / 6;
    //如果圆角最大为15
    value = value > 15 ? 15 : value;
    setRadius(value);
    scheduleinfoList = yearscheduleview->getlistdate();
}

void CYearScheduleOutView::clearData()
{
    qCDebug(ClientLogger) << "Clearing schedule data in CYearScheduleOutView";
    yearscheduleview->clearData();
}

void CYearScheduleOutView::setTheMe(int type)
{
    qCDebug(ClientLogger) << "Setting theme to type:" << type << "in CYearScheduleOutView";
    yearscheduleview->setTheMe(type);
    //根据主题设备不一样的背景色
    if (type == 2) {
        setBackgroundColor(DBlurEffectWidget::DarkColor);
    } else {
        setBackgroundColor(DBlurEffectWidget::LightColor);
    }
}

void CYearScheduleOutView::setCurrentDate(const QDate &cDate)
{
    qCDebug(ClientLogger) << "Setting current date to:" << cDate.toString() << "in CYearScheduleOutView";
    currentdate = cDate;
    yearscheduleview->setCurrentDate(cDate);
}

/**
 * @brief CYearScheduleOutView::setDirection       设置箭头方向
 * @param value
 */
void CYearScheduleOutView::setDirection(DArrowRectangle::ArrowDirection value)
{
    qCDebug(ClientLogger) << "Setting arrow direction to:" << value;
    //设置箭头方向
    this->setArrowDirection(value);
    //设置内容窗口
    this->setContent(yearscheduleview);
}

void CYearScheduleOutView::setTimeFormat(const QString &format)
{
    qCDebug(ClientLogger) << "Setting time format to:" << format << "in CYearScheduleOutView";
    yearscheduleview->setTimeFormat(format);
}

void CYearScheduleOutView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    //获取当前点击的标签编号
    int currentIndex = yearscheduleview->getPressScheduleIndex();
    // qCDebug(ClientLogger) << "Mouse press event in CYearScheduleOutView, index:" << currentIndex;

    if (currentIndex > -1 && currentIndex < scheduleinfoList.size()) {
        if (currentIndex > 3 && list_count > DDEYearCalendar::YearScheduleListMaxcount) {
            qCDebug(ClientLogger) << "Clicked on '...' item, switching to week view for date:" << currentdate.toString();
            emit signalsViewSelectDate(currentdate);
            this->hide();
            //跳转到周视图
        } else {
            //如果日程类型不为节假日或纪念日则显示编辑框
            if (!CScheduleOperation::isFestival(scheduleinfoList.at(currentIndex))) {
                qCDebug(ClientLogger) << "Opening schedule dialog for schedule:" << scheduleinfoList.at(currentIndex)->summary();
                //因为提示框会消失，所以设置CScheduleDlg的父类为主窗口
                CScheduleDlg dlg(0, qobject_cast<QWidget *>(this->parent()));
                dlg.setData(scheduleinfoList.at(currentIndex));
                dlg.exec();
            }
        }
    }
}
