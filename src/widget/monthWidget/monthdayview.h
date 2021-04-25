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
#ifndef MONTHDAYVIEW_H
#define MONTHDAYVIEW_H

#include <DFrame>

#include <QObject>
#include <QDate>

DWIDGET_USE_NAMESPACE

class CMonthWidget;
/**
 * @brief The CMonthDayView class
 * 新建frame，放置年份widget
 */
class CMonthDayView : public DFrame
{
    Q_OBJECT
public:
    explicit CMonthDayView(QWidget *parent = nullptr);
    ~CMonthDayView();
    void setCurrentDate(const QDate date);
    void setRCurrentDate(const QDate date);
    void setTheMe(int type = 0);
    void setsearchfalg(bool flag);
signals:
    /**
     * @brief signalsSelectDate 选择日期的信号
     * @param date 日期
     */
    void signalsSelectDate(QDate date);
    /**
     * @brief signalsCurrentDate 当前时间的信号
     * @param date 日期
     */
    void signalsCurrentDate(QDate date);
private:
    /**
     * @brief m_monthWidget widget
     *
     * 月份所在的widget，用于显示12个月份
     */
    CMonthWidget *m_monthWidget = nullptr;
    //选择的日期
    QDate                       m_selectDate;
    //12个月份
    QDate                       m_days[12];
    int                         m_fixwidth = 200;
    int                         m_realwidth = 100;
    //搜索标志
    bool                        m_searchfalg = false;
};

class CMonthRect;
/**
 * @brief The CMonthWidget class
 * 年份所在的widget
 */
class CMonthWidget : public QWidget
{
    Q_OBJECT
public:
    CMonthWidget(QWidget *parent = nullptr);
    ~CMonthWidget() override;
    void setDate(const QDate date[12]);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    void updateSize();
    int getMousePosItem(const QPointF &pos);
signals:
    /**
     * @brief signalsSelectDate 选择日期的信号
     * @param date 选择的日期
     */
    void signalsSelectDate(QDate date);
private:
    QVector<CMonthRect *> m_MonthItem;
    QDate                       m_days[12];

};
/**
 * @brief The CMonthRect class
 * 将月份放在矩形中
 */
class CMonthRect
{
public:
    CMonthRect();
    void setDate(const QDate &date);
    QDate getDate()const;
    QRectF rect() const;
    void setRect(const QRectF &rect);
    inline void setRect(qreal x, qreal y, qreal w, qreal h);
    void paintItem(QPainter *painter, const QRectF &rect);
    void setDevicePixelRatio(const qreal pixel);
    static void setTheMe(int type);
    static void setSelectRect(CMonthRect *selectRect);
private:
    //月份所在矩形
    QRectF                              m_rect;
    //日期
    QDate                               m_Date;
    //系统主题类型
    static int                          m_themetype ;
    static qreal                        m_DevicePixelRatio;
    //默认字体颜色
    static QColor                       m_defaultTextColor;
    //默认背景色
    static QColor                       m_backgrounddefaultColor;
    //当前月的字体颜色
    static QColor                       m_currentDayTextColor;
    //当前月的背景色
    static QColor                       m_backgroundcurrentDayColor;
    //填充色
    static QColor                       m_fillColor;
    //月份字体
    static QFont                        m_dayNumFont;
    //选择的矩形
    static CMonthRect                  *m_SelectRect;
    //选择的颜色
    QColor                              m_selectColor;
};

#endif // MONTDAYVIEW_H
