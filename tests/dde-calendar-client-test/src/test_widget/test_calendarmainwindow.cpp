// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calendarmainwindow.h"
#include "../third-party_stub/stub.h"
#include "../dialog_stub.h"
#include "dschedule.h"

#include <QMouseEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QShowEvent>
#include <QEvent>
#include <QTest>
#include <QApplication>
#include <QScreen>
#include <QAbstractButton>

test_calendarmainwindow::test_calendarmainwindow()
{
}

test_calendarmainwindow::~test_calendarmainwindow()
{
}

void test_calendarmainwindow::SetUp()
{
    mMainWindow = new Calendarmainwindow(0, nullptr);
}

void test_calendarmainwindow::TearDown()
{
    delete mMainWindow;
    mMainWindow = nullptr;
}

// Constructor / Destructor — exercised via SetUp/TearDown
TEST_F(test_calendarmainwindow, ConstructorDestructor)
{
    ASSERT_NE(mMainWindow, nullptr);
}

// initUI — called from constructor; call again is safe (re-init)
TEST_F(test_calendarmainwindow, initUI)
{
    mMainWindow->initUI();
    SUCCEED();
}

// initConnection
TEST_F(test_calendarmainwindow, initConnection)
{
    mMainWindow->initConnection();
    SUCCEED();
}

// initData
TEST_F(test_calendarmainwindow, initData)
{
    mMainWindow->initData();
    SUCCEED();
}

// initLater — deprecated, empty body
TEST_F(test_calendarmainwindow, initLater)
{
    mMainWindow->initLater();
    SUCCEED();
}

// startDeferredViewDataInit — guarded by m_deferredViewDataInitDone flag
TEST_F(test_calendarmainwindow, startDeferredViewDataInit)
{
    mMainWindow->startDeferredViewDataInit();
    // second call returns early
    mMainWindow->startDeferredViewDataInit();
    SUCCEED();
}

// createview
TEST_F(test_calendarmainwindow, createview)
{
    mMainWindow->createview();
    SUCCEED();
}

// createViewByIndex — all 4 indices
TEST_F(test_calendarmainwindow, createViewByIndex)
{
    mMainWindow->createViewByIndex(0);
    mMainWindow->createViewByIndex(1);
    mMainWindow->createViewByIndex(2);
    mMainWindow->createViewByIndex(3);
    // invalid index
    mMainWindow->createViewByIndex(99);
    SUCCEED();
}

// ensureYearWindow
TEST_F(test_calendarmainwindow, ensureYearWindow)
{
    auto *w = mMainWindow->ensureYearWindow();
    EXPECT_NE(w, nullptr);
    // second call returns existing
    auto *w2 = mMainWindow->ensureYearWindow();
    EXPECT_EQ(w, w2);
}

// ensureMonthWindow
TEST_F(test_calendarmainwindow, ensureMonthWindow)
{
    auto *w = mMainWindow->ensureMonthWindow();
    EXPECT_NE(w, nullptr);
    auto *w2 = mMainWindow->ensureMonthWindow();
    EXPECT_EQ(w, w2);
}

// ensureWeekWindow
TEST_F(test_calendarmainwindow, ensureWeekWindow)
{
    auto *w = mMainWindow->ensureWeekWindow();
    EXPECT_NE(w, nullptr);
    auto *w2 = mMainWindow->ensureWeekWindow();
    EXPECT_EQ(w, w2);
}

// ensureDayWindow
TEST_F(test_calendarmainwindow, ensureDayWindow)
{
    auto *w = mMainWindow->ensureDayWindow();
    EXPECT_NE(w, nullptr);
    auto *w2 = mMainWindow->ensureDayWindow();
    EXPECT_EQ(w, w2);
}

// viewWindow — all valid indices, with and without animation
TEST_F(test_calendarmainwindow, viewWindow)
{
    mMainWindow->viewWindow(0, false);
    mMainWindow->viewWindow(1, false);
    mMainWindow->viewWindow(2, false);
    mMainWindow->viewWindow(3, false);
    mMainWindow->viewWindow(1, true);
    // invalid index
    mMainWindow->viewWindow(-1, false);
    mMainWindow->viewWindow(99, false);
    SUCCEED();
}

// updateHeight
TEST_F(test_calendarmainwindow, updateHeight)
{
    mMainWindow->updateHeight();
    SUCCEED();
}

// setSearchWidth
TEST_F(test_calendarmainwindow, setSearchWidth)
{
    mMainWindow->setSearchWidth(200);
    mMainWindow->setSearchWidth(0);
    SUCCEED();
}

// resizeView
TEST_F(test_calendarmainwindow, resizeView)
{
    mMainWindow->resizeView();
    SUCCEED();
}

// slotTheme — light, dark, default
TEST_F(test_calendarmainwindow, slotTheme)
{
    mMainWindow->slotTheme(1);
    mMainWindow->slotTheme(2);
    mMainWindow->slotTheme(0);
    SUCCEED();
}

// slotOpenSchedule — empty and valid JSON
TEST_F(test_calendarmainwindow, slotOpenSchedule)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    mMainWindow->slotOpenSchedule("");
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary("test");
    schedule->setDtStart(QDateTime::currentDateTime());
    schedule->setDtEnd(QDateTime::currentDateTime().addSecs(3600));
    schedule->setScheduleTypeID("1");
    QString json;
    DSchedule::toJsonString(schedule, json);
    mMainWindow->slotOpenSchedule(json);
    SUCCEED();
}

// slotViewShortcut — creates QProcess
TEST_F(test_calendarmainwindow, slotViewShortcut)
{
    mMainWindow->slotViewShortcut();
    SUCCEED();
}

// slotCurrentDateUpdate
TEST_F(test_calendarmainwindow, slotCurrentDateUpdate)
{
    mMainWindow->slotCurrentDateUpdate();
    SUCCEED();
}

// slotSetSearchFocus
TEST_F(test_calendarmainwindow, slotSetSearchFocus)
{
    mMainWindow->slotSetSearchFocus();
    SUCCEED();
}

// slotapplicationStateChanged
TEST_F(test_calendarmainwindow, slotapplicationStateChanged)
{
    mMainWindow->slotapplicationStateChanged(Qt::ApplicationActive);
    mMainWindow->slotapplicationStateChanged(Qt::ApplicationInactive);
    mMainWindow->slotapplicationStateChanged(Qt::ApplicationHidden);
    SUCCEED();
}

// setScheduleHide
TEST_F(test_calendarmainwindow, setScheduleHide)
{
    mMainWindow->setScheduleHide();
    SUCCEED();
}

// slotstackWClicked
TEST_F(test_calendarmainwindow, slotstackWClicked)
{
    auto *btn = mMainWindow->findChild<QAbstractButton *>();
    if (btn) {
        mMainWindow->slotstackWClicked(btn);
    }
    SUCCEED();
}

// slotWUpdateSchedule
TEST_F(test_calendarmainwindow, slotWUpdateSchedule)
{
    mMainWindow->slotWUpdateSchedule();
    SUCCEED();
}

// slotSreturnPressed
TEST_F(test_calendarmainwindow, slotSreturnPressed)
{
    mMainWindow->slotSreturnPressed();
    SUCCEED();
}

// slotStextChanged
TEST_F(test_calendarmainwindow, slotStextChanged)
{
    mMainWindow->slotStextChanged();
    SUCCEED();
}

// slotStextfocusChanged
TEST_F(test_calendarmainwindow, slotStextfocusChanged)
{
    mMainWindow->slotStextfocusChanged(true);
    mMainWindow->slotStextfocusChanged(false);
    SUCCEED();
}

// slotSearchEdit
TEST_F(test_calendarmainwindow, slotSearchEdit)
{
    mMainWindow->slotSearchEdit();
    SUCCEED();
}

// slotSearchSelectSchedule
TEST_F(test_calendarmainwindow, slotSearchSelectSchedule)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary("test");
    mMainWindow->slotSearchSelectSchedule(schedule);
    SUCCEED();
}

// slotViewtransparentFrame
TEST_F(test_calendarmainwindow, slotViewtransparentFrame)
{
    mMainWindow->slotViewtransparentFrame(true);
    mMainWindow->slotViewtransparentFrame(false);
    SUCCEED();
}

// slotSetButtonBox
TEST_F(test_calendarmainwindow, slotSetButtonBox)
{
    mMainWindow->slotSetButtonBox();
    SUCCEED();
}

// slotSwitchView — all cases
TEST_F(test_calendarmainwindow, slotSwitchView)
{
    mMainWindow->slotSwitchView(0);
    mMainWindow->slotSwitchView(1);
    mMainWindow->slotSwitchView(2);
    mMainWindow->slotSwitchView(3);
    SUCCEED();
}

// slotNewSchedule — needs dialog exec stub
TEST_F(test_calendarmainwindow, slotNewSchedule)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    mMainWindow->slotNewSchedule();
    SUCCEED();
}

// slotOpenSettingDialog — needs dialog exec stub
TEST_F(test_calendarmainwindow, slotOpenSettingDialog)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    mMainWindow->slotOpenSettingDialog();
    SUCCEED();
}

// slotShowPrivacy
TEST_F(test_calendarmainwindow, slotShowPrivacy)
{
    mMainWindow->slotShowPrivacy();
    SUCCEED();
}

// slotDeleteitem
TEST_F(test_calendarmainwindow, slotDeleteitem)
{
    mMainWindow->slotDeleteitem();
    SUCCEED();
}

// slotSetMaxSize
TEST_F(test_calendarmainwindow, slotSetMaxSize)
{
    mMainWindow->slotSetMaxSize();
    SUCCEED();
}

// slotSearchFocusSwitch
TEST_F(test_calendarmainwindow, slotSearchFocusSwitch)
{
    mMainWindow->slotSearchFocusSwitch();
    SUCCEED();
}

// slotSidebarStatusChange
TEST_F(test_calendarmainwindow, slotSidebarStatusChange)
{
    mMainWindow->slotSidebarStatusChange(true);
    mMainWindow->slotSidebarStatusChange(false);
    SUCCEED();
}

// slotShowSyncToast
TEST_F(test_calendarmainwindow, slotShowSyncToast)
{
    mMainWindow->slotShowSyncToast(-1);
    mMainWindow->slotShowSyncToast(0);
    mMainWindow->slotShowSyncToast(1);
    mMainWindow->slotShowSyncToast(2);
    mMainWindow->slotShowSyncToast(3);
    SUCCEED();
}

// slotCalDavAccountStatusChanged
TEST_F(test_calendarmainwindow, slotCalDavAccountStatusChanged)
{
    mMainWindow->slotCalDavAccountStatusChanged("test-account-id");
    SUCCEED();
}

// slotCalDavScheduleCreateFailed
TEST_F(test_calendarmainwindow, slotCalDavScheduleCreateFailed)
{
    mMainWindow->slotCalDavScheduleCreateFailed("test-account-id", 0);
    mMainWindow->slotCalDavScheduleCreateFailed("test-account-id", 1);
    SUCCEED();
}

// removeSyncToast
TEST_F(test_calendarmainwindow, removeSyncToast)
{
    mMainWindow->removeSyncToast();
    mMainWindow->removeSyncToast(mMainWindow);
    SUCCEED();
}

// removeAllSyncToasts
TEST_F(test_calendarmainwindow, removeAllSyncToasts)
{
    mMainWindow->removeAllSyncToasts();
    SUCCEED();
}

// syncToastParent
TEST_F(test_calendarmainwindow, syncToastParent)
{
    auto *parent = mMainWindow->syncToastParent();
    EXPECT_NE(parent, nullptr);
}

// resizeEvent
TEST_F(test_calendarmainwindow, resizeEvent)
{
    QResizeEvent event(QSize(800, 600), QSize(400, 300));
    mMainWindow->resizeEvent(&event);
    SUCCEED();
}

// mouseMoveEvent
TEST_F(test_calendarmainwindow, mouseMoveEvent)
{
    QMouseEvent event(QEvent::MouseMove, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mMainWindow->mouseMoveEvent(&event);
    SUCCEED();
}

// changeEvent
TEST_F(test_calendarmainwindow, changeEvent)
{
    QEvent event(QEvent::ActivationChange);
    mMainWindow->changeEvent(&event);
    QEvent event2(QEvent::None);
    mMainWindow->changeEvent(&event2);
    SUCCEED();
}

// mousePressEvent
TEST_F(test_calendarmainwindow, mousePressEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mMainWindow->mousePressEvent(&event);
    SUCCEED();
}

// event — Move, WindowActivate, WindowDeactivate, None
TEST_F(test_calendarmainwindow, event)
{
    QEvent moveEvent(QEvent::Move);
    mMainWindow->event(&moveEvent);
    QEvent activateEvent(QEvent::WindowActivate);
    mMainWindow->event(&activateEvent);
    QEvent deactivateEvent(QEvent::WindowDeactivate);
    mMainWindow->event(&deactivateEvent);
    QEvent noneEvent(QEvent::None);
    mMainWindow->event(&noneEvent);
    SUCCEED();
}

// paintEvent
TEST_F(test_calendarmainwindow, paintEvent)
{
    QPaintEvent event(QRect(0, 0, 100, 100));
    mMainWindow->paintEvent(&event);
    SUCCEED();
}

// showEvent
TEST_F(test_calendarmainwindow, showEvent)
{
    QShowEvent event;
    mMainWindow->showEvent(&event);
    // second call should return early due to m_clientShowSyncScheduled
    mMainWindow->showEvent(&event);
    SUCCEED();
}
