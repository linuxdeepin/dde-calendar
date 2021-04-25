#ifndef YEARVIEW_H
#define YEARVIEW_H

#include "schedulestructs.h"
#include "customframe.h"

#include <DWidget>
#include <DFrame>

#include <QList>
#include <QDate>
#include <QStyleOption>
#include <QLabel>
#include <QSignalMapper>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
class CalendarDBus;
class CaLunarDayInfo;
class QLabel;
class CYearSchceduleView;
class CYearSchceduleOutView;
class WeekIndicator;
/**
 * @brief The CalendarYearDayType enum
 */
enum CalendarYearDayType {
    SO_YFestival = QStyleOption::SO_CustomBase + 0x01,
    SO_YWeekends = QStyleOption::SO_CustomBase + 0x02,
    SO_YWeekendsAndFestival = SO_YFestival | SO_YWeekends,
    SO_YNotCurrentMonth = 0x04,
    SO_YNotCurrentMonthFestival = SO_YNotCurrentMonth | SO_YFestival,
    SO_YDefault,
};
//#define M_FLAG 0;

class CYearMonthView;
class CYearView: public CustomFrame
{
    Q_OBJECT

    Q_PROPERTY(QColor backgroundCircleColor MEMBER m_backgroundCircleColor DESIGNABLE true SCRIPTABLE true)
    Q_PROPERTY(QColor weekendsTextColor MEMBER m_weekendsTextColor DESIGNABLE true SCRIPTABLE true)
    Q_PROPERTY(QColor topBorderColor MEMBER m_topBorderColor DESIGNABLE true SCRIPTABLE true)
    Q_PROPERTY(bool cellSelectable READ cellSelectable WRITE setCellSelectable NOTIFY cellSelectableChanged)
public:
    explicit CYearView(QWidget *parent = nullptr);
    void setFirstWeekday(int weekday);
    /**
     * @brief cellSelectable
     * @return
     */
    inline bool cellSelectable() const
    {
        return m_cellSelectable;
    }
    void setTheMe(int type = 0);
    static void SchceduleViewHide();
    void getInfoAndSetLineFlag();
signals:
    /**
     * @brief signalcurrentDateChanged 当前现实日期改变的信号
     * @param date 传递日期参数
     */
    void signalcurrentDateChanged(QDate date);
    /**
     * @brief cellSelectableChanged
     * @param cellSelectable
     */
    void cellSelectableChanged(bool cellSelectable) const;
    /**
     * @brief signaldoubleclickDate 鼠标双击日期的信号
     * @param date 传递日期参数
     */
    void signaldoubleclickDate(QDate date);
    /**
     * @brief signalselectMonth 鼠标双击月份的信号
     * @param date 传递日期参数
     */
    void signalselectMonth(QDate date);
    /**
     * @brief signalselectWeekwindow 选择周视图的信号
     * @param date 传递日期参数
     */
    void signalselectWeekwindow(QDate date);
    /**
     * @brief signalupdateschcedule 更新日程的信号
     */
    void signalupdateschcedule();
    /**
     * @brief signalHideInfo 隐藏日程浮框的信号
     */
    void signalHideInfo();
    /**
     * @brief signalSelectInfo 设置选择是否选择日期的标志
     * @param flag 是否选择日期
     */
    void signalSelectInfo(bool flag);
public slots:
    void setCurrentDate(const QDate date, int type = 0);
    void setCellSelectable(bool selectable);
    /**
     * @brief slotupdateSchedule 设置当前时间
     * @param id
     */
    void slotupdateSchedule(const int id);
    void slotDoubleClickDate(const QDate &date);
    void slotPressClickDate(const QDate &date);
private:
    int getDateIndex(const QDate &date) const;
    bool eventFilter(QObject *o, QEvent *e) override;
    void updateDate();
    void createYearSchceduleView(QWidget *parent = nullptr);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
protected:
    void paintEvent(QPaintEvent *e) override;
private:
    //当前月
    CustomFrame *m_currentMouth = nullptr;
    //一个月的日期
    QDate m_days[42];
    //当天
    QDate m_currentDate;
    QStringList m_monthList;
    int m_selectedCell = -1;
    bool m_cellSelectable = true;

    QFont  m_momthFont;
    QColor m_topBorderColor = Qt::red;
    QColor m_backgroundCircleColor = "#2ca7f8";
    QColor m_currentDayTextColor = "#2ca7f8";
    QColor m_weekendsTextColor = Qt::black;
    QColor m_festivalTextColor = Qt::black;
    QColor m_cellBackgroundColor = "#FFFFFF" ;
    //每周以周几开始
    int m_firstWeekDay = 0;
    //总体布局
    QVBoxLayout *m_hhLayout = nullptr;
    //系统主题类型
    int m_themetype = 1;
    QColor m_bnormalColor = "#FFFFFF";
    //设置圆角半径
    const int m_radius = 8;
    const int m_borderframew = 0;
    //日程浮框
    static CYearSchceduleOutView *m_Scheduleview;
    //选择标志
    bool m_selectFlag = false;
    //选择的日期
    QDate m_selectDate;
    //节假日和日程标识
    QVector<bool> m_vlineflag;
    //月份
    CYearMonthView *m_monthView = nullptr;
    //日程信息
    QVector<ScheduleDateRangeInfo> m_DateRangeInfo;
};

class CMonthDayRect;
class CYearMonthView : public DWidget
{
    Q_OBJECT
public:
    /**
     * @brief CYearMonthView 构造函数
     * @param parent 父类
     */
    explicit CYearMonthView(DWidget *parent = nullptr);
    /**
      *@brief ~CYearMonthView 析构函数
      */
    ~CYearMonthView() override;
    void setDate(const QDate date[42]);
    void setTheMe(int type = 0);
    void setLintFlag(const QVector<bool> &lineFlag);
private:
    void updateSize();
    int getMousePosItem(const QPointF &pos);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void leaveEvent(QEvent *event) override;
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
    //日期容器
    QVector<CMonthDayRect *>        m_DayItem;
    int                             m_currentMonth =1;
    //节假日和日程标识
    QVector<bool>                   m_vlineflag;
    //鼠标点击标志
    bool                            m_press = false;
    //鼠标点击日期的索引
    int                             m_pressIndex =0;
};


class CMonthDayRect
{
public:
    /**
     * @brief The CellEventType enum
     */
    enum CellEventType {
        CellNormal = 0,//正常状态
        Cellhover = 1,//hover状态
        CellPress = 2//press状态
    };
    /**
     * @brief The CellColor struct
     */
    struct CellColor {
        QColor normalColor;//正常状态的颜色
        QColor hoverColor;//hover状态的颜色
        QColor pressColor;//press状态的颜色
    };
    CMonthDayRect();
    ~CMonthDayRect();
    void setDate(const QDate &date);
    QDate getDate()const;
    void setCellEvent(const CellEventType &type);
    void setIsCurrentMonth(const bool isCurrMonth);
    QRectF rect() const;
    void setRect(const QRectF &rect);
    inline void setRect(qreal x, qreal y, qreal w, qreal h);
    void paintItem(QPainter *painter, const QRectF &rect);
    void setLineFlag(const bool flag);
    static void setTheMe(int type = 0);
    static void setDevicePixelRatio(const qreal pixel);
    static void setCurrentRect(CMonthDayRect *currrect);
private:
    QFont                   m_dayNumFont;
    QFont                   m_hightFont;
    //系统主题类型
    int                     m_themetype = 0;
    //日期矩形的类型
    CellEventType           m_cellEventType {CellNormal};
    QColor                  m_highColor = "#0081FF";
    QColor                  m_highTextColor = "#FFFFFF";
    //节假日或日程标志
    bool                    m_vlineflag = false;
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
    //是否是当天月
    bool                    m_isCurrentMonth = false;
};

#endif // YEARVIEW_H
