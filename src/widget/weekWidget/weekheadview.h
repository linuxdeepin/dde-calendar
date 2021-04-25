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

#ifndef WEEKHEADVIEW_H
#define WEEKHEADVIEW_H

#include "constants.h"
#include "calendardbus.h"

#include <DLabel>
#include <DFrame>

#include <QWidget>
#include <QList>
#include <QDate>
#include <QStyleOption>
#include <QSignalMapper>

DWIDGET_USE_NAMESPACE
/**
 * @brief The CalendarWeekDayType enum
 */
enum CalendarWeekDayType {
    SO_MFestival = QStyleOption::SO_CustomBase + 0x01,
    SO_MWeekends = QStyleOption::SO_CustomBase + 0x02,
    SO_MWeekendsAndFestival = SO_MFestival | SO_MWeekends,
    SO_MNotCurrentMonth = 0x04,
    SO_MNotCurrentMonthFestival = SO_MNotCurrentMonth | SO_MFestival,
    SO_MDefault,
};
class CustomFrame;
class CWeekHeadView: public DFrame
{
    Q_OBJECT
public:
    /**
     * @brief The ShowState enum
     */
    enum ShowState {
        ShowLunar = 0x01,
        ShowLunarFestivalHighlight = 0x02,
        Normal = ShowLunar | ShowLunarFestivalHighlight,
    };

public:
    explicit CWeekHeadView(QWidget *parent = nullptr);
    ~CWeekHeadView() override;
    void setFirstWeekday(int weekday);
    int getDateType(const QDate &date);
    /**
     * @brief cellSelectable
     * @return
     */
    inline bool cellSelectable() const
    {
        return m_cellSelectable;
    }
    void setTheMe(int type = 0);
    void setWeekDay(QVector<QDate> vDays);
    void setMounthLabelWidth(int w, int rw);
signals:
    /**
     * @brief dateSelected 选择时间的信号
     * @param date 时间
     * @param detail 时间的阴历信息
     */
    void dateSelected(const QDate date, const CaLunarDayInfo &detail) const;
    /**
     * @brief signalcurrentLunarDateChanged 当前周的阴历日期改变的信号
     * @param vdate 一周的日期
     * @param vdetail 一周的阴历信息
     * @param type 是否显示信息
     */
    void signalcurrentLunarDateChanged(QVector<QDate> vdate, QVector<CaLunarDayInfo> vdetail, int type = 0);
    /**
     * @brief signalcurrentDateChanged 当前时间改变的信号
     * @param date 当前时间
     */
    void signalcurrentDateChanged(QDate date);
    /**
     * @brief currentDateChanged
     * @param year
     * @param month
     */
    void currentDateChanged(const int year, const int month);
    /**
     * @brief currentFestivalChanged 当前的节日信息改变的信号
     * @param festival 节日信息
     */
    void currentFestivalChanged(const QString &festival);
    /**
     * @brief cellSelectableChanged
     * @param cellSelectable
     */
    void cellSelectableChanged(bool cellSelectable) const;
    /**
     * @brief signalsViewSelectDate 选择日期的信号
     * @param date 日期
     */
    void signalsViewSelectDate(QDate date);
    /**
     * @brief signaleSchedulHide 隐藏日程浮框的信号
     */
    void signaleSchedulHide();
public slots:
    void setCurrentDate(const QDate date);
    void setLunarVisible(bool visible);
    /**
     * @brief setCellSelectable
     * @param selectable
     */
    void setCellSelectable(bool selectable);
    void handleCurrentDateChanged(const QDate date, const CaLunarDayInfo &detail);
private:
    int getDateIndex(const QDate &date) const;
    const QString getCellDayNum(int pos);
    const QDate getCellDate(int pos);
    const QString getLunar(int pos);
    const CaLunarDayInfo getCaLunarDayInfo(int pos);
    void paintCell(QWidget *cell);
    bool eventFilter(QObject *o, QEvent *e) override;
    void updateDate();
    void updateCurrentLunar(const CaLunarDayInfo &info);
    int checkDay(int weekday);
protected:
    void mousePressEvent(QMouseEvent *event) override;
private slots:
    void cellClicked(QWidget *cell);
    void setSelectedCell(int index);
    void getDbusData();
private:
    //放置cell的列表
    QList<QWidget *> m_cellList;
    //月份frame
    CustomFrame *m_monthLabel = nullptr;
    //dbus
    CalendarDBus *m_DBusInter = nullptr;
    //一周的时间
    QDate m_days[7];
    //当前时间
    QDate m_currentDate;
    //月份列表
    QStringList m_monthList;
    //显示状态
    ShowState m_showState = Normal;
    //选择cell的索引
    int m_selectedCell = 0;
    //是否选择
    bool m_cellSelectable = true;
    //daynum字体
    QFont m_dayNumFont;
    //月份字体
    QFont m_monthFont;

    QColor m_backgroundCircleColor = "#2ca7f8";
    QColor m_backgroundShowColor = "#2CA7F8";
    QColor m_defaultTextColor = "#6F6F6F";
    QColor m_currentDayTextColor = "#FFFFFF";
    QColor m_defaultLunarColor = "#898989";
    QColor m_weekendsTextColor = "#0887FF";
    QColor m_currentMonthColor = "#000000";
    QColor m_backgroudColor = "#E6EEF2";
    /**
     * @brief m_Background_Weekend_Color 周六周日背景色
     *
     * 周末背景色不同于工作日背景色
     */
    QColor m_Background_Weekend_Color = "#00429A";
    //节假日和阴历颜色
    QColor m_solofestivalLunarColor = "#4DFF7272";
    //队列
    QQueue<int> *queue = nullptr;
    //日期和阴历节日对应字典
    QMap<QDate, CaLunarDayInfo> *lunarCache = nullptr;
    //阴历信息
    CaLunarDayInfo *emptyCaLunarDayInfo = nullptr;
    //每周以周几开始
    int m_firstWeekDay;
    //系统主题类型
    int m_themetype = 1;
    int m_monthW = 80;
    int m_fixwidth = 200;
    //圆角半径
    const int m_radius = 8;
};

#endif // MYCALENDARWIDGET_H
