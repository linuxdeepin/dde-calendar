// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MONTHBREFWIDGET_H
#define MONTHBREFWIDGET_H

#include <QWidget>
#include <QDate>

class CMonthDayRect;
class MonthBrefWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MonthBrefWidget(QWidget *parent = nullptr);
    ~MonthBrefWidget() override;
    //设置每个月的日期
    void setDate(const int showMonth, const QVector<QDate> &showDate);
    //根据系统主题设置颜色
    void setTheMe(int type = 0);
    //设置是否有日程的标志
    void setLintFlag(const QVector<bool> &lineFlag);
    //设置是否有搜索日程
    void setSearchScheduleFlag(const QVector<bool> &searchFlag);
private:
    //设置每天的日期所在矩形框的大小
    void updateSize();
    //获取点击日期的索引
    int getMousePosItem(const QPointF &pos);
protected:
    //设置每天的日期所在矩形框的大小
    void resizeEvent(QResizeEvent *event) override;
    //鼠标单击事件，单击日期显示当天的日程和节日。
    void mousePressEvent(QMouseEvent *event) override;
    //鼠标双击事件，双击跳转到上次打开的视图
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    //鼠标释放事件
    void mouseReleaseEvent(QMouseEvent *event) override;
    //鼠标移动事件，设置hover状态
    void mouseMoveEvent(QMouseEvent *event) override;
    //绘制日期以及当天状态
    void paintEvent(QPaintEvent *event) override;
    //离开事件，设置当前选中的日期为空
    void leaveEvent(QEvent *event) override;
private:
    //鼠标点击触发事件
    void mousePress(const QPoint &point);
signals:
    /**
     * @brief signalPressDate 鼠标点击日期的信号
     * @param date 传递时间参数
     */
    void signalPressDate(const QDate &date);
    /**
     * @brief signalDoubleClickDate 鼠标双击日期的信号
     * @param date 传递时间参数
     */
    void signalDoubleClickDate(const QDate &date);
private:
    QVector<CMonthDayRect *>        m_DayItem;
    int m_currentMonth = 1;
    bool                            m_press = false;
    int m_pressIndex = 0;
    //触摸状态 0：原始  1：点击  2：移动
    int         m_touchState{0};
    //触摸点击坐标
    QPoint      m_touchBeginPoint;
};


class CMonthDayRect
{
public:
    /**
     * @brief The CellEventType enum
     */
    enum CellEventType {
        CellNormal = 0,
        Cellhover = 1,
        CellPress = 2
    };
    /**
     * @brief The CellColor struct
     */
    struct CellColor {
        QColor normalColor;
        QColor hoverColor;
        QColor pressColor;
    };
    CMonthDayRect();
    ~CMonthDayRect();
    //设置某一天的时间
    void setDate(const QDate &date);
    //获取某一天的时间
    QDate getDate()const;
    //设置某一天的状态，CellEventType
    void setCellEvent(const CellEventType &type);
    //设置是否是当前月
    void setIsCurrentMonth(const bool isCurrMonth);
    //当天日期所在的矩形
    QRectF rect() const;
    //设置当天日期的矩形
    void setRect(const QRectF &rect);
    //设置当天日期的矩形
    inline void setRect(qreal x, qreal y, qreal w, qreal h);
    //绘制年视图一天的所有状态并展示出来。
    void paintItem(QPainter *painter, const QRectF &rect);
    //设置是否有日程的标志
    void setLineFlag(const bool flag);
    //设置是否有搜索日程标志
    void setSearchScheduleFlag(const bool flag);
    //根据系统主题类型设置颜色
    static void setTheMe(int type = 0);
    //设置显示缩放比例
    static void setDevicePixelRatio(const qreal pixel);
    //设置当前天
    static void setCurrentRect(CMonthDayRect *currrect);
    void setSystemActiveColor(const QColor &activeColor);
private:
    QFont                   m_dayNumFont;
    QFont                   m_hightFont;
    int                     m_themetype = 0;
    CellEventType           m_cellEventType {CellNormal};
    QColor                  m_highColor = "#0081FF";
    QColor                  m_highTextColor = "#FFFFFF";
    bool                    m_vlineflag = false;
    bool                    m_searchScheduleFlag{false};
    QColor                  m_currentDayTextColor = "#2ca7f8";

    static QColor                   m_defaultTextColor;
    static QColor                   m_selectedTextColor;
    static QColor                   m_notCurrentTextColor;
    static CellColor                m_currentColor;
    static QColor                   m_ceventColor;
    static CMonthDayRect            *m_CurrentRect;
    static qreal                    m_DevicePixelRatio;

    QRectF                  m_rect;
    QDate                   m_date;
    bool                    m_isCurrentMonth = false;
};


#endif // MONTHBREFWIDGET_H
