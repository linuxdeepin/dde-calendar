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
#ifndef DAYMONTHVIEW_H
#define DAYMONTHVIEW_H
#include "customframe.h"
#include "calendardbus.h"

#include <DWidget>
#include <DImageButton>
#include <DPushButton>
#include <DFrame>
#include <DIconButton>

#include <QList>
#include <QDate>
#include <QStyleOption>
#include <QSignalMapper>
#include <QGridLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
class QLabel;
class CDayHuangLiLabel;
class CTodyButton;
/**
 * @brief The CDayMonthView class
 */
class CDayMonthView: public CustomFrame
{
    Q_OBJECT
public:
    explicit CDayMonthView(QWidget *parent = nullptr);
    ~CDayMonthView();
    void setFirstWeekday(int weekday);
    int getDateType(const QDate &date);
    inline bool cellSelectable() const
    {
        return m_cellSelectable;
    }
    void updateSelectState();
    void setLunarVisible(bool visible);
    QDate getCurrDate()
    {
        return m_currentDate;
    }
    void setTheMe(int type = 0);
    void setSearchFlag(bool flag);
    void updateFlag();
signals:
    /**
     * @brief dateSelected 选择时间的信号
     * @param date 日期
     * @param detail 黄历
     */
    void dateSelected(const QDate date, const CaHuangLiDayInfo &detail) const;
    /**
     * @brief signalcurrentDateChanged 当前日期改变的信号
     * @param date 日期
     */
    void signalcurrentDateChanged(QDate date);
    /**
     * @brief signalcurrentLunarDateChanged 当前阴历日期改变的信号
     * @param date 日期
     * @param detail 黄历
     * @param type
     */
    void signalcurrentLunarDateChanged(QDate date,  CaHuangLiDayInfo detail, int type = 0);
    /**
     * @brief cellSelectableChanged cell选择状态改变的信号
     * @param cellSelectable 选择状态
     */
    void cellSelectableChanged(bool cellSelectable) const;
    /**
     * @brief signalsReturnDay returnday的信号
     */
    void signalsReturnDay();
    /**
     * @brief signalSchedulHide 隐藏日程浮框的信号
     */
    void signalSchedulHide();
    /**
     * @brief signalIsDragging 拖拽信号
     * @param isDragging 拖拽状态
     */
    void signalIsDragging(bool &isDragging);
public slots:
    void setCurrentDate(const QDate date, int type = 0);
    void handleCurrentDateChanged(const QDate date, const CaHuangLiDayInfo &detail);
    void slottoday();
private:
    int getDateIndex(const QDate &date) const;
    const QString getCellDayNum(int pos);
    const QDate getCellDate(int pos);
    void paintCell(QWidget *cell);
    bool eventFilter(QObject *o, QEvent *e) override;
    void updateDate();
    void getlineflag();

    const QString getLunar(int pos);
    const CaHuangLiDayInfo getCaHuangLiDayInfo(int pos);

    void updateCurrentLunar();
    void initUI();
    void initConnection();
    void updateDateShow();
    void updateDateLunarDay();
private slots:
    void cellClicked(QWidget *cell);
    void setSelectedCell(int index);
    void slotprev();
    void slotnext();
    void getDbusData();
protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *e) override;
private:
    //上一个月按钮
    DIconButton      *m_prevButton = nullptr;
    //下一个月按钮
    DIconButton      *m_nextButton = nullptr;
    //返回今天按钮
    CTodyButton *m_today = nullptr;
    //本月frame
    CustomFrame *m_currentMouth = nullptr;
    //今天frame
    CustomFrame *m_currentDay = nullptr;
    //今天frame
    CustomFrame *m_currentYear = nullptr;
    //本周frame
    CustomFrame *m_currentWeek = nullptr;
    //阴历frame
    CustomFrame *m_currentLuna = nullptr;
    //宜
    CDayHuangLiLabel *m_yiLabel = nullptr;
    //忌
    CDayHuangLiLabel *m_jiLabel = nullptr;
    //cell list
    QList<QWidget *> m_cellList;
    //42天时间
    QDate m_days[42];
    //今天
    QDate m_currentDate;
    CalendarDBus *m_DBusInter = nullptr;
    //选择的cell
    int m_selectedCell = 0;
    //是否被选择
    bool m_cellSelectable = true;
    //daynun字体
    QFont m_dayNumFont;
    //颜色
    QColor m_topBorderColor = Qt::red;
    QColor m_backgroundCircleColor = "#2ca7f8";
    QColor m_defaultTextColor = Qt::black;
    QColor m_currentDayTextColor = "#2ca7f8";
    QColor m_weekendsTextColor = Qt::black;
    QColor m_selectedTextColor = Qt::white;
    QColor m_festivalTextColor = Qt::black;
    QColor m_notCurrentTextColor = "#b2b2b2";
    //每周以周几开始
    int m_firstWeekDay;
    //黄历状态
    bool m_huanglistate = true;
    //grid布局
    QGridLayout *m_gridLayout = nullptr;
    //cell的宽的
    int cellwidth = 20;
    //cell的高度
    int cellheight = 20;
    QVBoxLayout *m_hhLayout = nullptr;
    //上部分布局
    QVBoxLayout *m_upLayout = nullptr;
    //分割线
    DHorizontalLine *m_splitline = nullptr;
    //宜布局
    QVBoxLayout *m_yidownLayout = nullptr;
    //忌布局
    QVBoxLayout *m_jidownLayout = nullptr;
    QStringList m_weeklist;
    //队列
    QQueue<int> *queue = nullptr;
    //日期和黄历对应的map
    QMap<QDate, CaHuangLiDayInfo> *lunarCache = nullptr;
    //黄历信息
    CaHuangLiDayInfo *emptyCaHuangLiDayInfo = nullptr;
    //系统主题类型
    int                   m_themetype  = 1;
    //圆角半径
    const int m_radius = 8;
    //搜索标志
    bool m_searchflag = false;
    QColor m_ceventColor = "#FF5D00";
    //节假日和日程标识
    QVector<bool> m_vlineflag;
};

#endif // YEARVIEW_H
