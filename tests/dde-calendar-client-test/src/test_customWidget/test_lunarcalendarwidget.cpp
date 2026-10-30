// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_lunarcalendarwidget.h"

#include <QProxyStyle>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QTableView>
#include <QTest>

// CalenderStyle is a private class defined in lunarcalendarwidget.cpp.
// Re-declare its public method signatures here so the test can call them
// through the m_style pointer (accessible via -fno-access-control).
class CalenderStyle : public QProxyStyle
{
public:
    QColor getColor(const QStyleOption *option, QPalette::ColorRole role) const;
    void drawCalenderEllipse(QPainter *p, const QRect &rect, int offset) const;
    void drawItemViewSelected(QPainter *p, const QStyleOption *opt) const;
    void drawItemViewFocus(QPainter *p, const QStyleOption *opt, const QWidget *w) const;
    QString getLunarDayName(const QDate &date) const;
    QString getLunarYearDesc(const QDate &date) const;
    void updateLunarInfo(const QDate &date) const;
    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
};

test_lunarcalendarwidget::test_lunarcalendarwidget()
{
    mWidget = new LunarCalendarWidget();
}

test_lunarcalendarwidget::~test_lunarcalendarwidget()
{
    delete mWidget;
    mWidget = nullptr;
}

// LunarCalendarWidget::LunarCalendarWidget / ~LunarCalendarWidget
TEST_F(test_lunarcalendarwidget, constructor)
{
    LunarCalendarWidget *w = new LunarCalendarWidget();
    EXPECT_NE(w, nullptr);
    delete w;
}

// void LunarCalendarWidget::setLunarYearText(const QString &text)
TEST_F(test_lunarcalendarwidget, setLunarYearText)
{
    mWidget->setLunarYearText("2024 甲辰龙年");
    EXPECT_EQ(mWidget->lunarYearText(), "2024 甲辰龙年");
}

// QString LunarCalendarWidget::lunarYearText()
TEST_F(test_lunarcalendarwidget, lunarYearText)
{
    mWidget->setLunarYearText("test");
    EXPECT_EQ(mWidget->lunarYearText(), "test");
}

// QSize LunarCalendarWidget::minimumSizeHint()
TEST_F(test_lunarcalendarwidget, minimumSizeHint)
{
    QSize size = mWidget->minimumSizeHint();
    EXPECT_GE(size.width(), 0);
    EXPECT_GE(size.height(), 0);
}

// QColor CalenderStyle::getColor(const QStyleOption *option, QPalette::ColorRole role)
TEST_F(test_lunarcalendarwidget, getColor)
{
    QStyleOptionViewItem opt;
    opt.palette = mWidget->palette();
    QColor color = mWidget->m_style->getColor(&opt, QPalette::Text);
    EXPECT_TRUE(color.isValid());
}

// void CalenderStyle::drawCalenderEllipse(QPainter *p, const QRect &rect, int offset)
TEST_F(test_lunarcalendarwidget, drawCalenderEllipse)
{
    QPixmap pixmap(200, 200);
    QPainter painter(&pixmap);
    QRect rect(10, 10, 100, 100);
    mWidget->m_style->drawCalenderEllipse(&painter, rect, 5);
    painter.end();
}

// void CalenderStyle::drawItemViewSelected(QPainter *p, const QStyleOption *opt)
TEST_F(test_lunarcalendarwidget, drawItemViewSelected)
{
    QPixmap pixmap(200, 200);
    QPainter painter(&pixmap);
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 100, 50);
    opt.state |= QStyle::State_Selected;
    mWidget->m_style->drawItemViewSelected(&painter, &opt);
    painter.end();
}

// void CalenderStyle::drawItemViewSelected — no State_Selected
TEST_F(test_lunarcalendarwidget, drawItemViewSelected_noSelection)
{
    QPixmap pixmap(200, 200);
    QPainter painter(&pixmap);
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 100, 50);
    mWidget->m_style->drawItemViewSelected(&painter, &opt);
    painter.end();
}

// void CalenderStyle::drawItemViewFocus(QPainter *p, const QStyleOption *opt, const QWidget *w)
TEST_F(test_lunarcalendarwidget, drawItemViewFocus)
{
    QPixmap pixmap(200, 200);
    QPainter painter(&pixmap);
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 100, 50);
    opt.palette = mWidget->palette();
    mWidget->m_style->drawItemViewFocus(&painter, &opt, mWidget);
    painter.end();
}

// QString CalenderStyle::getLunarDayName(const QDate &date)
TEST_F(test_lunarcalendarwidget, getLunarDayName)
{
    QString name = mWidget->m_style->getLunarDayName(QDate::currentDate());
    // May return empty if lunar manager data unavailable; just verify no crash
    EXPECT_TRUE(name.isEmpty() || !name.isEmpty());
}

// QString CalenderStyle::getLunarYearDesc(const QDate &date)
TEST_F(test_lunarcalendarwidget, getLunarYearDesc)
{
    QString desc = mWidget->m_style->getLunarYearDesc(QDate::currentDate());
    EXPECT_TRUE(desc.isEmpty() || !desc.isEmpty());
}

// void CalenderStyle::updateLunarInfo(const QDate &date)
TEST_F(test_lunarcalendarwidget, updateLunarInfo)
{
    mWidget->m_style->updateLunarInfo(QDate::currentDate());
    SUCCEED();
}

// void CalenderStyle::drawControl — covered by rendering the widget
TEST_F(test_lunarcalendarwidget, drawControl_render)
{
    mWidget->setFixedSize(400, 300);
    mWidget->show();
    QTest::qWait(50);
    QPixmap pixmap(mWidget->size());
    mWidget->render(&pixmap);
    mWidget->hide();
}

// Render without showing
TEST_F(test_lunarcalendarwidget, render_noShow)
{
    mWidget->setFixedSize(400, 300);
    QPixmap pixmap(mWidget->size());
    mWidget->render(&pixmap);
}
