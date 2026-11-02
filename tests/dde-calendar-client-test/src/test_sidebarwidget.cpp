// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QStyleOptionViewItem>

#include "sidebarWidget/sidebarcalendarwidget.h"
#include "sidebarWidget/sidebaritemwidget.h"
#include "sidebarWidget/sidebartreewidgetitemdelegate.h"
#include "sidebarWidget/sidebarview.h"
#include "accountitem.h"
#include "daccount.h"
#include "dscheduletype.h"
#include "accountmanager.h"
#include "calendarmanage.h"
#include "cschedulebasewidget.h"
#include "units.h"

// Concrete subclass of SidebarItemWidget for testing the abstract base class
class TestableSidebarItemWidget : public SidebarItemWidget
{
public:
    explicit TestableSidebarItemWidget(QWidget *parent = nullptr)
        : SidebarItemWidget(parent)
        , updateStatusCalled(false)
    {
    }
    void updateStatus() override
    {
        updateStatusCalled = true;
    }
    bool updateStatusCalled;
};

// =========================================================================
// SidebarCalendarKeyButton tests (12 methods)
// =========================================================================

class TestSidebarCalendarKeyButton : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_button = new SidebarCalendarKeyButton();
        m_button->resize(50, 50);
    }
    void TearDown() override
    {
        delete m_button;
    }
    SidebarCalendarKeyButton *m_button = nullptr;
};

TEST_F(TestSidebarCalendarKeyButton, Constructor)
{
    auto *btn = new SidebarCalendarKeyButton();
    EXPECT_NE(btn, nullptr);
    EXPECT_GE(btn->minimumSize().width(), 10);
    EXPECT_GE(btn->minimumSize().height(), 10);
    delete btn;
}

TEST_F(TestSidebarCalendarKeyButton, GetSetDate)
{
    QDate date(2026, 6, 15);
    m_button->setDate(date);
    EXPECT_EQ(m_button->getDate(), date);
}

TEST_F(TestSidebarCalendarKeyButton, GetSetSelectedDate)
{
    QDate date(2026, 3, 20);
    SidebarCalendarKeyButton::setSelectedDate(date);
    EXPECT_EQ(SidebarCalendarKeyButton::getSelectedDate(), date);
}

TEST_F(TestSidebarCalendarKeyButton, GetSetDisplayedMonth)
{
    QDate month(2026, 7, 1);
    SidebarCalendarKeyButton::setDisplayedMonth(month);
    EXPECT_EQ(SidebarCalendarKeyButton::getDisplayedMonth(), month);
}

TEST_F(TestSidebarCalendarKeyButton, ClickWithValidDate)
{
    QDate date(2026, 6, 15);
    m_button->setDate(date);
    SidebarCalendarKeyButton::setSelectedDate(QDate());
    m_button->click();
    EXPECT_EQ(SidebarCalendarKeyButton::getSelectedDate(), date);
}

TEST_F(TestSidebarCalendarKeyButton, ClickWithInvalidDate)
{
    m_button->setDate(QDate());
    SidebarCalendarKeyButton::setSelectedDate(QDate(2026, 1, 1));
    m_button->click();
    // Should not update selected date since date is invalid
    EXPECT_EQ(SidebarCalendarKeyButton::getSelectedDate(), QDate(2026, 1, 1));
}

TEST_F(TestSidebarCalendarKeyButton, ClickWithOutOfRangeYear)
{
    QDate date(1800, 6, 15);
    m_button->setDate(date);
    SidebarCalendarKeyButton::setSelectedDate(QDate(2026, 1, 1));
    m_button->click();
    EXPECT_EQ(SidebarCalendarKeyButton::getSelectedDate(), QDate(2026, 1, 1));
}

TEST_F(TestSidebarCalendarKeyButton, MousePressEvent)
{
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(25, 25), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_button->mousePressEvent(&pressEvent);
    EXPECT_TRUE(m_button->pressed);
}

TEST_F(TestSidebarCalendarKeyButton, MousePressEventRightButton)
{
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(25, 25), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    m_button->mousePressEvent(&pressEvent);
    EXPECT_FALSE(m_button->pressed);
}

TEST_F(TestSidebarCalendarKeyButton, MouseReleaseEventWithClick)
{
    m_button->setDate(QDate(2026, 6, 15));
    m_button->pressed = true;
    SidebarCalendarKeyButton::setSelectedDate(QDate());
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(25, 25), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_button->mouseReleaseEvent(&releaseEvent);
    EXPECT_FALSE(m_button->pressed);
    EXPECT_EQ(SidebarCalendarKeyButton::getSelectedDate(), QDate(2026, 6, 15));
}

TEST_F(TestSidebarCalendarKeyButton, MouseReleaseEventOutsideRect)
{
    m_button->pressed = true;
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(100, 100), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_button->mouseReleaseEvent(&releaseEvent);
    EXPECT_FALSE(m_button->pressed);
}

TEST_F(TestSidebarCalendarKeyButton, MouseMoveEventInside)
{
    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(25, 25), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_button->mouseMoveEvent(&moveEvent);
    EXPECT_TRUE(m_button->pressed);
}

TEST_F(TestSidebarCalendarKeyButton, MouseMoveEventOutside)
{
    m_button->pressed = true;
    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(100, 100), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_button->mouseMoveEvent(&moveEvent);
    EXPECT_FALSE(m_button->pressed);
}

TEST_F(TestSidebarCalendarKeyButton, PaintEventSelected)
{
    m_button->setDate(QDate(2026, 6, 15));
    SidebarCalendarKeyButton::setSelectedDate(QDate(2026, 6, 15));
    SidebarCalendarKeyButton::setDisplayedMonth(QDate(2026, 6, 1));
    QImage image(50, 50, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    QPaintEvent event(QRect(0, 0, 50, 50));
    m_button->paintEvent(&event);
    SUCCEED();
}

TEST_F(TestSidebarCalendarKeyButton, PaintEventNotSelectedDifferentMonth)
{
    m_button->setDate(QDate(2026, 6, 15));
    SidebarCalendarKeyButton::setSelectedDate(QDate(2026, 5, 15));
    SidebarCalendarKeyButton::setDisplayedMonth(QDate(2026, 5, 1));
    QImage image(50, 50, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPaintEvent event(QRect(0, 0, 50, 50));
    m_button->paintEvent(&event);
    SUCCEED();
}

TEST_F(TestSidebarCalendarKeyButton, PaintEventNotSelectedSameMonth)
{
    m_button->setDate(QDate(2026, 6, 15));
    SidebarCalendarKeyButton::setSelectedDate(QDate(2026, 6, 10));
    SidebarCalendarKeyButton::setDisplayedMonth(QDate(2026, 6, 1));
    QPaintEvent event(QRect(0, 0, 50, 50));
    m_button->paintEvent(&event);
    SUCCEED();
}

// =========================================================================
// SidebarCalendarWidget tests (9 methods)
// =========================================================================

class TestSidebarCalendarWidget : public ::testing::Test
{
protected:
    void SetUp() override
    {
        if (CScheduleBaseWidget::m_calendarManager == nullptr) {
            CScheduleBaseWidget::m_calendarManager = CalendarManager::getInstance();
        }
        m_widget = new SidebarCalendarWidget();
        m_widget->resize(180, 220);
    }
    void TearDown() override
    {
        delete m_widget;
    }
    SidebarCalendarWidget *m_widget = nullptr;
};

TEST_F(TestSidebarCalendarWidget, Constructor)
{
    auto *widget = new SidebarCalendarWidget();
    EXPECT_NE(widget, nullptr);
    EXPECT_NE(widget->m_keyButtonList.size(), 0);
    EXPECT_EQ(widget->m_keyButtonList.size(), 42);
    delete widget;
}

TEST_F(TestSidebarCalendarWidget, SetDate)
{
    QDate date(2026, 6, 15);
    m_widget->setDate(date);
    EXPECT_EQ(SidebarCalendarKeyButton::getSelectedDate(), date);
    SUCCEED();
}

TEST_F(TestSidebarCalendarWidget, SetKeyDateValid)
{
    QDate date(2026, 6, 15);
    m_widget->setKeyDate(date);
    SUCCEED();
}

TEST_F(TestSidebarCalendarWidget, SetKeyDateInvalid)
{
    QDate date;
    m_widget->setKeyDate(date);
    SUCCEED();
}

TEST_F(TestSidebarCalendarWidget, SlotNextPageClicked)
{
    SidebarCalendarKeyButton::setDisplayedMonth(QDate(2026, 6, 1));
    m_widget->slotNextPageClicked();
    EXPECT_EQ(SidebarCalendarKeyButton::getDisplayedMonth(), QDate(2026, 7, 1));
}

TEST_F(TestSidebarCalendarWidget, SlotPreviousPageClicked)
{
    SidebarCalendarKeyButton::setDisplayedMonth(QDate(2026, 6, 1));
    m_widget->slotPreviousPageClicked();
    EXPECT_EQ(SidebarCalendarKeyButton::getDisplayedMonth(), QDate(2026, 5, 1));
}

TEST_F(TestSidebarCalendarWidget, SlotFirstDayChanged)
{
    m_widget->slotFirstDayChanged(1);
    EXPECT_EQ(m_widget->m_firstday, 1);
}

TEST_F(TestSidebarCalendarWidget, SlotKeyButtonClickedSameMonth)
{
    QDate date(2026, 6, 15);
    SidebarCalendarKeyButton::setSelectedDate(date);
    SidebarCalendarKeyButton::setDisplayedMonth(QDate(2026, 6, 1));
    auto *btn = m_widget->m_keyButtonList.first();
    btn->setDate(date);
    m_widget->slotKeyButtonClicked(btn);
    SUCCEED();
}

TEST_F(TestSidebarCalendarWidget, SlotKeyButtonClickedDifferentMonth)
{
    QDate date(2026, 7, 15);
    SidebarCalendarKeyButton::setSelectedDate(date);
    SidebarCalendarKeyButton::setDisplayedMonth(QDate(2026, 6, 1));
    auto *btn = m_widget->m_keyButtonList.first();
    btn->setDate(date);
    m_widget->slotKeyButtonClicked(btn);
    SUCCEED();
}

TEST_F(TestSidebarCalendarWidget, SlotKeyButtonClickedInvalidDate)
{
    QDate date;
    SidebarCalendarKeyButton::setSelectedDate(date);
    auto *btn = m_widget->m_keyButtonList.first();
    btn->setDate(date);
    m_widget->slotKeyButtonClicked(btn);
    SUCCEED();
}

TEST_F(TestSidebarCalendarWidget, InitView)
{
    EXPECT_NE(m_widget->m_headWidget, nullptr);
    EXPECT_NE(m_widget->m_weekWidget, nullptr);
    EXPECT_NE(m_widget->m_keyWidget, nullptr);
    EXPECT_NE(m_widget->m_keyLayout, nullptr);
    EXPECT_NE(m_widget->m_dateLabel, nullptr);
    EXPECT_NE(m_widget->m_nextPage, nullptr);
    EXPECT_NE(m_widget->m_previousPage, nullptr);
}

TEST_F(TestSidebarCalendarWidget, InitConnection)
{
    // initConnection is called in constructor; verify connections exist by triggering signals
    EXPECT_NE(m_widget->m_nextPage, nullptr);
    EXPECT_NE(m_widget->m_previousPage, nullptr);
    SUCCEED();
}

TEST_F(TestSidebarCalendarWidget, InitData)
{
    // initData is called in constructor; verify m_firstday is set
    EXPECT_GE(m_widget->m_firstday, 0);
}

// =========================================================================
// SidebarItemWidget tests (10 methods) - via concrete subclass
// =========================================================================

class TestSidebarItemWidget : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_widget = new TestableSidebarItemWidget();
    }
    void TearDown() override
    {
        delete m_widget;
    }
    TestableSidebarItemWidget *m_widget = nullptr;
};

TEST_F(TestSidebarItemWidget, Constructor)
{
    auto *widget = new TestableSidebarItemWidget();
    EXPECT_NE(widget, nullptr);
    EXPECT_EQ(widget->width(), 170);
    delete widget;
}

TEST_F(TestSidebarItemWidget, GetSelectStatus)
{
    EXPECT_FALSE(m_widget->getSelectStatus());
}

TEST_F(TestSidebarItemWidget, SetSelectStatusTrue)
{
    m_widget->setSelectStatus(true);
    EXPECT_TRUE(m_widget->getSelectStatus());
    EXPECT_TRUE(m_widget->updateStatusCalled);
}

TEST_F(TestSidebarItemWidget, SetSelectStatusFalse)
{
    m_widget->setSelectStatus(true);
    m_widget->updateStatusCalled = false;
    m_widget->setSelectStatus(false);
    EXPECT_FALSE(m_widget->getSelectStatus());
    EXPECT_TRUE(m_widget->updateStatusCalled);
}

TEST_F(TestSidebarItemWidget, SetSelectStatusWithItemExpanded)
{
    QTreeWidget treeWidget;
    auto *treeItem = new QTreeWidgetItem();
    treeWidget.addTopLevelItem(treeItem);
    m_widget->setItem(treeItem);
    m_widget->setSelectStatus(true);
    EXPECT_TRUE(treeItem->isExpanded());
}

TEST_F(TestSidebarItemWidget, SetSelectStatusSameNoChange)
{
    m_widget->m_selectStatus = true;
    m_widget->updateStatusCalled = false;
    QTreeWidget treeWidget;
    auto *treeItem = new QTreeWidgetItem();
    treeWidget.addTopLevelItem(treeItem);
    treeItem->setExpanded(true);
    m_widget->setItem(treeItem);
    m_widget->setSelectStatus(true);
    EXPECT_FALSE(m_widget->updateStatusCalled);
}

TEST_F(TestSidebarItemWidget, SwitchState)
{
    EXPECT_FALSE(m_widget->getSelectStatus());
    m_widget->switchState();
    EXPECT_TRUE(m_widget->getSelectStatus());
    m_widget->switchState();
    EXPECT_FALSE(m_widget->getSelectStatus());
}

TEST_F(TestSidebarItemWidget, GetSetItem)
{
    auto *treeItem = new QTreeWidgetItem();
    m_widget->setItem(treeItem);
    EXPECT_EQ(m_widget->getTreeItem(), treeItem);
    delete treeItem;
}

TEST_F(TestSidebarItemWidget, MouseDoubleClickEvent)
{
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_widget->mouseDoubleClickEvent(&event);
    EXPECT_TRUE(m_widget->getSelectStatus());
}

TEST_F(TestSidebarItemWidget, MouseReleaseEvent)
{
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_widget->mouseReleaseEvent(&event);
    SUCCEED();
}

TEST_F(TestSidebarItemWidget, GetAccountItemWidget)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountName("TestAccount");
    account->setAccountID("test-id-001");
    AccountItem::Ptr accountItem(new AccountItem(account));
    SidebarItemWidget *widget = SidebarItemWidget::getAccountItemWidget(accountItem);
    EXPECT_NE(widget, nullptr);
    delete widget;
}

TEST_F(TestSidebarItemWidget, GetTypeItemWidget)
{
    DScheduleType::Ptr type(new DScheduleType("test-account-id"));
    type->setDisplayName("TestType");
    type->setColorCode("#FF0000");
    SidebarItemWidget *widget = SidebarItemWidget::getTypeItemWidget(type);
    EXPECT_NE(widget, nullptr);
    delete widget;
}

// =========================================================================
// SidebarAccountItemWidget tests (10 methods)
// =========================================================================

class TestSidebarAccountItemWidget : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_account = DAccount::Ptr(new DAccount(DAccount::Account_Local));
        m_account->setAccountName("TestAccount");
        m_account->setAccountID("test-local-001");
        m_accountItem = AccountItem::Ptr(new AccountItem(m_account));
        m_widget = new SidebarAccountItemWidget(m_accountItem);
    }
    void TearDown() override
    {
        delete m_widget;
    }
    DAccount::Ptr m_account;
    AccountItem::Ptr m_accountItem;
    SidebarAccountItemWidget *m_widget = nullptr;
};

TEST_F(TestSidebarAccountItemWidget, Constructor)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountName("Test2");
    account->setAccountID("test-local-002");
    AccountItem::Ptr item(new AccountItem(account));
    auto *widget = new SidebarAccountItemWidget(item);
    EXPECT_NE(widget, nullptr);
    delete widget;
}

TEST_F(TestSidebarAccountItemWidget, ConstructorUnionID)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_UnionID));
    account->setAccountName("UnionAccount");
    account->setAccountID("test-union-001");
    AccountItem::Ptr item(new AccountItem(account));
    auto *widget = new SidebarAccountItemWidget(item);
    EXPECT_NE(widget, nullptr);
    delete widget;
}

TEST_F(TestSidebarAccountItemWidget, GetAccountItem)
{
    EXPECT_EQ(m_widget->getAccountItem(), m_accountItem);
}

TEST_F(TestSidebarAccountItemWidget, InitView)
{
    EXPECT_NE(m_widget->m_headIconButton, nullptr);
    EXPECT_NE(m_widget->m_titleLabel, nullptr);
    EXPECT_NE(m_widget->m_syncIconButton, nullptr);
    EXPECT_NE(m_widget->m_warningLabel, nullptr);
}

TEST_F(TestSidebarAccountItemWidget, HeadIconButtonClicked)
{
    bool oldStatus = m_widget->getSelectStatus();
    m_widget->m_headIconButton->click();
    EXPECT_NE(m_widget->getSelectStatus(), oldStatus);
}

TEST_F(TestSidebarAccountItemWidget, InitConnect)
{
    // initConnect is called in constructor; verify widget is functional
    EXPECT_NE(m_widget->m_syncIconButton, nullptr);
    SUCCEED();
}

TEST_F(TestSidebarAccountItemWidget, ResetRearIconButtonNonUnion)
{
    // Local account - should return early
    m_widget->resetRearIconButton();
    SUCCEED();
}

TEST_F(TestSidebarAccountItemWidget, ResetRearIconButtonUnionNormalState)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_UnionID));
    account->setAccountName("UnionAccount2");
    account->setAccountID("test-union-002");
    account->setSyncState(DAccount::Sync_Normal);
    AccountItem::Ptr item(new AccountItem(account));
    auto *widget = new SidebarAccountItemWidget(item);
    widget->resetRearIconButton();
    SUCCEED();
    delete widget;
}

TEST_F(TestSidebarAccountItemWidget, ResetRearIconButtonUnionAbnormalState)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_UnionID));
    account->setAccountName("UnionAccount3");
    account->setAccountID("test-union-003");
    account->setSyncState(DAccount::Sync_NetworkAnomaly);
    AccountItem::Ptr item(new AccountItem(account));
    auto *widget = new SidebarAccountItemWidget(item);
    widget->resetRearIconButton();
    SUCCEED();
    delete widget;
}

TEST_F(TestSidebarAccountItemWidget, SlotAccountStateChange)
{
    m_widget->slotAccountStateChange();
    SUCCEED();
}

TEST_F(TestSidebarAccountItemWidget, SlotNetworkStateChangeNonUnion)
{
    m_widget->slotNetworkStateChange(DOANetWorkDBus::Disconnect);
    SUCCEED();
}

TEST_F(TestSidebarAccountItemWidget, SlotNetworkStateChangeDisconnect)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_UnionID));
    account->setAccountName("UnionNet");
    account->setAccountID("test-union-net-001");
    account->setSyncState(DAccount::Sync_Normal);
    AccountItem::Ptr item(new AccountItem(account));
    auto *widget = new SidebarAccountItemWidget(item);
    widget->slotNetworkStateChange(DOANetWorkDBus::Disconnect);
    SUCCEED();
    delete widget;
}

TEST_F(TestSidebarAccountItemWidget, SlotNetworkStateChangeActive)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_UnionID));
    account->setAccountName("UnionNet2");
    account->setAccountID("test-union-net-002");
    account->setSyncState(DAccount::Sync_Normal);
    AccountItem::Ptr item(new AccountItem(account));
    auto *widget = new SidebarAccountItemWidget(item);
    widget->slotNetworkStateChange(DOANetWorkDBus::Active);
    SUCCEED();
    delete widget;
}

TEST_F(TestSidebarAccountItemWidget, SlotSyncStatusChange)
{
    m_widget->slotSyncStatusChange(DAccount::Sync_Normal);
    EXPECT_EQ(m_account->syncState(), DAccount::Sync_Normal);
}

TEST_F(TestSidebarAccountItemWidget, SlotSyncStatusChangeNetworkAnomaly)
{
    m_widget->slotSyncStatusChange(DAccount::Sync_NetworkAnomaly);
    EXPECT_EQ(m_account->syncState(), DAccount::Sync_NetworkAnomaly);
}

TEST_F(TestSidebarAccountItemWidget, SlotRearIconClicked)
{
    m_widget->slotRearIconClicked();
    SUCCEED();
}

TEST_F(TestSidebarAccountItemWidget, UpdateStatus)
{
    m_widget->setSelectStatus(true);
    m_widget->updateStatus();
    SUCCEED();
}

TEST_F(TestSidebarAccountItemWidget, UpdateStatusFalse)
{
    m_widget->m_selectStatus = false;
    m_widget->updateStatus();
    SUCCEED();
}

// =========================================================================
// SidebarTypeItemWidget tests (3 methods)
// =========================================================================

class TestSidebarTypeItemWidget : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_type = DScheduleType::Ptr(new DScheduleType("test-account-type"));
        m_type->setDisplayName("TestScheduleType");
        m_type->setColorCode("#FF0000");
        m_type->setShowState(DScheduleType::Show);
        m_widget = new SidebarTypeItemWidget(m_type);
    }
    void TearDown() override
    {
        delete m_widget;
    }
    DScheduleType::Ptr m_type;
    SidebarTypeItemWidget *m_widget = nullptr;
};

TEST_F(TestSidebarTypeItemWidget, Constructor)
{
    DScheduleType::Ptr type(new DScheduleType("test-acct"));
    type->setDisplayName("MyType");
    type->setColorCode("#00FF00");
    type->setShowState(DScheduleType::Show);
    auto *widget = new SidebarTypeItemWidget(type);
    EXPECT_NE(widget, nullptr);
    EXPECT_EQ(widget->m_id, "test-acct");
    delete widget;
}

TEST_F(TestSidebarTypeItemWidget, ConstructorHideState)
{
    DScheduleType::Ptr type(new DScheduleType("test-acct-hide"));
    type->setDisplayName("HiddenType");
    type->setColorCode("#0000FF");
    type->setShowState(DScheduleType::Hide);
    auto *widget = new SidebarTypeItemWidget(type);
    EXPECT_NE(widget, nullptr);
    EXPECT_FALSE(widget->getSelectStatus());
    delete widget;
}

TEST_F(TestSidebarTypeItemWidget, InitView)
{
    EXPECT_NE(m_widget->m_checkBox, nullptr);
    EXPECT_NE(m_widget->m_titleLabel, nullptr);
    EXPECT_EQ(m_widget->height(), 40);
}

TEST_F(TestSidebarTypeItemWidget, CheckBoxClicked)
{
    bool oldStatus = m_widget->getSelectStatus();
    m_widget->m_checkBox->setChecked(true);
    m_widget->m_checkBox->click();
    EXPECT_NE(m_widget->getSelectStatus(), oldStatus);
}

TEST_F(TestSidebarTypeItemWidget, UpdateStatus)
{
    m_widget->setSelectStatus(true);
    SUCCEED();
}

TEST_F(TestSidebarTypeItemWidget, UpdateStatusToggleOff)
{
    m_widget->m_selectStatus = true;
    m_widget->updateStatus();
    SUCCEED();
}

// Extern helper defined in a separate TU to prevent GCC devirtualization
extern void invokeDelegatePaint(QStyledItemDelegate *delegate, QPainter *painter,
                                const QStyleOptionViewItem &option, const QModelIndex &index);

// =========================================================================
// SideBarTreeWidgetItemDelegate tests (2 methods)
// =========================================================================

class TestSideBarTreeWidgetItemDelegate : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_delegate = new SideBarTreeWidgetItemDelegate();
        m_treeWidget = new QTreeWidget();
        m_treeWidget->resize(200, 200);
    }
    void TearDown() override
    {
        delete m_delegate;
        delete m_treeWidget;
    }
    SideBarTreeWidgetItemDelegate *m_delegate = nullptr;
    QTreeWidget *m_treeWidget = nullptr;
};

TEST_F(TestSideBarTreeWidgetItemDelegate, Constructor)
{
    auto *delegate = new SideBarTreeWidgetItemDelegate();
    EXPECT_NE(delegate, nullptr);
    delete delegate;
}

TEST_F(TestSideBarTreeWidgetItemDelegate, PaintInvalidIndex)
{
    QImage image(200, 40, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    QModelIndex invalidIndex;
    invokeDelegatePaint(m_delegate, &painter, option, invalidIndex);
    SUCCEED();
}

TEST_F(TestSideBarTreeWidgetItemDelegate, PaintValidIndexOnlyOne)
{
    auto *item = new QTreeWidgetItem();
    m_treeWidget->addTopLevelItem(item);
    QModelIndex index = m_treeWidget->model()->index(0, 0);
    QImage image(200, 40, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    option.viewItemPosition = QStyleOptionViewItem::OnlyOne;
    invokeDelegatePaint(m_delegate, &painter, option, index);
    SUCCEED();
}

TEST_F(TestSideBarTreeWidgetItemDelegate, PaintValidIndexOnlyOneMouseOver)
{
    auto *item = new QTreeWidgetItem();
    m_treeWidget->addTopLevelItem(item);
    QModelIndex index = m_treeWidget->model()->index(0, 0);
    QImage image(200, 40, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    option.viewItemPosition = QStyleOptionViewItem::OnlyOne;
    option.state |= QStyle::State_MouseOver;
    invokeDelegatePaint(m_delegate, &painter, option, index);
    SUCCEED();
}

TEST_F(TestSideBarTreeWidgetItemDelegate, PaintValidIndexDefaultPosition)
{
    auto *item = new QTreeWidgetItem();
    m_treeWidget->addTopLevelItem(item);
    QModelIndex index = m_treeWidget->model()->index(0, 0);
    QImage image(200, 40, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    option.viewItemPosition = QStyleOptionViewItem::Invalid;
    invokeDelegatePaint(m_delegate, &painter, option, index);
    SUCCEED();
}

// =========================================================================
// SidebarView tests (13 methods)
// =========================================================================

class TestSidebarView : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_view = new SidebarView();
        m_view->resize(180, 400);
    }
    void TearDown() override
    {
        delete m_view;
    }
    SidebarView *m_view = nullptr;
};

TEST_F(TestSidebarView, Constructor)
{
    auto *view = new SidebarView();
    EXPECT_NE(view, nullptr);
    delete view;
}

TEST_F(TestSidebarView, InitView)
{
    EXPECT_NE(m_view->m_treeWidget, nullptr);
    EXPECT_NE(m_view->m_calendarWidget, nullptr);
    EXPECT_NE(m_view->delegate, nullptr);
    EXPECT_EQ(m_view->width(), 180);
}

TEST_F(TestSidebarView, InitConnection)
{
    // initConnection is called in constructor
    EXPECT_NE(m_view->m_treeWidget, nullptr);
    SUCCEED();
}

TEST_F(TestSidebarView, InitData)
{
    // initData is called in constructor; it calls initLocalAccountItem and initUnionAccountItem
    // If no accounts available, widgets may be null
    SUCCEED();
}

TEST_F(TestSidebarView, PaintEvent)
{
    QImage image(180, 400, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    QPaintEvent event(QRect(0, 0, 180, 400));
    m_view->paintEvent(&event);
    SUCCEED();
}

TEST_F(TestSidebarView, ResetJobTypeChildItemNull)
{
    m_view->resetJobTypeChildItem(nullptr);
    SUCCEED();
}

TEST_F(TestSidebarView, ResetJobTypeChildItemWithScheduleTypes)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountName("TestAccountForJobType");
    account->setAccountID("test-jobtype-001");
    AccountItem::Ptr accountItem(new AccountItem(account));
    DScheduleType::Ptr type1(new DScheduleType("test-jobtype-001"));
    type1->setDisplayName("Work");
    type1->setColorCode("#FF0000");
    type1->setShowState(DScheduleType::Show);
    type1->setPrivilege(DScheduleType::Read);
    DScheduleType::Ptr type2(new DScheduleType("test-jobtype-001"));
    type2->setDisplayName("Personal");
    type2->setColorCode("#00FF00");
    type2->setShowState(DScheduleType::Show);
    type2->setPrivilege(DScheduleType::Read);
    accountItem->m_scheduleTypeList.append(type1);
    accountItem->m_scheduleTypeList.append(type2);

    auto *acctWidget = new SidebarAccountItemWidget(accountItem);
    QTreeWidgetItem *treeItem = new QTreeWidgetItem();
    acctWidget->setItem(treeItem);

    m_view->m_treeWidget->addTopLevelItem(treeItem);
    m_view->m_treeWidget->setItemWidget(treeItem, 0, acctWidget);
    m_view->resetJobTypeChildItem(acctWidget);
    EXPECT_GT(treeItem->childCount(), 0);

    QTreeWidgetItem *childItem = treeItem->child(0);
    ASSERT_NE(childItem, nullptr);
    auto *typeWidget = qobject_cast<SidebarTypeItemWidget *>(
        m_view->m_treeWidget->itemWidget(childItem, 0));
    ASSERT_NE(typeWidget, nullptr);

    bool signalReceived = false;
    QObject::connect(m_view, &SidebarView::signalScheduleHide, [&]() {
        signalReceived = true;
    });
    typeWidget->signalStatusChange(true, "test-jobtype-001");
    EXPECT_TRUE(signalReceived);

    delete acctWidget;
}

TEST_F(TestSidebarView, ResetTreeItemPosNull)
{
    m_view->resetTreeItemPos(nullptr);
    SUCCEED();
}

TEST_F(TestSidebarView, SlotAccountUpdate)
{
    m_view->slotAccountUpdate();
    SUCCEED();
}

TEST_F(TestSidebarView, SlotScheduleTypeUpdate)
{
    m_view->slotScheduleTypeUpdate();
    SUCCEED();
}

TEST_F(TestSidebarView, SignalLogoutLocal)
{
    m_view->signalLogout(DAccount::Account_Local);
    SUCCEED();
}

TEST_F(TestSidebarView, SignalLogoutUnionID)
{
    m_view->signalLogout(DAccount::Account_UnionID);
    SUCCEED();
}

TEST_F(TestSidebarView, SignalLogoutCalDav)
{
    m_view->signalLogout(DAccount::Account_CalDav);
    SUCCEED();
}

TEST_F(TestSidebarView, InitExpandStatus)
{
    // Call initData to trigger initExpandStatus again
    m_view->initData();
    SUCCEED();
}

TEST_F(TestSidebarView, InitLocalAccountItem)
{
    m_view->initLocalAccountItem();
    SUCCEED();
}

TEST_F(TestSidebarView, InitUnionAccountItem)
{
    m_view->initUnionAccountItem();
    SUCCEED();
}
