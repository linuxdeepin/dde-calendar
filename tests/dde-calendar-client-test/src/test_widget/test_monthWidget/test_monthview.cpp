// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_monthview.h"
#include <QTimer>
#include <QApplication>
#include "huangliData/dbusdatastruct.h"

static DSchedule::Ptr makeMonthViewSchedule(const QDate &start, const QDate &end, const QString &summary = "test")
{
    DSchedule::Ptr info(new DSchedule);
    info->setDtStart(QDateTime(start, QTime(10, 0)));
    info->setDtEnd(QDateTime(end, QTime(11, 0)));
    info->setSummary(summary);
    info->setAllDay(false);
    return info;
}

static DSchedule::List makeMonthViewScheduleList()
{
    DSchedule::List list;
    QDate current = QDate::currentDate();
    list.append(makeMonthViewSchedule(current, current, "schedule1"));
    list.append(makeMonthViewSchedule(current.addDays(1), current.addDays(1), "schedule2"));
    list.append(makeMonthViewSchedule(current.addDays(2), current.addDays(2), "schedule3"));
    return list;
}

static QMap<QDate, DSchedule::List> makeMonthViewScheduleMap()
{
    QMap<QDate, DSchedule::List> data;
    QDate current = QDate::currentDate();
    data[current] = makeMonthViewScheduleList();
    return data;
}

static QVector<QDate> makeMonthViewDateList()
{
    QVector<QDate> list;
    QDate current = QDate::currentDate();
    for (int i = 0; i < 42; ++i) {
        list.append(current.addDays(i));
    }
    return list;
}

static QMap<QDate, CaHuangLiDayInfo> makeMonthViewHuangLi()
{
    QMap<QDate, CaHuangLiDayInfo> data;
    CaHuangLiDayInfo info;
    info.mGanZhiYear = "甲辰龙年";
    info.mGanZhiMonth = "戊辰月";
    info.mGanZhiDay = "甲子日";
    info.mLunarMonthName = "三月";
    info.mLunarDayName = "初一";
    info.mLunarLeapMonth = 0;
    info.mZodiac = "龙";
    info.mTerm = "";
    info.mSolarFestival = "";
    info.mLunarFestival = "";
    info.mSuit = "嫁娶";
    info.mAvoid = "入土";
    data[QDate::currentDate()] = info;
    return data;
}

test_monthview::test_monthview()
{
    mMonthView = new CMonthView();
}

test_monthview::~test_monthview()
{
    delete mMonthView;
    mMonthView = nullptr;
}

// CMonthView::CMonthView — mid priority
TEST_F(test_monthview, Constructor)
{
    // Arrange
    // Act
    CMonthView *view = new CMonthView();

    // Assert
    EXPECT_NE(view, nullptr);
    delete view;
}

// CMonthView::setTheMe
TEST_F(test_monthview, setTheMe)
{
    // Arrange
    // Act
    mMonthView->setTheMe(0);
    mMonthView->setTheMe(1);
    mMonthView->setTheMe(2);

    // Assert
    SUCCEED();
}

// CMonthView::setFirstWeekday
TEST_F(test_monthview, setFirstWeekday)
{
    // Arrange
    // Act
    mMonthView->setFirstWeekday(Qt::Sunday);
    mMonthView->setFirstWeekday(Qt::Monday);

    // Assert
    SUCCEED();
}

// CMonthView::setShowDate
TEST_F(test_monthview, setShowDate)
{
    // Arrange
    QVector<QDate> dates = makeMonthViewDateList();

    // Act
    mMonthView->setShowDate(dates);

    // Assert
    SUCCEED();
}

// CMonthView::setHuangLiInfo
TEST_F(test_monthview, setHuangLiInfo)
{
    // Arrange
    QMap<QDate, CaHuangLiDayInfo> huangLi = makeMonthViewHuangLi();

    // Act
    mMonthView->setHuangLiInfo(huangLi);

    // Assert
    SUCCEED();
}

// CMonthView::setFestival
TEST_F(test_monthview, setFestival)
{
    // Arrange
    QMap<QDate, int> festivalInfo;
    festivalInfo[QDate::currentDate()] = 1;
    festivalInfo[QDate::currentDate().addDays(1)] = 2;

    // Act
    mMonthView->setFestival(festivalInfo);

    // Assert
    SUCCEED();
}

// CMonthView::setScheduleInfo
TEST_F(test_monthview, setScheduleInfo)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeMonthViewScheduleMap();

    // Act
    mMonthView->setScheduleInfo(data);

    // Assert
    SUCCEED();
}

// CMonthView::setSearchScheduleInfo
TEST_F(test_monthview, setSearchScheduleInfo)
{
    // Arrange
    DSchedule::List list = makeMonthViewScheduleList();

    // Act
    mMonthView->setSearchScheduleInfo(list);

    // Assert
    SUCCEED();
}

// CMonthView::setCurrentDate
TEST_F(test_monthview, setCurrentDate)
{
    // Arrange
    QDate date = QDate::currentDate();

    // Act
    mMonthView->setCurrentDate(date);

    // Assert
    SUCCEED();
}

// CMonthView::setRemindWidgetTimeFormat
TEST_F(test_monthview, setRemindWidgetTimeFormat)
{
    // Arrange
    // Act
    mMonthView->setRemindWidgetTimeFormat("hh:mm");

    // Assert
    SUCCEED();
}

// CMonthView::setSelectSchedule
TEST_F(test_monthview, setSelectSchedule)
{
    // Arrange
    DSchedule::Ptr info = makeMonthViewSchedule(QDate::currentDate(), QDate::currentDate());

    // Act
    mMonthView->setSelectSchedule(info);

    // Assert
    SUCCEED();
}

// CMonthView::setLunarVisible
TEST_F(test_monthview, setLunarVisible)
{
    // Arrange
    // Act
    mMonthView->setLunarVisible(true);
    mMonthView->setLunarVisible(false);

    // Assert
    SUCCEED();
}

// CMonthView::deleteSelectSchedule — mid priority
TEST_F(test_monthview, deleteSelectSchedule)
{
    // Arrange — dismiss any modal dialog that deleteSelectSchedule may pop up
    QTimer::singleShot(100, []() {
        QWidget *w = QApplication::activeModalWidget();
        if (w) w->close();
    });

    // Act
    mMonthView->deleteSelectSchedule();

    // Assert
    SUCCEED();
}

// CMonthView::slotScheduleRemindWidget — mid priority
TEST_F(test_monthview, slotScheduleRemindWidget_hide)
{
    // Arrange
    DSchedule::Ptr info = makeMonthViewSchedule(QDate::currentDate(), QDate::currentDate());

    // Act
    mMonthView->slotScheduleRemindWidget(false, info);

    // Assert
    SUCCEED();
}

TEST_F(test_monthview, slotScheduleRemindWidget_show)
{
    // Arrange
    DSchedule::Ptr info = makeMonthViewSchedule(QDate::currentDate(), QDate::currentDate());

    // Act
    mMonthView->slotScheduleRemindWidget(true, info);

    // Assert
    SUCCEED();
}

// CMonthView::resizeEvent
TEST_F(test_monthview, resizeEvent)
{
    // Arrange
    QResizeEvent e(QSize(400, 300), QSize(200, 150));

    // Act
    mMonthView->resizeEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthView::mousePressEvent
TEST_F(test_monthview, mousePressEvent)
{
    // Arrange
    QMouseEvent e(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    mMonthView->mousePressEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthView::event
TEST_F(test_monthview, event)
{
    // Arrange
    QFocusEvent e(QEvent::FocusIn, Qt::TabFocusReason);

    // Act
    mMonthView->event(&e);

    // Assert
    SUCCEED();
}

// CMonthView::getScheduleInfo — private
TEST_F(test_monthview, getScheduleInfo_normal)
{
    // Arrange
    QDate begin = QDate::currentDate();
    QDate end = begin.addDays(1);

    // Act
    DSchedule::Ptr info = mMonthView->getScheduleInfo(begin, end);

    // Assert
    EXPECT_NE(info, nullptr);
}

TEST_F(test_monthview, getScheduleInfo_swapped)
{
    // Arrange
    QDate begin = QDate::currentDate();
    QDate end = begin.addDays(-1);

    // Act
    DSchedule::Ptr info = mMonthView->getScheduleInfo(begin, end);

    // Assert
    EXPECT_NE(info, nullptr);
}

// CMonthView::isDragging
TEST_F(test_monthview, isDragging)
{
    // Arrange
    // Act
    bool result = mMonthView->isDragging();

    // Assert
    SUCCEED();
}
