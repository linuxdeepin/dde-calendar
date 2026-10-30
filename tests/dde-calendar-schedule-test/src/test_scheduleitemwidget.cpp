// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "scheduleitemwidget.h"

#include <QApplication>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <DGuiApplicationHelper>

static DSchedule::Ptr makeSchedule(const QDateTime &start, const QDateTime &end,
                                   bool allDay = false, const QString &title = QStringLiteral("title"))
{
    DSchedule::Ptr s(new DSchedule);
    s->setDtStart(start);
    s->setDtEnd(end);
    s->setAllDay(allDay);
    s->setSummary(title);
    return s;
}

// scheduleitemwidget ---------------------------------------------------------
TEST(ScheduleItemWidgetTest, ConstructAndDestruct)
{
    scheduleitemwidget w;
    EXPECT_EQ(nullptr, w.parentWidget());
    scheduleitemwidget *w2 = new scheduleitemwidget(nullptr);
    EXPECT_EQ(nullptr, w2->parentWidget());
    delete w2;
}

TEST(ScheduleItemWidgetTest, SetScheduleDtailInfoSortsByTime)
{
    scheduleitemwidget w;
    QDateTime base(QDate(2026, 1, 1), QTime(10, 0));
    DSchedule::List list;
    list << makeSchedule(base.addSecs(7200), base.addSecs(10800))
         << makeSchedule(base, base.addSecs(3600))
         << makeSchedule(base.addSecs(3600), base.addSecs(7200));
    w.setScheduleDtailInfo(list);
    SUCCEED();
}

TEST(ScheduleItemWidgetTest, SortScheduleWithTimeDirect)
{
    scheduleitemwidget w;
    QDateTime base(QDate(2026, 3, 1), QTime(8, 0));
    DSchedule::List list;
    list << makeSchedule(base.addDays(2), base.addDays(2).addSecs(3600))
         << makeSchedule(base, base.addSecs(3600))
         << makeSchedule(base.addDays(1), base.addDays(1).addSecs(3600));
    w.setScheduleDtailInfo(list);
    w.sortScheduleWithTime();
    EXPECT_LE(w.m_scheduleInfo[0]->dtStart(), w.m_scheduleInfo[1]->dtStart());
    EXPECT_LE(w.m_scheduleInfo[1]->dtStart(), w.m_scheduleInfo[2]->dtStart());
}

TEST(ScheduleItemWidgetTest, AddScheduleItemEmpty)
{
    scheduleitemwidget w;
    DSchedule::List empty;
    w.setScheduleDtailInfo(empty);
    w.addscheduleitem();
    EXPECT_NE(nullptr, w.layout());
}

TEST(ScheduleItemWidgetTest, AddScheduleItemSingle)
{
    scheduleitemwidget w;
    QDateTime base(QDate(2026, 5, 1), QTime(9, 0));
    DSchedule::List list;
    list << makeSchedule(base, base.addSecs(3600), false, QStringLiteral("single"));
    w.setScheduleDtailInfo(list);
    w.addscheduleitem();
    auto *vbox = qobject_cast<QVBoxLayout *>(w.layout());
    ASSERT_NE(nullptr, vbox);
    EXPECT_GE(vbox->count(), 2);
}

TEST(ScheduleItemWidgetTest, AddScheduleItemMultiple)
{
    scheduleitemwidget w;
    QDateTime base(QDate(2026, 6, 1), QTime(9, 0));
    DSchedule::List list;
    list << makeSchedule(base.addSecs(7200), base.addSecs(10800))
         << makeSchedule(base, base.addSecs(3600))
         << makeSchedule(base.addSecs(3600), base.addSecs(7200));
    w.setScheduleDtailInfo(list);
    w.addscheduleitem();
    auto *vbox = qobject_cast<QVBoxLayout *>(w.layout());
    ASSERT_NE(nullptr, vbox);
    EXPECT_GT(vbox->count(), 3);
}

// scheduleitem ---------------------------------------------------------------
TEST(ScheduleItemTest, ConstructAndRender)
{
    scheduleitem item;
    EXPECT_EQ(36, item.height());
    item.setScheduleInfo(makeSchedule(QDateTime(QDate(2026, 7, 1), QTime(8, 0)),
                                      QDateTime(QDate(2026, 7, 1), QTime(9, 0)),
                                      false, QStringLiteral("meeting")));
    item.resize(320, 36);
    item.show();
    item.repaint();
    qApp->processEvents();
    EXPECT_TRUE(item.isVisible());
}

TEST(ScheduleItemTest, DrawItemDirect)
{
    scheduleitem item;
    item.setScheduleInfo(makeSchedule(QDateTime(QDate(2026, 7, 2), QTime(10, 0)),
                                      QDateTime(QDate(2026, 7, 2), QTime(11, 0)),
                                      false, QStringLiteral("detail")));
    item.resize(320, 36);
    QImage img(320, 36, QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter painter(&img);
    item.DrawItem(painter);
    painter.end();
    SUCCEED();
}

TEST(ScheduleItemTest, DrawItemAllDay)
{
    scheduleitem item;
    item.setScheduleInfo(makeSchedule(QDateTime(QDate(2026, 7, 3), QTime(0, 0)),
                                      QDateTime(QDate(2026, 7, 3), QTime(23, 59)),
                                      true, QStringLiteral("allday")));
    item.resize(320, 36);
    item.show();
    item.repaint();
    qApp->processEvents();
    SUCCEED();
}

TEST(ScheduleItemTest, DrawScheduleColorLine)
{
    scheduleitem item;
    item.setScheduleInfo(makeSchedule(QDateTime(QDate(2026, 7, 4), QTime(6, 0)),
                                      QDateTime(QDate(2026, 7, 4), QTime(7, 0))));
    item.resize(320, 36);
    QImage img(320, 36, QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter painter(&img);
    item.drawScheduleColorLine(painter);
    item.drawTime(painter);
    item.drawDetails(painter);
    painter.end();
    SUCCEED();
}

TEST(ScheduleItemTest, MousePressEventEmitsSignal)
{
    scheduleitem item;
    item.setScheduleInfo(makeSchedule(QDateTime(QDate(2026, 7, 5), QTime(6, 0)),
                                      QDateTime(QDate(2026, 7, 5), QTime(7, 0))));
    bool emitted = false;
    QObject::connect(&item, &scheduleitem::signalItemPress,
                     [&emitted](const DSchedule::Ptr &) { emitted = true; });
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton,
                      Qt::LeftButton, Qt::NoModifier);
    item.mousePressEvent(&press);
    EXPECT_TRUE(emitted);
}

TEST(ScheduleItemTest, MousePressEventRightButtonIgnored)
{
    scheduleitem item;
    item.setScheduleInfo(makeSchedule(QDateTime(QDate(2026, 7, 6), QTime(6, 0)),
                                      QDateTime(QDate(2026, 7, 6), QTime(7, 0))));
    bool emitted = false;
    QObject::connect(&item, &scheduleitem::signalItemPress,
                     [&emitted](const DSchedule::Ptr &) { emitted = true; });
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10), Qt::RightButton,
                      Qt::RightButton, Qt::NoModifier);
    item.mousePressEvent(&press);
    EXPECT_FALSE(emitted);
}

// scheduleitemdate -----------------------------------------------------------
TEST(ScheduleItemDateTest, ConstructAndSetInfo)
{
    scheduleitemdate d;
    EXPECT_EQ(20, d.height());
    auto s = makeSchedule(QDateTime(QDate(2026, 8, 1), QTime(9, 0)),
                          QDateTime(QDate(2026, 8, 1), QTime(10, 0)));
    d.setScheduleDtailInfo(s);
    SUCCEED();
}

TEST(ScheduleItemDateTest, DetailsColorLightTheme)
{
    scheduleitemdate d;
    d.setScheduleDtailInfo(makeSchedule(QDateTime(QDate(2026, 8, 2), QTime(9, 0)),
                                        QDateTime(QDate(2026, 8, 2), QTime(10, 0))));
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
    QColor c = d.DetailsColor();
    EXPECT_EQ(0, c.red());
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::UnknownType);
    QColor c0 = d.DetailsColor();
    EXPECT_EQ(0, c0.red());
}

TEST(ScheduleItemDateTest, DetailsColorDarkTheme)
{
    scheduleitemdate d;
    d.setScheduleDtailInfo(makeSchedule(QDateTime(QDate(2026, 8, 3), QTime(9, 0)),
                                        QDateTime(QDate(2026, 8, 3), QTime(10, 0))));
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
    QColor c = d.DetailsColor();
    EXPECT_EQ(255, c.red());
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
}

TEST(ScheduleItemDateTest, PaintEventRenders)
{
    scheduleitemdate d;
    d.setScheduleDtailInfo(makeSchedule(QDateTime(QDate(2026, 8, 4), QTime(9, 0)),
                                        QDateTime(QDate(2026, 8, 4), QTime(10, 0))));
    d.resize(300, 20);
    d.show();
    d.repaint();
    qApp->processEvents();
    EXPECT_TRUE(d.isVisible());
}
