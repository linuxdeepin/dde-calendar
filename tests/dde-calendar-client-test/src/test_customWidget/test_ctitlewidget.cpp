// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ctitlewidget.h"

#include <QTest>
#include <QSignalSpy>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QEvent>
#include <DButtonBox>
#include <DSearchEdit>
#include <DIconButton>

test_ctitlewidget::test_ctitlewidget()
{

}

// Constructor: verify object creation
TEST_F(test_ctitlewidget, constructor)
{
    EXPECT_NE(mWidget, nullptr);
    EXPECT_NE(mWidget->m_sidebarIcon, nullptr);
    EXPECT_NE(mWidget->m_buttonBox, nullptr);
    EXPECT_NE(mWidget->m_searchEdit, nullptr);
    EXPECT_NE(mWidget->m_newScheduleBtn, nullptr);
    EXPECT_NE(mWidget->m_searchPush, nullptr);
}

// setShowState: Mini
TEST_F(test_ctitlewidget, setShowState_mini)
{
    mWidget->setShowState(CTitleWidget::Title_State_Mini);
    EXPECT_EQ(mWidget->m_showState, CTitleWidget::Title_State_Mini);
}

// setShowState: Normal
TEST_F(test_ctitlewidget, setShowState_normal)
{
    mWidget->setShowState(CTitleWidget::Title_State_Normal);
    EXPECT_EQ(mWidget->m_showState, CTitleWidget::Title_State_Normal);
}

// setSidebarStatus: true
TEST_F(test_ctitlewidget, setSidebarStatus_true)
{
    QSignalSpy spy(mWidget, &CTitleWidget::signalSidebarStatusChange);
    mWidget->setSidebarStatus(true);
    EXPECT_TRUE(mWidget->m_sidebarstatus);
    EXPECT_GE(spy.count(), 1);
}

// setSidebarStatus: false
TEST_F(test_ctitlewidget, setSidebarStatus_false)
{
    QSignalSpy spy(mWidget, &CTitleWidget::signalSidebarStatusChange);
    mWidget->setSidebarStatus(false);
    EXPECT_FALSE(mWidget->m_sidebarstatus);
    EXPECT_GE(spy.count(), 1);
}

// setSidebarCanDisplay: true
TEST_F(test_ctitlewidget, setSidebarCanDisplay_true)
{
    mWidget->setSidebarCanDisplay(true);
    EXPECT_TRUE(mWidget->m_sidebarCanDisplay);
}

// setSidebarCanDisplay: false
TEST_F(test_ctitlewidget, setSidebarCanDisplay_false)
{
    mWidget->setSidebarCanDisplay(false);
    EXPECT_FALSE(mWidget->m_sidebarCanDisplay);
}

// getSidevarStatus: returns m_sidebarstatus
TEST_F(test_ctitlewidget, getSidevarStatus)
{
    mWidget->m_sidebarstatus = true;
    EXPECT_TRUE(mWidget->getSidevarStatus());
    mWidget->m_sidebarstatus = false;
    EXPECT_FALSE(mWidget->getSidevarStatus());
}

// buttonBox: returns m_buttonBox
TEST_F(test_ctitlewidget, buttonBox)
{
    EXPECT_EQ(mWidget->buttonBox(), mWidget->m_buttonBox);
}

// searchEdit: returns m_searchEdit
TEST_F(test_ctitlewidget, searchEdit)
{
    EXPECT_EQ(mWidget->searchEdit(), mWidget->m_searchEdit);
}

// newScheduleBtn: returns m_newScheduleBtn
TEST_F(test_ctitlewidget, newScheduleBtn)
{
    EXPECT_EQ(mWidget->newScheduleBtn(), mWidget->m_newScheduleBtn);
}

// stateUpdate: Mini state
TEST_F(test_ctitlewidget, stateUpdate_mini)
{
    mWidget->setShowState(CTitleWidget::Title_State_Mini);
    mWidget->stateUpdate();
}

// stateUpdate: Normal state
TEST_F(test_ctitlewidget, stateUpdate_normal)
{
    mWidget->setShowState(CTitleWidget::Title_State_Normal);
    mWidget->stateUpdate();
}

// miniStateShowSearchEdit
TEST_F(test_ctitlewidget, miniStateShowSearchEdit)
{
    mWidget->miniStateShowSearchEdit();
    EXPECT_TRUE(mWidget->m_buttonBox->isHidden());
    EXPECT_TRUE(mWidget->m_searchPush->isHidden());
}

// normalStateUpdateSearchEditWidth
TEST_F(test_ctitlewidget, normalStateUpdateSearchEditWidth)
{
    mWidget->normalStateUpdateSearchEditWidth();
    EXPECT_LE(mWidget->m_searchEdit->maximumWidth(), 354);
    EXPECT_GE(mWidget->m_searchEdit->maximumWidth(), 240);
}

// updateSidebarIconStatus
TEST_F(test_ctitlewidget, updateSidebarIconStatus)
{
    // Icon is loaded from system theme; not available in offscreen test context.
    // Verify method executes without crash and button remains valid.
    mWidget->updateSidebarIconStatus();
    EXPECT_NE(mWidget->m_sidebarIcon, nullptr);
}

// updateSearchIcon
TEST_F(test_ctitlewidget, updateSearchIcon)
{
    // Icon is loaded from SVG resources; not available in offscreen test context.
    // Verify method executes without crash and button remains valid.
    mWidget->updateSearchIcon();
    EXPECT_NE(mWidget->m_searchPush, nullptr);
}

// updateSearchEditPlaceHolder
TEST_F(test_ctitlewidget, updateSearchEditPlaceHolder)
{
    mWidget->updateSearchEditPlaceHolder();
    EXPECT_FALSE(mWidget->m_searchEdit->placeholderText().isEmpty());
}

// resizeEvent: Normal state
TEST_F(test_ctitlewidget, resizeEvent_normal)
{
    mWidget->setShowState(CTitleWidget::Title_State_Normal);
    QResizeEvent event(QSize(600, 40), QSize(800, 40));
    mWidget->resizeEvent(&event);
}

// resizeEvent: Mini state
TEST_F(test_ctitlewidget, resizeEvent_mini)
{
    mWidget->setShowState(CTitleWidget::Title_State_Mini);
    QResizeEvent event(QSize(300, 40), QSize(800, 40));
    mWidget->resizeEvent(&event);
}

// changeEvent: FontChange
TEST_F(test_ctitlewidget, changeEvent_fontChange)
{
    QEvent event(QEvent::FontChange);
    mWidget->changeEvent(&event);
}

// changeEvent: other type
TEST_F(test_ctitlewidget, changeEvent_other)
{
    QEvent event(QEvent::PaletteChange);
    mWidget->changeEvent(&event);
}

// eventFilter: key press Return on DButtonBoxButton
TEST_F(test_ctitlewidget, eventFilter_keyReturn)
{
    QSignalSpy spy(mWidget, &CTitleWidget::signalSetButtonFocus);
    DButtonBoxButton *btn = mWidget->findChild<DButtonBoxButton *>();
    ASSERT_NE(btn, nullptr);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    mWidget->eventFilter(btn, &event);
    EXPECT_EQ(spy.count(), 1);
}

// eventFilter: key press Space on DButtonBoxButton
TEST_F(test_ctitlewidget, eventFilter_keySpace)
{
    QSignalSpy spy(mWidget, &CTitleWidget::signalSetButtonFocus);
    DButtonBoxButton *btn = mWidget->findChild<DButtonBoxButton *>();
    ASSERT_NE(btn, nullptr);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    mWidget->eventFilter(btn, &event);
    EXPECT_EQ(spy.count(), 1);
}

// eventFilter: other key on DButtonBoxButton (no signal)
TEST_F(test_ctitlewidget, eventFilter_keyOther)
{
    QSignalSpy spy(mWidget, &CTitleWidget::signalSetButtonFocus);
    DButtonBoxButton *btn = mWidget->findChild<DButtonBoxButton *>();
    ASSERT_NE(btn, nullptr);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    mWidget->eventFilter(btn, &event);
    EXPECT_EQ(spy.count(), 0);
}

// eventFilter: focus out TabFocusReason on search edit lineEdit
TEST_F(test_ctitlewidget, eventFilter_focusOutTab)
{
    QSignalSpy spy(mWidget, &CTitleWidget::signalSearchFocusSwitch);
    QFocusEvent event(QEvent::FocusOut, Qt::TabFocusReason);
    mWidget->eventFilter(mWidget->m_searchEdit->lineEdit(), &event);
    EXPECT_EQ(spy.count(), 1);
}

// eventFilter: focus out MouseFocusReason on search edit lineEdit
TEST_F(test_ctitlewidget, eventFilter_focusOutMouse)
{
    QSignalSpy spy(mWidget, &CTitleWidget::signalSearchFocusSwitch);
    QFocusEvent event(QEvent::FocusOut, Qt::MouseFocusReason);
    mWidget->eventFilter(mWidget->m_searchEdit->lineEdit(), &event);
    EXPECT_EQ(spy.count(), 0);
}

// slotShowSearchEdit
TEST_F(test_ctitlewidget, slotShowSearchEdit)
{
    // slotShowSearchEdit calls miniStateShowSearchEdit which hides buttonBox/searchPush
    // and shows searchEdit, then calls setFocus. In offscreen test context,
    // focus/visibility depend on windowing system, so check isHidden state instead.
    mWidget->slotShowSearchEdit();
    EXPECT_TRUE(mWidget->m_buttonBox->isHidden());            // buttonBox should be hidden
    EXPECT_TRUE(mWidget->m_searchPush->isHidden());           // searchPush should be hidden
    EXPECT_FALSE(mWidget->m_searchEdit->isHidden());          // searchEdit should be shown
}

// slotSearchEditFocusChanged: onFocus=true (no-op)
TEST_F(test_ctitlewidget, slotSearchEditFocusChanged_true)
{
    mWidget->setShowState(CTitleWidget::Title_State_Mini);
    mWidget->slotSearchEditFocusChanged(true);
    // no crash, controls remain
}

// slotSearchEditFocusChanged: onFocus=false, Mini state, empty text
TEST_F(test_ctitlewidget, slotSearchEditFocusChanged_false_mini)
{
    mWidget->setShowState(CTitleWidget::Title_State_Mini);
    mWidget->m_searchEdit->lineEdit()->clear();
    mWidget->slotSearchEditFocusChanged(false);
    EXPECT_TRUE(mWidget->m_searchEdit->isHidden());
}

// slotSearchEditFocusChanged: onFocus=false, Normal state
TEST_F(test_ctitlewidget, slotSearchEditFocusChanged_false_normal)
{
    mWidget->setShowState(CTitleWidget::Title_State_Normal);
    mWidget->m_searchEdit->lineEdit()->clear();
    mWidget->slotSearchEditFocusChanged(false);
    // Normal state: no hiding
}

// searchAborted lambda: triggers slotSearchEditFocusChanged(false) via signal
TEST_F(test_ctitlewidget, searchAborted_lambda)
{
    // The constructor connects DSearchEdit::searchAborted to a lambda that
    // calls slotSearchEditFocusChanged(false). Emit the signal to exercise the lambda.
    mWidget->setShowState(CTitleWidget::Title_State_Mini);
    mWidget->m_searchEdit->lineEdit()->clear();
    QMetaObject::invokeMethod(mWidget->m_searchEdit, "searchAborted");
    EXPECT_TRUE(mWidget->m_searchEdit->isHidden());
}

// slotSidebarIconClicked: toggles sidebar status and emits signal
TEST_F(test_ctitlewidget, slotSidebarIconClicked)
{
    QSignalSpy spy(mWidget, &CTitleWidget::signalSidebarStatusChange);
    bool before = mWidget->m_sidebarstatus;
    mWidget->slotSidebarIconClicked();
    EXPECT_GE(spy.count(), 1);
    // status should toggle
    EXPECT_NE(mWidget->m_sidebarstatus, before);
}
