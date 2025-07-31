// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthscheduleview.h"
#include "monthview.h"
#include "schedulectrldlg.h"
#include "scheduledlg.h"
#include "myscheduleview.h"
#include "graphicsItem/cmonthschedulenumitem.h"
#include "commondef.h"



#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>
#include <QPropertyAnimation>
#include <QDebug>

DGUI_USE_NAMESPACE

const int schedule_Item_Y = 31; //日程item的相对于该天的高度差

CMonthScheduleView::CMonthScheduleView(QWidget *parent, QGraphicsScene *scene)
    : QObject(parent)
    , m_Scene(scene)
{
    qCDebug(ClientLogger) << "CMonthScheduleView::CMonthScheduleView";
    for (int i = 0; i < 6; ++i) {
        CWeekScheduleView *weekSchedule = new CWeekScheduleView(this);
        m_weekSchedule.append(weekSchedule);
    }
    slotFontChange();
}

CMonthScheduleView::~CMonthScheduleView()
{
    qCDebug(ClientLogger) << "CMonthScheduleView::~CMonthScheduleView";
}

void CMonthScheduleView::setallsize(int w, int h, int left, int top, int buttom, int itemHeight)
{
    qCDebug(ClientLogger) << "CMonthScheduleView::setallsize, width:" << w << "height:" << h;
    m_width = w;
    m_height = h;
    m_bottomMargin = buttom;
    m_leftMargin = left;
    m_topMargin = top;
    m_cNum = static_cast<int>(((m_height - m_topMargin - m_bottomMargin) / 6.0 + 0.5 - schedule_Item_Y) / (itemHeight + 1));
}

void CMonthScheduleView::setData(QMap<QDate, DSchedule::List> &data, int currentMonth)
{
    qCDebug(ClientLogger) << "CMonthScheduleView::setData, data count:" << data.count() << "currentMonth:" << currentMonth;
    m_data = data;
    m_currentMonth = currentMonth;
    updateData();
}

void CMonthScheduleView::slotFontChange()
{
    qCDebug(ClientLogger) << "CMonthScheduleView::slotFontChange";
    QFont font;
    DFontSizeManager::instance()->setFontGenericPixelSize(
        static_cast<quint16>(DFontSizeManager::instance()->fontPixelSize(qGuiApp->font())));
    font = DFontSizeManager::instance()->t8(font);
    QFontMetrics fm(font);
    int h = fm.height();

    if (m_ItemHeight != h) {
        qCDebug(ClientLogger) << "Font height changed, updating data";
        m_ItemHeight = h + 1;
        updateData();
    }
}

void CMonthScheduleView::slotStateChange(bool bState)
{
    qCDebug(ClientLogger) << "CMonthScheduleView::slotStateChange, state:" << bState;
    if(bState) {
        qCDebug(ClientLogger) << "Hiding schedule items";
        //日程显示
        for (int i = 0; i < m_weekSchedule.size(); ++i) {
            m_weekSchedule[i]->hideItem();
        }
    }

}

/**
 * @brief CMonthScheduleView::updateData        更新日程数据
 */
void CMonthScheduleView::updateData()
{
    qCDebug(ClientLogger) << "CMonthScheduleView::updateData";
    //清空日程显示
    for (int i = 0; i < m_weekSchedule.size(); ++i) {
        m_weekSchedule[i]->clearItem();
    }
    //保护数据防止越界
    if (m_data.count() != DDEMonthCalendar::ItemSizeOfMonthDay || m_cNum < 1) {
        qCWarning(ClientLogger) << "Data count or cNum is invalid, returning";
        return;
    }
    //开始结束时间
    QMap<QDate, DSchedule::List>::iterator _iter = m_data.begin();
    QDate begindate = _iter.key();
    _iter += (m_data.size() - 1);
    QDate enddate = _iter.key();
    m_beginDate = begindate;
    m_endDate = enddate;
    for (int i = 0; i < m_weekSchedule.size(); ++i) {
        m_weekSchedule[i]->setHeight(m_ItemHeight, qRound((m_height - m_topMargin - m_bottomMargin) / 6.0 - schedule_Item_Y));
        m_weekSchedule[i]->setData(m_data, begindate.addDays(i * 7), begindate.addDays(i * 7 + 6));
        QVector<QVector<MScheduleDateRangeInfo>> mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        updateDateShow(mSchedule, m_weekSchedule[i]->getScheduleShowItem());
    }
}

void CMonthScheduleView::updateHeight()
{
    qCDebug(ClientLogger) << "CMonthScheduleView::updateHeight";
    for (int j = 0; j < m_weekSchedule.size(); ++j) {
        for (int i = 0; i < m_weekSchedule[j]->getScheduleShowItem().count(); i++) {
            m_weekSchedule[j]->getScheduleShowItem().at(i)->update();
        }
    }
}

QVector<QGraphicsRectItem *> CMonthScheduleView::getScheduleShowItem() const
{
    qCDebug(ClientLogger) << "CMonthScheduleView::getScheduleShowItem";
    QVector<QGraphicsRectItem *> m_scheduleShowItem;

    for (int j = 0; j < m_weekSchedule.size(); ++j) {
        for (int i = 0; i < m_weekSchedule[j]->getScheduleShowItem().count(); i++) {
            m_scheduleShowItem.append(m_weekSchedule[j]->getScheduleShowItem().at(i));
        }
    }

    return m_scheduleShowItem;
}

void CMonthScheduleView::updateDate(const DSchedule::Ptr &info)
{
    qCDebug(ClientLogger) << "CMonthScheduleView::updateDate";
    for (int i = 0; i < m_weekSchedule.size(); ++i) {
        if (m_weekSchedule.at(i)->addData(info)) {
            qCDebug(ClientLogger) << "Added data to week" << i;
        } else {
            qCDebug(ClientLogger) << "Failed to add data to week" << i << ", clearing and updating";
            m_weekSchedule[i]->clearItem();
            m_weekSchedule[i]->updateSchedule(true);
        };
        QVector<QVector<MScheduleDateRangeInfo>> mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        updateDateShow(mSchedule, m_weekSchedule[i]->getScheduleShowItem());
    }
}

void CMonthScheduleView::changeDate(const DSchedule::Ptr &info)
{
    qCDebug(ClientLogger) << "CMonthScheduleView::changeDate";
    for (int i = 0; i < m_weekSchedule.size(); ++i) {
        m_weekSchedule.at(i)->changeDate(info);
        QVector<QVector<MScheduleDateRangeInfo>> mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        updateDateShow(mSchedule, m_weekSchedule[i]->getScheduleShowItem());
    }
}

void CMonthScheduleView::updateDate(const int row, const DSchedule::Ptr &info)
{
    qCDebug(ClientLogger) << "CMonthScheduleView::updateDate, row:" << row;
    for (int i = 0; i < m_weekSchedule.size(); ++i) {
        if (row == i) {
            m_weekSchedule.at(i)->addData(info);
        } else {
            m_weekSchedule[i]->clearItem();
            m_weekSchedule[i]->updateSchedule(true);
        };
        QVector<QVector<MScheduleDateRangeInfo>> mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        updateDateShow(mSchedule, m_weekSchedule[i]->getScheduleShowItem());
    }
}
void CMonthScheduleView::updateDateShow(QVector<QVector<MScheduleDateRangeInfo>> &vCMDaySchedule, QVector<QGraphicsRectItem *> &scheduleShowItem)
{
    // qCDebug(ClientLogger) << "CMonthScheduleView::updateDateShow";
    for (int i = 0; i < vCMDaySchedule.count(); i++) {
        for (int j = 0; j < vCMDaySchedule[i].count(); j++) {
            if (vCMDaySchedule[i].at(j).state) {
                createScheduleNumWidget(vCMDaySchedule[i].at(j), i + 1, scheduleShowItem);
            } else {
                createScheduleItemWidget(vCMDaySchedule[i].at(j), i + 1, scheduleShowItem);
            }
        }
    }
}

void CMonthScheduleView::createScheduleItemWidget(MScheduleDateRangeInfo info, int cNum, QVector<QGraphicsRectItem *> &scheduleShowItem)
{
    // qCDebug(ClientLogger) << "CMonthScheduleView::createScheduleItemWidget";
    DSchedule::Ptr gd = info.tData;
    QPoint pos;
    int fw;
    int fh;
    computePos(cNum, info.bdate, info.edate, pos, fw, fh);
    CMonthScheduleItem *gwi = new CMonthScheduleItem(QRect(pos.x(), pos.y(), fw, fh), nullptr);
    m_Scene->addItem(gwi);

    gwi->setData(gd);

    QColor TransparentC = "#000000";
    TransparentC.setAlphaF(0.05);
    scheduleShowItem.append(gwi);
}

void CMonthScheduleView::createScheduleNumWidget(MScheduleDateRangeInfo info, int cNum, QVector<QGraphicsRectItem *> &scheduleShowItem)
{
    // qCDebug(ClientLogger) << "CMonthScheduleView::createScheduleNumWidget";
    int type = CScheduleDataManage::getScheduleDataManage()->getTheme();
    CMonthScheduleNumItem *gwi = new CMonthScheduleNumItem(nullptr);
    QPoint pos;
    int fw;
    int fh;
    computePos(cNum, info.bdate, info.edate, pos, fw, fh);
    QColor gradientFromC = "#000000";
    gradientFromC.setAlphaF(0.00);
    gwi->setColor(gradientFromC, gradientFromC);
    QFont font;
    gwi->setSizeType(DFontSizeManager::T8);

    if (type == 0 || type == 1) {
        QColor tc("#5E5E5E");
        tc.setAlphaF(0.9);
        gwi->setText(tc, font);
    } else {
        QColor tc("#798190");
        tc.setAlphaF(1);
        gwi->setText(tc, font);
    }
    m_Scene->addItem(gwi);
    gwi->setRect(pos.x(), pos.y(), fw, fh);
    gwi->setData(info.num);
    gwi->setDate(info.bdate);
    scheduleShowItem.append(gwi);
}

void CMonthScheduleView::computePos(int cNum, QDate bgeindate, QDate enddate, QPoint &pos, int &fw, int &fh)
{
    // qCDebug(ClientLogger) << "CMonthScheduleView::computePos";
    int brow = static_cast<int>((m_beginDate.daysTo(bgeindate)) / DDEMonthCalendar::AFewDaysOfWeek);
    int bcol = (m_beginDate.daysTo(bgeindate)) % DDEMonthCalendar::AFewDaysOfWeek;
    int ecol = (m_beginDate.daysTo(enddate)) % DDEMonthCalendar::AFewDaysOfWeek;

    fw = static_cast<int>((ecol - bcol + 1) * ((m_width - m_leftMargin) / 7.0) - 11);
    fh = m_ItemHeight + 2;
    int x = static_cast<int>(m_leftMargin + bcol * ((m_width - m_leftMargin) / 7.0) + 5);
    //根据UI图调整item坐标
    int y = static_cast<int>(m_topMargin + ((m_height - m_topMargin - m_bottomMargin) / 6.0) * brow + schedule_Item_Y + (cNum - 1) * fh);
    pos = QPoint(x, y);
}

CWeekScheduleView::CWeekScheduleView(QObject *parent)
    : QObject(parent)
    , m_ScheduleHeight(22)
    , m_DayHeight(47)
{
    qCDebug(ClientLogger) << "CWeekScheduleView::CWeekScheduleView";
    setMaxNum();
}

CWeekScheduleView::~CWeekScheduleView()
{
    qCDebug(ClientLogger) << "CWeekScheduleView::~CWeekScheduleView";
}

void CWeekScheduleView::setData(QMap<QDate, DSchedule::List> &data, const QDate &startDate, const QDate &stopDate)
{
    qCDebug(ClientLogger) << "CWeekScheduleView::setData, from" << startDate << "to" << stopDate;
    //显示一周的日程
    Q_ASSERT(startDate.daysTo(stopDate) == 6);
    m_ScheduleInfo.clear();
    beginDate = startDate;
    endDate = stopDate;
    for (int i = 0 ; i <= beginDate.daysTo(endDate); ++i) {
        for (int j = 0; j < data[beginDate.addDays(i)].size(); ++j) {
            bool have = false;
            DSchedule::Ptr info = data[beginDate.addDays(i)].at(j);
            //过滤重复日程
            for (DSchedule::Ptr p : m_ScheduleInfo) {
                if (p == info) {
                    have = true;
                    break;
                }
            }
            if (!have) {
                m_ScheduleInfo.append(info);
            }
        }
    }
    //设置日程显示列数
    m_colum = static_cast<int>(startDate.daysTo(stopDate) + 1);
    updateSchedule(true);
}

bool CWeekScheduleView::addData(const DSchedule::Ptr &info)
{
    qCDebug(ClientLogger) << "CWeekScheduleView::addData";
    if(info.isNull()) {
        qCWarning(ClientLogger) << "Info is null, returning false";
        return false;
    }
    if (info->dtStart().date().daysTo(endDate) >= 0 && beginDate.daysTo(info->dtEnd().date()) >= 0) {
        qCDebug(ClientLogger) << "Schedule is within date range, updating view";
        clearItem();
        updateSchedule(false, info);
        return true;
    }

    qCDebug(ClientLogger) << "Schedule is outside date range, returning false";
    return false;
}

void CWeekScheduleView::changeDate(const DSchedule::Ptr &info)
{
    qCDebug(ClientLogger) << "CWeekScheduleView::changeDate";
    int index = m_ScheduleInfo.indexOf(info);

    if (index < 0) {
        qCDebug(ClientLogger) << "Info not found, appending to schedule list";
        m_ScheduleInfo.append(info);
    } else {
        qCDebug(ClientLogger) << "Info found, updating schedule at index" << index;
        m_ScheduleInfo[index] = info;
    }
    clearItem();
    updateSchedule(true);
}

void CWeekScheduleView::setHeight(const int ScheduleHeight, const int dayHeight)
{
    qCDebug(ClientLogger) << "CWeekScheduleView::setHeight, scheduleHeight:" << ScheduleHeight << "dayHeight:" << dayHeight;
    m_ScheduleHeight = ScheduleHeight;
    m_DayHeight = dayHeight;
    setMaxNum();
}

void CWeekScheduleView::updateSchedule(const bool isNormalDisplay, const DSchedule::Ptr &info)
{
    qCDebug(ClientLogger) << "CWeekScheduleView::updateSchedule, isNormalDisplay:" << isNormalDisplay;
    DSchedule::List schedulev;
    schedulev.clear();
    schedulev = m_ScheduleInfo;
    if (isNormalDisplay) {
        Q_UNUSED(info);
    } else {
        schedulev.append(info);
    }
    QDate tbegindate, tenddate;
    QVector<MScheduleDateRangeInfo> vMDaySchedule;
    m_ColumnScheduleCount.clear();
    m_ColumnScheduleCount.fill(0, m_colum);

    for (int i = 0; i < schedulev.size(); ++i) {
        //日程时间重新标定
        tbegindate = schedulev.at(i)->dtStart().date();
        tenddate = schedulev.at(i)->dtEnd().date();

        if (tenddate < beginDate || tbegindate > endDate)
            continue;
        if (tbegindate < beginDate)
            tbegindate = beginDate;
        if (tenddate > endDate)
            tenddate = endDate;
        //日程信息
        MScheduleDateRangeInfo _rangeInfo;
        _rangeInfo.bdate = tbegindate;
        _rangeInfo.edate = tenddate;
        _rangeInfo.tData = schedulev.at(i);
        _rangeInfo.state = false;
        vMDaySchedule.append(_rangeInfo);
        qint64 pos = beginDate.daysTo(_rangeInfo.bdate);
        qint64 count = _rangeInfo.bdate.daysTo(_rangeInfo.edate);
        int j = static_cast<int>(pos);

        for (; j < (pos + count + 1); ++j) {
            ++m_ColumnScheduleCount[j];
        }
    }
    std::sort(vMDaySchedule.begin(), vMDaySchedule.end());
    sortAndFilter(vMDaySchedule);
}

void CWeekScheduleView::clearItem()
{
    qCDebug(ClientLogger) << "CWeekScheduleView::clearItem";
    for (int i = 0; i < m_scheduleShowItem.count(); i++) {
        delete m_scheduleShowItem[i];
    }
    m_scheduleShowItem.clear();
}

void CWeekScheduleView::hideItem()
{
    qCDebug(ClientLogger) << "CWeekScheduleView::hideItem";
    for (int i = 0; i < m_scheduleShowItem.count(); i++) {
        m_scheduleShowItem[i]->setVisible(false);
    }
}


void CWeekScheduleView::setMaxNum()
{
    qCDebug(ClientLogger) << "CWeekScheduleView::setMaxNum";
    m_MaxNum = m_DayHeight / (m_ScheduleHeight + 1);
}

void CWeekScheduleView::mScheduleClear()
{
    qCDebug(ClientLogger) << "CWeekScheduleView::mScheduleClear";
    for (int i = 0; i < m_MScheduleInfo.size(); ++i) {
        m_MScheduleInfo[i].clear();
    }
    m_MScheduleInfo.clear();
}

void CWeekScheduleView::sortAndFilter(QVector<MScheduleDateRangeInfo> &vMDaySchedule)
{
    qCDebug(ClientLogger) << "CWeekScheduleView::sortAndFilter";
    QVector<QVector<bool>> scheduleFill;
    QVector<bool> scheduf;
    //初始化
    //m_colum列
    scheduf.fill(false, m_colum);
    //m_MaxNum 行
    scheduleFill.fill(scheduf, m_MaxNum);
    //标签起始位置
    int postion = 0;
    //标签结束位置
    int end = 0;
    mScheduleClear();
    for (int i = 0; i < vMDaySchedule.size(); ++i) {
        //获取起始位置
        postion = static_cast<int>(beginDate.daysTo(vMDaySchedule.at(i).bdate));
        //获取结束位置
        end = static_cast<int>(beginDate.daysTo(vMDaySchedule.at(i).edate));
        //初始化当前行
        int row = 0;
        int pos = postion;
        //日程长度
        int count = 0;
        int scheduleRow = row;

        for (; postion < end + 1; ++postion) {
            //如果当前行等于最大显示行
            if (row == m_MaxNum) {
                //初始化当前行
                row = 0;
                //初始化当前位置
                pos = postion;
            }
            while (row < m_MaxNum) {
                if (m_MScheduleInfo.size() < (row + 1)) {
                    RowScheduleInfo ms;
                    m_MScheduleInfo.append(ms);
                }
                //如果该位置没有被占用
                if (!scheduleFill[row][postion]) {
                    //如果该列日程总数大于最大显m_MScheduleInfo示数且该显示行没有超过最大显示行
                    if ((m_ColumnScheduleCount[postion] > m_MaxNum) && (row >= m_MaxNum - 1)) {
                        //占用该位置
                        scheduleFill[row][postion] = true;
                        //如果该位置不为起始位置
                        if (pos != postion) {
                            addShowSchedule(pos, postion - 1, row, vMDaySchedule.at(i).tData);
                        }
                        //设置还有xxx项
                        MScheduleDateRangeInfo info;
                        info.bdate = beginDate.addDays(postion);
                        info.edate = info.bdate;
                        info.num = m_ColumnScheduleCount[postion] - m_MaxNum + 1;
                        info.state = true;
                        m_MScheduleInfo[row].append(info);
                        //将该位置设为日程新的起始位置
                        pos = postion;
                        //从该列0行开始继续，因为for循环最后会跳转到下一列，所以在这里--以保证下次循环还在该列
                        --postion;
                        row = 0;
                        count = 0;
                    } else {
                        scheduleFill[row][postion] = true;
                        ++count;
                        scheduleRow = row;
                    }
                    break;
                } else {
                    //如果有显示的日程
                    if (count > 0) {
                        //如果该位置不为起始位置
                        if (pos != postion) {
                            addShowSchedule(pos, postion - 1, scheduleRow, vMDaySchedule.at(i).tData);
                        }
                    }
                    ++row;
                }
            }
        }
        //如果开始位置小于7并且长度大于0,则添加显示日程
        //开始位置0-6,显示长度1-7
        if (pos < 7 && count > 0) {
            addShowSchedule(pos, postion - 1, scheduleRow, vMDaySchedule.at(i).tData);
        }
    }
}

void CWeekScheduleView::addShowSchedule(const int &startPos, const int &endPos, const int &addRow, const DSchedule::Ptr &addInfo)
{
    qCDebug(ClientLogger) << "CWeekScheduleView::addShowSchedule, startPos:" << startPos << "endPos:" << endPos << "row:" << addRow;
    MScheduleDateRangeInfo scheduleInfo;
    //设置显示的开始日期
    scheduleInfo.bdate = beginDate.addDays(startPos);
    //设置显示的结束日期
    scheduleInfo.edate = beginDate.addDays(endPos);
    //false表示该信息为正常日程信息
    scheduleInfo.state = false;
    scheduleInfo.num = 0;
    //设置需要显示的日程
    scheduleInfo.tData = addInfo;
    //添加需要显示的日程
    m_MScheduleInfo[addRow].append(scheduleInfo);
}
