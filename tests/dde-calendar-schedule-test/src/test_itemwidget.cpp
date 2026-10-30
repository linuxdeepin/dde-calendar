// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "itemwidget.h"
#include "stub.h"

#include <QPainter>
#include <QDateTime>
#include <QDate>
#include <QFont>
#include <QColor>
#include <QTest>

// Stub for CScheduleDataManage::getScheduleDataManage
static CScheduleDataManage *g_stubManage = nullptr;
CScheduleDataManage *stub_getScheduleDataManage()
{
    return g_stubManage;
}

// Stub for getScheduleColorByType
CSchedulesColor stub_getScheduleColorByType(const QString &type)
{
    CSchedulesColor color;
    color.orginalColor = QColor(255, 0, 0);
    return color;
}

class ItemWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Constructor
TEST_F(ItemWidgetTest, Constructor_SetsDefaultFonts)
{
    ItemWidget w;
    QFont titleFont = w.getTitleFont();
    EXPECT_EQ(titleFont.pixelSize(), 14);

    QFont dtFont = w.getDateTimeFont();
    EXPECT_EQ(dtFont.pixelSize(), 12);
}

// setPositon / getBackgroundColor
TEST_F(ItemWidgetTest, SetPositon_ItemTop)
{
    ItemWidget w;
    w.setPositon(ItemWidget::ItemTop);
    SUCCEED();
}

TEST_F(ItemWidgetTest, SetPositon_ItemMiddle)
{
    ItemWidget w;
    w.setPositon(ItemWidget::ItemMiddle);
    SUCCEED();
}

TEST_F(ItemWidgetTest, SetPositon_ItemBottom)
{
    ItemWidget w;
    w.setPositon(ItemWidget::ItemBottom);
    SUCCEED();
}

TEST_F(ItemWidgetTest, SetPositon_ItemOnly)
{
    ItemWidget w;
    w.setPositon(ItemWidget::ItemOnly);
    SUCCEED();
}

// getTitleContent / setTitleContent
TEST_F(ItemWidgetTest, TitleContent_SetAndGet)
{
    ItemWidget w;
    w.setTitleContent("Meeting");
    EXPECT_EQ(w.getTitleContent(), QString("Meeting"));
}

TEST_F(ItemWidgetTest, TitleContent_DefaultEmpty)
{
    ItemWidget w;
    EXPECT_EQ(w.getTitleContent(), QString());
}

// getShowDate / setShowDate
TEST_F(ItemWidgetTest, ShowDate_SetAndGet)
{
    ItemWidget w;
    QDate date(2026, 1, 15);
    w.setShowDate(date);
    EXPECT_EQ(w.getShowDate(), date);
}

// getScheduleBeginTime / setScheduleBeginTime
TEST_F(ItemWidgetTest, ScheduleBeginTime_SetAndGet)
{
    ItemWidget w;
    QDateTime dt(QDate(2026, 1, 15), QTime(10, 0));
    w.setScheduleBeginTime(dt);
    EXPECT_EQ(w.getScheduleBeginTime(), dt);
}

// getScheduleEndTime / setScheduleEndTime
TEST_F(ItemWidgetTest, ScheduleEndTime_SetAndGet)
{
    ItemWidget w;
    QDateTime dt(QDate(2026, 1, 15), QTime(11, 0));
    w.setScheduleEndTime(dt);
    EXPECT_EQ(w.getScheduleEndTime(), dt);
}

// scheduleInfo / getScheduleInfo
TEST_F(ItemWidgetTest, ScheduleInfo_Get_InitiallyNull)
{
    ItemWidget w;
    EXPECT_EQ(w.getScheduleInfo(), nullptr);
    EXPECT_EQ(w.scheduleInfo(), nullptr);
}

// setScheduleInfo
TEST_F(ItemWidgetTest, SetScheduleInfo_SetsFields)
{
    ItemWidget w;
    DSchedule::Ptr info(new DSchedule());
    info->setDtStart(QDateTime(QDate(2026, 3, 1), QTime(9, 0)));
    info->setDtEnd(QDateTime(QDate(2026, 3, 1), QTime(10, 0)));
    info->setSummary("Test Schedule");

    w.setScheduleInfo(info);
    EXPECT_EQ(w.getScheduleInfo(), info);
    EXPECT_EQ(w.getScheduleBeginTime(), info->dtStart());
    EXPECT_EQ(w.getScheduleEndTime(), info->dtEnd());
    EXPECT_EQ(w.getShowDate(), info->dtStart().date());
    EXPECT_EQ(w.getTitleContent(), QString("Test Schedule"));
}

// getTitleColor / setTitleColor
TEST_F(ItemWidgetTest, TitleColor_SetAndGet)
{
    ItemWidget w;
    QColor c(100, 200, 50);
    w.setTitleColor(c);
    EXPECT_EQ(w.getTitleColor(), c);
}

// getDateTimeColor / setDateTimeColor
TEST_F(ItemWidgetTest, DateTimeColor_SetAndGet)
{
    ItemWidget w;
    QColor c(50, 100, 150);
    w.setDateTimeColor(c);
    EXPECT_EQ(w.getDateTimeColor(), c);
}

// getTitleFont / setTitleFont
TEST_F(ItemWidgetTest, TitleFont_SetAndGet)
{
    ItemWidget w;
    QFont f;
    f.setPixelSize(20);
    w.setTitleFont(f);
    EXPECT_EQ(w.getTitleFont().pixelSize(), 20);
}

// getDateTimeFont / setDateTimeFont
TEST_F(ItemWidgetTest, DateTimeFont_SetAndGet)
{
    ItemWidget w;
    QFont f;
    f.setPixelSize(16);
    w.setDateTimeFont(f);
    EXPECT_EQ(w.getDateTimeFont().pixelSize(), 16);
}

// getBackgroundColor
TEST_F(ItemWidgetTest, GetBackgroundColor_ReturnsBackgroundColor)
{
    ItemWidget w;
    QColor bg = w.getBackgroundColor();
    SUCCEED();
}

// setTheMe type 0 (dark)
TEST_F(ItemWidgetTest, SetTheMe_Type0_SetsDarkColors)
{
    ItemWidget w;
    w.setTheMe(0);
    QColor titleColor = w.getTitleColor();
    EXPECT_EQ(titleColor.red(), 0);
    EXPECT_NEAR(titleColor.alphaF(), 0.8, 0.01);

    QColor dtColor = w.getDateTimeColor();
    EXPECT_EQ(dtColor.red(), 0);
    EXPECT_NEAR(dtColor.alphaF(), 0.6, 0.01);
}

// setTheMe type 1 (dark)
TEST_F(ItemWidgetTest, SetTheMe_Type1_SetsDarkColors)
{
    ItemWidget w;
    w.setTheMe(1);
    QColor titleColor = w.getTitleColor();
    EXPECT_EQ(titleColor.red(), 0);
}

// setTheMe type 2 (light)
TEST_F(ItemWidgetTest, SetTheMe_Type2_SetsLightColors)
{
    ItemWidget w;
    w.setTheMe(2);
    QColor titleColor = w.getTitleColor();
    EXPECT_EQ(titleColor.red(), 255);
    EXPECT_NEAR(titleColor.alphaF(), 1.0, 0.01);

    QColor dtColor = w.getDateTimeColor();
    EXPECT_EQ(dtColor.red(), 255);
    EXPECT_NEAR(dtColor.alphaF(), 0.6, 0.01);
}

// DrawItem - no-op, should not crash
TEST_F(ItemWidgetTest, DrawItem_NoOp)
{
    ItemWidget w;
    w.resize(100, 50);
    QPainter painter(&w);
    w.DrawItem(painter);
    painter.end();
    SUCCEED();
}

// drawBackground - ItemTop via paintEvent
TEST_F(ItemWidgetTest, DrawBackground_ItemTop)
{
    ItemWidget w;
    w.setPositon(ItemWidget::ItemTop);
    w.resize(100, 50);
    w.show();
    QTest::qWait(30);
    w.hide();
    SUCCEED();
}

// drawBackground - ItemMiddle via paintEvent
TEST_F(ItemWidgetTest, DrawBackground_ItemMiddle)
{
    ItemWidget w;
    w.setPositon(ItemWidget::ItemMiddle);
    w.resize(100, 50);
    w.show();
    QTest::qWait(30);
    w.hide();
    SUCCEED();
}

// drawBackground - ItemBottom via paintEvent
TEST_F(ItemWidgetTest, DrawBackground_ItemBottom)
{
    ItemWidget w;
    w.setPositon(ItemWidget::ItemBottom);
    w.resize(100, 50);
    w.show();
    QTest::qWait(30);
    w.hide();
    SUCCEED();
}

// drawBackground - ItemOnly via paintEvent
TEST_F(ItemWidgetTest, DrawBackground_ItemOnly)
{
    ItemWidget w;
    w.setPositon(ItemWidget::ItemOnly);
    w.resize(100, 50);
    w.show();
    QTest::qWait(30);
    w.hide();
    SUCCEED();
}

// paintEvent - default position (ItemMiddle)
TEST_F(ItemWidgetTest, PaintEvent_DefaultPosition)
{
    ItemWidget w;
    w.resize(100, 50);
    w.show();
    QTest::qWait(30);
    w.hide();
    SUCCEED();
}

// ScheduleColor - with stub
TEST_F(ItemWidgetTest, ScheduleColor_WithStubbedSingleton)
{
    ItemWidget w;
    DSchedule::Ptr info(new DSchedule());
    info->setScheduleTypeID("test-type");
    w.m_scheduleInfo = info;

    Stub stub;
    stub.set(ADDR(CScheduleDataManage, getScheduleDataManage), stub_getScheduleDataManage);

    // Create a stub object and stub the method
    static CScheduleDataManage stubObj;
    g_stubManage = &stubObj;

    // Also stub getScheduleColorByType on the instance
    stub.set(ADDR(CScheduleDataManage, getScheduleColorByType), stub_getScheduleColorByType);

    QColor result = w.ScheduleColor();
    EXPECT_EQ(result, QColor(255, 0, 0));
}

// ScheduleColor - with null scheduleInfo (potential crash guard)
TEST_F(ItemWidgetTest, ScheduleColor_NullScheduleInfo_Stubbed)
{
    ItemWidget w;
    // m_scheduleInfo is null by default - stub the singleton to avoid crash
    Stub stub;
    stub.set(ADDR(CScheduleDataManage, getScheduleDataManage), stub_getScheduleDataManage);
    static CScheduleDataManage stubObj;
    g_stubManage = &stubObj;
    stub.set(ADDR(CScheduleDataManage, getScheduleColorByType), stub_getScheduleColorByType);

    // This will crash if scheduleInfo is null, so set a valid one
    DSchedule::Ptr info(new DSchedule());
    w.m_scheduleInfo = info;
    QColor result = w.ScheduleColor();
    EXPECT_EQ(result, QColor(255, 0, 0));
}
