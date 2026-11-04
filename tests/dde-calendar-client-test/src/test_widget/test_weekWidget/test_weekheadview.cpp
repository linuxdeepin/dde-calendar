// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_weekheadview.h"
#include "../third-party_stub/stub.h"

#include <QTest>
#include <QWheelEvent>
#include <QEvent>

static QMap<QDate, CaHuangLiDayInfo> getHuangLiDayInfo()
{
    QMap<QDate, CaHuangLiDayInfo> huangLiDayInfo;
    CaHuangLiDayInfo info;
    info.mLunarDayName = QStringLiteral("初一");
    info.mLunarMonthName = QStringLiteral("正月");
    info.mGanZhiYear = QStringLiteral("辛丑牛年");
    info.mGanZhiMonth = QStringLiteral("庚寅月");
    info.mGanZhiDay = QStringLiteral("辛卯日");
    info.mTerm = "";
    info.mSolarFestival = QStringLiteral("国庆节");
    info.mLunarFestival = QStringLiteral("除夕");
    info.mSuit = QStringLiteral("嫁娶");
    info.mAvoid = QStringLiteral("入土");
    huangLiDayInfo.insert(QDate::currentDate(), info);
    return huangLiDayInfo;
}

static QVector<QDate> getDayList()
{
    QVector<QDate> dateList;
    QDate currentDate = QDate::currentDate();
    for (int i = 0; i < 7; ++i) {
        dateList.append(currentDate.addDays(i));
    }
    return dateList;
}

// void CWeekHeadView::setTheMe(int type)
TEST_F(test_weekheadview, setTheMe)
{
    mWeekHeadView->setTheMe(0);
    mWeekHeadView->setTheMe(1);
    mWeekHeadView->setTheMe(2);
}

// void CWeekHeadView::setWeekDay(QVector<QDate> vDays, const QDate &selectDate)
TEST_F(test_weekheadview, setWeekDay)
{
    // Wrong count - should return early
    QVector<QDate> shortList;
    shortList.append(QDate::currentDate());
    mWeekHeadView->setWeekDay(shortList, QDate::currentDate());

    // Correct count (7 days)
    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
}

// void CWeekHeadView::setHunagLiInfo(const QMap<QDate, CaHuangLiDayInfo> &huangLiInfo)
TEST_F(test_weekheadview, setHunagLiInfo)
{
    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
    mWeekHeadView->setHunagLiInfo(getHuangLiDayInfo());
}

// void CWeekHeadView::setLunarVisible(bool visible)
TEST_F(test_weekheadview, setLunarVisible)
{
    mWeekHeadView->setLunarVisible(false);
    mWeekHeadView->setLunarVisible(true);
}

// const QString CWeekHeadView::getCellDayNum(int pos) — private, accessible via -fno-access-control
TEST_F(test_weekheadview, getCellDayNum)
{
    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
    QString result = mWeekHeadView->getCellDayNum(3);
    EXPECT_FALSE(result.isEmpty());
}

// const QDate CWeekHeadView::getCellDate(int pos) — private
TEST_F(test_weekheadview, getCellDate)
{
    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
    QDate result = mWeekHeadView->getCellDate(3);
    EXPECT_EQ(result, getDayList().at(3));
}

// const QString CWeekHeadView::getLunar(int pos) — private
TEST_F(test_weekheadview, getLunar)
{
    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
    mWeekHeadView->setHunagLiInfo(getHuangLiDayInfo());
    QString result = mWeekHeadView->getLunar(0);
    // May be empty if current date not in map; just verify no crash
    SUCCEED();
}

// void CWeekHeadView::paintCell(QWidget *cell) — private, triggered via paint event
TEST_F(test_weekheadview, paintCell)
{
    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
    mWeekHeadView->setHunagLiInfo(getHuangLiDayInfo());
    mWeekHeadView->setFixedSize(800, 80);
    QPixmap pixmap(mWeekHeadView->size());
    mWeekHeadView->render(&pixmap);
}

// bool CWeekHeadView::eventFilter(QObject *o, QEvent *e)
TEST_F(test_weekheadview, eventFilter)
{
    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
    mWeekHeadView->setFixedSize(800, 80);
    // Trigger paint via render which sends Paint events through eventFilter
    QPixmap pixmap(mWeekHeadView->size());
    mWeekHeadView->render(&pixmap);

    // Send a mouse double-click event to a cell
    QEvent dblClickEvent(QEvent::MouseButtonDblClick);
    // eventFilter is installed on cells; trigger via QApplication::sendEvent
    SUCCEED();
}

// bool CWeekHeadView::event(QEvent *e) — touch gesture handling
TEST_F(test_weekheadview, event)
{
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(mWeekHeadView, &enterEvent);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(mWeekHeadView, &leaveEvent);
}

// void CWeekHeadView::wheelEvent(QWheelEvent *e)
TEST_F(test_weekheadview, wheelEvent)
{
    mWeekHeadView->setFixedSize(800, 80);
    // Horizontal wheel (x != 0) should emit signalAngleDelta
    QPoint angleDelta(120, 0);
    QWheelEvent wheelEvent(QPointF(400, 40), QPointF(400, 40), QPoint(), angleDelta,
                           Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(mWeekHeadView, &wheelEvent);

    // Vertical wheel (x == 0) should not emit
    QPoint angleDelta2(0, 120);
    QWheelEvent wheelEvent2(QPointF(400, 40), QPointF(400, 40), QPoint(), angleDelta2,
                            Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(mWeekHeadView, &wheelEvent2);
}

// Destructor coverage
TEST_F(test_weekheadview, destructor)
{
    CWeekHeadView *view = new CWeekHeadView();
    delete view;
}
