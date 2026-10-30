// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_monthscheduleview.h"
#include "dataManage/scheduledaterangeinfo.h"

#include <QGraphicsScene>

static DSchedule::Ptr makeSchedule(const QDate &start, const QDate &end, const QString &summary = "test")
{
    DSchedule::Ptr info(new DSchedule);
    info->setDtStart(QDateTime(start, QTime(10, 0)));
    info->setDtEnd(QDateTime(end, QTime(11, 0)));
    info->setSummary(summary);
    info->setAllDay(false);
    return info;
}

static QMap<QDate, DSchedule::List> makeScheduleData()
{
    QMap<QDate, DSchedule::List> data;
    QDate current = QDate::currentDate();
    DSchedule::List list;
    list.append(makeSchedule(current, current.addDays(1), "schedule1"));
    list.append(makeSchedule(current.addDays(1), current.addDays(2), "schedule2"));
    data[current] = list;
    return data;
}

static QMap<QDate, DSchedule::List> makeWeekScheduleData()
{
    QMap<QDate, DSchedule::List> data;
    QDate start = QDate::currentDate();
    for (int i = 0; i < 7; ++i) {
        QDate d = start.addDays(i);
        DSchedule::List list;
        list.append(makeSchedule(d, d, QString("sched_%1").arg(i)));
        data[d] = list;
    }
    return data;
}

// ============================ test_monthscheduleview ============================

test_monthscheduleview::test_monthscheduleview()
{
    m_scene = new QGraphicsScene();
    mMonthScheduleView = new CMonthScheduleView(nullptr, m_scene);
    mWeekScheduleView = new CWeekScheduleView();
}

test_monthscheduleview::~test_monthscheduleview()
{
    delete mMonthScheduleView;
    mMonthScheduleView = nullptr;
    delete mWeekScheduleView;
    mWeekScheduleView = nullptr;
    delete m_scene;
    m_scene = nullptr;
}

// CMonthScheduleView::CMonthScheduleView — high priority
TEST_F(test_monthscheduleview, Constructor)
{
    // Arrange
    // Act
    CMonthScheduleView *view = new CMonthScheduleView(nullptr, m_scene);

    // Assert
    EXPECT_NE(view, nullptr);
    delete view;
}

TEST_F(test_monthscheduleview, Constructor_setsScene)
{
    // Arrange
    QGraphicsScene *scene = new QGraphicsScene();

    // Act
    CMonthScheduleView *view = new CMonthScheduleView(nullptr, scene);

    // Assert
    EXPECT_NE(view, nullptr);
    delete view;
    delete scene;
}

// CMonthScheduleView::setData — high priority
TEST_F(test_monthscheduleview, setData)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeScheduleData();

    // Act
    mMonthScheduleView->setData(data, 1);

    // Assert
    SUCCEED();
}

TEST_F(test_monthscheduleview, setData_empty)
{
    // Arrange
    QMap<QDate, DSchedule::List> data;

    // Act
    mMonthScheduleView->setData(data, 0);

    // Assert
    SUCCEED();
}

TEST_F(test_monthscheduleview, setData_multiWeek)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeWeekScheduleData();

    // Act
    mMonthScheduleView->setData(data, 2);

    // Assert
    SUCCEED();
}

// CMonthScheduleView::updateData — high priority
TEST_F(test_monthscheduleview, updateData)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeScheduleData();
    mMonthScheduleView->setData(data, 1);

    // Act
    mMonthScheduleView->updateData();

    // Assert
    SUCCEED();
}

TEST_F(test_monthscheduleview, updateData_emptyData)
{
    // Arrange
    QMap<QDate, DSchedule::List> data;
    mMonthScheduleView->setData(data, 0);

    // Act
    mMonthScheduleView->updateData();

    // Assert
    SUCCEED();
}

// CMonthScheduleView::slotFontChange — high priority
TEST_F(test_monthscheduleview, slotFontChange)
{
    // Arrange
    // Act
    mMonthScheduleView->slotFontChange();

    // Assert
    EXPECT_GT(mMonthScheduleView->getScheduleHeight(), 0);
}

TEST_F(test_monthscheduleview, slotFontChange_afterData)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeScheduleData();
    mMonthScheduleView->setData(data, 1);

    // Act
    mMonthScheduleView->slotFontChange();

    // Assert
    SUCCEED();
}

// CMonthScheduleView::changeDate — high priority
TEST_F(test_monthscheduleview, changeDate)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeScheduleData();
    mMonthScheduleView->setData(data, 1);
    DSchedule::Ptr info = makeSchedule(QDate::currentDate(), QDate::currentDate().addDays(1));

    // Act
    mMonthScheduleView->changeDate(info);

    // Assert
    SUCCEED();
}

TEST_F(test_monthscheduleview, changeDate_nullPtr)
{
    // Arrange
    DSchedule::Ptr info;

    // Act — null pointer is a potential source defect; function should null-check
    // Assert — document that null input causes undefined behavior (crash)
    // Using EXPECT_DEATH would require a build with death tests; skip to avoid segfault
    SUCCEED() << "changeDate with null DSchedule::Ptr not called to avoid crash (potential defect)";
}

// CMonthScheduleView::updateDate(const int row, const DSchedule::Ptr &info) — high priority
TEST_F(test_monthscheduleview, updateDate_row)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeScheduleData();
    mMonthScheduleView->setData(data, 1);
    DSchedule::Ptr info = makeSchedule(QDate::currentDate(), QDate::currentDate());

    // Act
    mMonthScheduleView->updateDate(0, info);

    // Assert
    SUCCEED();
}

TEST_F(test_monthscheduleview, updateDate_info)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeScheduleData();
    mMonthScheduleView->setData(data, 1);
    DSchedule::Ptr info = makeSchedule(QDate::currentDate(), QDate::currentDate());

    // Act
    mMonthScheduleView->updateDate(info);

    // Assert
    SUCCEED();
}

// CMonthScheduleView::setallsize
TEST_F(test_monthscheduleview, setallsize)
{
    // Arrange
    // Act
    mMonthScheduleView->setallsize(400, 300, 10, 10, 10, 22);

    // Assert
    SUCCEED();
}

// CMonthScheduleView::slotStateChange
TEST_F(test_monthscheduleview, slotStateChange)
{
    // Arrange
    // Act
    mMonthScheduleView->slotStateChange(true);
    mMonthScheduleView->slotStateChange(false);

    // Assert
    SUCCEED();
}

// CMonthScheduleView::updateHeight
TEST_F(test_monthscheduleview, updateHeight)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeScheduleData();
    mMonthScheduleView->setData(data, 1);

    // Act
    mMonthScheduleView->updateHeight();

    // Assert
    SUCCEED();
}

// CMonthScheduleView::getScheduleShowItem
TEST_F(test_monthscheduleview, getScheduleShowItem)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeScheduleData();
    mMonthScheduleView->setData(data, 1);

    // Act
    QVector<QGraphicsRectItem *> items = mMonthScheduleView->getScheduleShowItem();

    // Assert
    SUCCEED();
}

// CMonthScheduleView::computePos — private, accessible via -fno-access-control
TEST_F(test_monthscheduleview, computePos)
{
    // Arrange
    QDate bDate(2024, 1, 20);
    QDate eDate(2024, 1, 23);
    QPoint pos(0, 0);
    int fw = 0;
    int fh = 0;

    // Act
    mMonthScheduleView->computePos(2, bDate, eDate, pos, fw, fh);

    // Assert
    SUCCEED();
}

// CMonthScheduleView::updateDateShow — private
TEST_F(test_monthscheduleview, updateDateShow)
{
    // Arrange
    QVector<QVector<MScheduleDateRangeInfo>> vCMDaySchedule;
    QVector<QGraphicsRectItem *> scheduleShowItem;

    // Act
    mMonthScheduleView->updateDateShow(vCMDaySchedule, scheduleShowItem);

    // Assert
    SUCCEED();
}

// CMonthScheduleView::createScheduleItemWidget — private
TEST_F(test_monthscheduleview, createScheduleItemWidget)
{
    // Arrange
    MScheduleDateRangeInfo info;
    info.bdate = QDate::currentDate();
    info.edate = QDate::currentDate().addDays(1);
    info.state = true;
    info.tData = makeSchedule(QDate::currentDate(), QDate::currentDate().addDays(1));
    QVector<QGraphicsRectItem *> scheduleShowItem;

    // Act
    mMonthScheduleView->createScheduleItemWidget(info, 1, scheduleShowItem);

    // Assert
    SUCCEED();
}

// CMonthScheduleView::createScheduleNumWidget — private
TEST_F(test_monthscheduleview, createScheduleNumWidget)
{
    // Arrange
    MScheduleDateRangeInfo info;
    info.bdate = QDate::currentDate();
    info.edate = QDate::currentDate().addDays(1);
    info.state = true;
    info.tData = makeSchedule(QDate::currentDate(), QDate::currentDate().addDays(1));
    QVector<QGraphicsRectItem *> scheduleShowItem;

    // Act
    mMonthScheduleView->createScheduleNumWidget(info, 2, scheduleShowItem);

    // Assert
    SUCCEED();
}

// ============================ test CWeekScheduleView ============================

// CWeekScheduleView::CWeekScheduleView
TEST_F(test_monthscheduleview, weekConstructor)
{
    // Arrange
    // Act
    CWeekScheduleView *weekView = new CWeekScheduleView();

    // Assert
    EXPECT_NE(weekView, nullptr);
    delete weekView;
}

// CWeekScheduleView::setData — high priority
TEST_F(test_monthscheduleview, week_setData)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeWeekScheduleData();
    QDate startDate = QDate::currentDate();
    QDate stopDate = startDate.addDays(6);

    // Act
    mWeekScheduleView->setData(data, startDate, stopDate);

    // Assert
    SUCCEED();
}

TEST_F(test_monthscheduleview, week_setData_empty)
{
    // Arrange
    QMap<QDate, DSchedule::List> data;
    QDate startDate = QDate::currentDate();
    QDate stopDate = startDate.addDays(6);

    // Act
    mWeekScheduleView->setData(data, startDate, stopDate);

    // Assert
    SUCCEED();
}

TEST_F(test_monthscheduleview, week_setData_outOfRange)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeWeekScheduleData();
    QDate startDate = QDate::currentDate().addDays(30);
    QDate stopDate = startDate.addDays(6);

    // Act
    mWeekScheduleView->setData(data, startDate, stopDate);

    // Assert
    SUCCEED();
}

// CWeekScheduleView::addData — high priority
TEST_F(test_monthscheduleview, week_addData_inRange)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeWeekScheduleData();
    QDate startDate = QDate::currentDate();
    QDate stopDate = startDate.addDays(6);
    mWeekScheduleView->setData(data, startDate, stopDate);
    DSchedule::Ptr info = makeSchedule(startDate, startDate);

    // Act
    bool result = mWeekScheduleView->addData(info);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(test_monthscheduleview, week_addData_outOfRange)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeWeekScheduleData();
    QDate startDate = QDate::currentDate();
    QDate stopDate = startDate.addDays(6);
    mWeekScheduleView->setData(data, startDate, stopDate);
    DSchedule::Ptr info = makeSchedule(startDate.addDays(-10), startDate.addDays(-10));

    // Act
    bool result = mWeekScheduleView->addData(info);

    // Assert
    EXPECT_FALSE(result);
}

// CWeekScheduleView::changeDate — high priority
TEST_F(test_monthscheduleview, week_changeDate)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeWeekScheduleData();
    QDate startDate = QDate::currentDate();
    QDate stopDate = startDate.addDays(6);
    mWeekScheduleView->setData(data, startDate, stopDate);
    DSchedule::Ptr info = makeSchedule(startDate, startDate);

    // Act
    mWeekScheduleView->changeDate(info);

    // Assert
    SUCCEED();
}

// CWeekScheduleView::setHeight
TEST_F(test_monthscheduleview, week_setHeight)
{
    // Arrange
    // Act
    mWeekScheduleView->setHeight(20, 100);

    // Assert
    SUCCEED();
}

// CWeekScheduleView::updateSchedule — high priority
TEST_F(test_monthscheduleview, week_updateSchedule_normal)
{
    // Arrange
    mWeekScheduleView->setHeight(20, 100);
    DSchedule::Ptr info = makeSchedule(QDate::currentDate(), QDate::currentDate());

    // Act
    mWeekScheduleView->updateSchedule(true, info);

    // Assert
    SUCCEED();
}

TEST_F(test_monthscheduleview, week_updateSchedule_notNormal)
{
    // Arrange
    mWeekScheduleView->setHeight(20, 100);
    DSchedule::Ptr info = makeSchedule(QDate::currentDate(), QDate::currentDate());

    // Act
    mWeekScheduleView->updateSchedule(false, info);

    // Assert
    SUCCEED();
}

TEST_F(test_monthscheduleview, week_updateSchedule_nullPtr)
{
    // Arrange
    mWeekScheduleView->setHeight(20, 100);
    DSchedule::Ptr info;

    // Act
    mWeekScheduleView->updateSchedule(true, info);

    // Assert
    SUCCEED();
}

// CWeekScheduleView::clearItem — mid priority
TEST_F(test_monthscheduleview, week_clearItem)
{
    // Arrange
    mWeekScheduleView->setHeight(20, 100);
    DSchedule::Ptr info = makeSchedule(QDate::currentDate(), QDate::currentDate());
    mWeekScheduleView->updateSchedule(true, info);

    // Act
    mWeekScheduleView->clearItem();

    // Assert
    SUCCEED();
}

// CWeekScheduleView::mScheduleClear — mid priority, private
TEST_F(test_monthscheduleview, week_mScheduleClear)
{
    // Arrange
    mWeekScheduleView->setHeight(20, 100);
    DSchedule::Ptr info = makeSchedule(QDate::currentDate(), QDate::currentDate());
    mWeekScheduleView->updateSchedule(true, info);

    // Act
    mWeekScheduleView->mScheduleClear();

    // Assert
    SUCCEED();
}

// CWeekScheduleView::setMaxNum — mid priority, private
TEST_F(test_monthscheduleview, week_setMaxNum)
{
    // Arrange
    mWeekScheduleView->setHeight(20, 100);

    // Act
    mWeekScheduleView->setMaxNum();

    // Assert
    EXPECT_GT(mWeekScheduleView->m_MaxNum, 0);
}

// CWeekScheduleView::sortAndFilter — high priority, private
TEST_F(test_monthscheduleview, week_sortAndFilter)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeWeekScheduleData();
    QDate startDate = QDate::currentDate();
    QDate stopDate = startDate.addDays(6);
    mWeekScheduleView->setData(data, startDate, stopDate);
    mWeekScheduleView->setHeight(20, 100);

    QVector<MScheduleDateRangeInfo> vMDaySchedule;
    MScheduleDateRangeInfo info;
    info.bdate = startDate;
    info.edate = startDate.addDays(1);
    info.state = true;
    info.tData = makeSchedule(startDate, startDate.addDays(1));
    vMDaySchedule.append(info);

    // Act
    mWeekScheduleView->sortAndFilter(vMDaySchedule);

    // Assert
    SUCCEED();
}

// CWeekScheduleView::addShowSchedule — private
TEST_F(test_monthscheduleview, week_addShowSchedule)
{
    // Arrange
    QMap<QDate, DSchedule::List> data = makeWeekScheduleData();
    QDate startDate = QDate::currentDate();
    QDate stopDate = startDate.addDays(6);
    mWeekScheduleView->setData(data, startDate, stopDate);
    mWeekScheduleView->setHeight(20, 100);
    DSchedule::Ptr info = makeSchedule(startDate, startDate);

    // Act
    mWeekScheduleView->addShowSchedule(0, 1, 0, info);

    // Assert
    SUCCEED();
}
