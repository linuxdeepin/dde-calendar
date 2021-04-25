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
#ifndef YEARSCHCEDULEVIEW_H
#define YEARSCHCEDULEVIEW_H

#include "schedulestructs.h"

#include <DLabel>
#include <DPushButton>
#include <DListWidget>
#include <DWidget>
#include <DArrowRectangle>

DWIDGET_USE_NAMESPACE
class QVBoxLayout;
class CYearSchceduleItem;
class CYearSchceduleOutView;
/**
 * @brief The CYearSchceduleView class
 * 对日程进行排序
 * 绘制日程信息
 */
class CYearSchceduleView : public DWidget
{
    Q_OBJECT

public:
    /**
     * @brief CYearSchceduleView 构造函数
     * @param parent 父类
     */
    explicit CYearSchceduleView(QWidget *parent = nullptr);
    /**
      * @brief ~CYearSchceduleView 析构函数
      */
    ~CYearSchceduleView();
    void setSoloDay(QString soloday);
    void setData(QVector<ScheduleDtailInfo> &vListData);
    /**
     * @brief getlistdate 获取日程信息
     * @return  日程信息
     */
    QVector<ScheduleDtailInfo> getlistdate()
    {
        return m_vlistData;
    }
    void clearData();
    int showWindow();
    void setTheMe(int type = 0);
    void setDtype(int type, int arrowheight);
    void setCurrentDate(QDate cdate);
    QDate getCurrentDate();
    void adjustPosition(bool ad);
private:
    void updateDateShow();
    void createItemWidget(ScheduleDtailInfo info, int type = 0);
protected:
    void paintEvent(QPaintEvent *event) override;
    void paintItem(ScheduleDtailInfo info, int index, int type = 0);
    void paintItem();
private:
    //下拉列表窗
    DListWidget *m_gradientItemList = nullptr;
    bool                                         m_widgetFlag;
    /**
     * @brief m_vlistData 日程信息容器
     *
     * 将所有日程的信息放到容器中，以方便其他类调用
     */
    QVector<ScheduleDtailInfo>                   m_vlistData;
    QString                                      m_soloDay;
    //日程容器
    QVector<DLabel *> m_labellist;
    //系统主题类型
    int                                          m_type;
    //当天日期
    QDate                                        m_currentDate;
    //背景色
    QColor                m_bBackgroundcolor = "#000000";
    //时间颜色
    QColor                m_btimecolor = "#526A7F";
    //文字颜色
    QColor                m_bttextcolor = "#414D68";
    QColor                m_lBackgroundcolor = Qt::white;
    QColor                m_ltextcolor = "#001A2E";
    QColor                m_solocolor = "#001A2E";
    QColor                m_TBcolor = "#001A2E";
    int                   m_dtype = 3;
    int                   m_arrowheight = 0;
    QColor                m_borderColor = "#000000";
    //日程浮框朝向
    bool                  adjustPos = false;
};
/**
 * @brief The CYearSchceduleItem class
 */
class CYearSchceduleItem : public DLabel
{
    Q_OBJECT

public:
    explicit CYearSchceduleItem(QWidget *parent = nullptr);
    void setBackgroundColor(QColor color1);
    void setStateColor(QColor color1);
    void setText(QColor tcolor, QFont font);
    void setTimeC(QColor tcolor, QFont font);
    void setData(ScheduleDtailInfo  vScheduleInfo);
protected:
    void paintEvent(QPaintEvent *e) override;
private:
    //日程信息
    ScheduleDtailInfo m_ScheduleInfo;
    //背景颜色
    QColor                m_Backgroundcolor;
    //时间颜色
    QColor                m_timecolor;
    QColor                m_Statecolor;
    //时间字体
    QFont                 m_timefont;
    //文字的颜色
    QColor                m_ttextcolor;
    //文字的字体
    QFont                 m_tfont;
};
/**
 * @brief The CYearSchceduleOutView class
 * DTK的浮框
 * 将yearschceduleview绘制完成的日程信息放到浮框中
 */
class CYearSchceduleOutView : public DArrowRectangle
{
    Q_OBJECT

public:
    explicit CYearSchceduleOutView(QWidget *parent = nullptr);
    void setSoloDay(QString soloday);
    void setData(QVector<ScheduleDtailInfo> &vListData);
    void clearData();
    void showWindow();
    void setTheMe(int type = 0);
    void setDtype(int type, int arrowheight);
    void setCurrentDate(QDate cdate);
    void adjustPosition(bool ad);
signals:
    /**
     * @brief signalsViewSelectDate 跳转视图信号
     * @param date 当天的时间
     */
    void signalsViewSelectDate(QDate date);
    /**
     * @brief signalupdateschcedule 更新日程信息的信号
     */
    void signalupdateschcedule();
private:
    /**
     * @brief yearschceduleview 年视图日程
     *
     * 将年视图的日程信息放到DTK的浮框中，构成整体的日程浮框
     */
    CYearSchceduleView *yearschceduleview = nullptr;
    /**
     * @brief scheduleinfoList 日程信息
     *
     * 将需要展示的日程信息放到容器中，再对每个日程进行分析
     */
    QVector<ScheduleDtailInfo> scheduleinfoList;
    //当天时间
    QDate currentdate;
    //日程数量
    int list_count = 0;

protected:
    void mousePressEvent(QMouseEvent *event) override;
};
#endif // CSHCEDULEDAYVIEW_H

