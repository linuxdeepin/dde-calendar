// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_caldavaccountlistwidget.h"

#include <QApplication>
#include <QEvent>
#include <QVBoxLayout>
#include "accountmanager.h"
#include <DToolButton>
#include <DBackgroundGroup>

test_caldavaccountlistwidget::test_caldavaccountlistwidget()
{
    mWidget = new CalDavAccountListWidget();
}

test_caldavaccountlistwidget::~test_caldavaccountlistwidget()
{
    delete mWidget;
    mWidget = nullptr;
}

// CalDavAccountListWidget::CalDavAccountListWidget(QWidget *parent)
TEST_F(test_caldavaccountlistwidget, Constructor)
{
    ASSERT_NE(mWidget, nullptr);
    ASSERT_NE(mWidget->m_cardsLayout, nullptr);
    EXPECT_GE(mWidget->m_cardsLayout->count(), 1);
}

// bool CalDavAccountListWidget::eventFilter(QObject *watched, QEvent *event)
TEST_F(test_caldavaccountlistwidget, eventFilter_NullWatched)
{
    QEvent event(QEvent::None);
    bool result = mWidget->eventFilter(nullptr, &event);
    EXPECT_FALSE(result);
}

TEST_F(test_caldavaccountlistwidget, eventFilter_NoDeleteButton)
{
    QWidget dummy;
    QEvent enterEvent(QEvent::Enter);
    bool result = mWidget->eventFilter(&dummy, &enterEvent);
    EXPECT_FALSE(result);
}

TEST_F(test_caldavaccountlistwidget, eventFilter_EnterShowsButton)
{
    QWidget dummy;
    Dtk::Widget::DToolButton *btn = new Dtk::Widget::DToolButton(&dummy);
    btn->hide();
    EXPECT_TRUE(btn->isHidden());

    QEvent enterEvent(QEvent::Enter);
    mWidget->eventFilter(&dummy, &enterEvent);
    EXPECT_FALSE(btn->isHidden());

    QEvent leaveEvent(QEvent::Leave);
    mWidget->eventFilter(&dummy, &leaveEvent);
    EXPECT_TRUE(btn->isHidden());
}

// void CalDavAccountListWidget::slotAccountUpdate()
TEST_F(test_caldavaccountlistwidget, slotAccountUpdate)
{
    QMetaObject::invokeMethod(gAccountManager, "signalAccountUpdate",
                              Qt::DirectConnection);
    qApp->processEvents();
    EXPECT_GE(mWidget->m_cardsLayout->count(), 1);
}

// void CalDavAccountListWidget::slotStatusChanged(const QString &accountID)
TEST_F(test_caldavaccountlistwidget, slotStatusChanged_EmptyAccountID)
{
    QMetaObject::invokeMethod(gAccountManager, "signalCalDavAccountStatusChanged",
                              Qt::DirectConnection,
                              Q_ARG(QString, QString()));
    qApp->processEvents();
    SUCCEED();
}

TEST_F(test_caldavaccountlistwidget, slotStatusChanged_NonExistentAccountID)
{
    QMetaObject::invokeMethod(gAccountManager, "signalCalDavAccountStatusChanged",
                              Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("non-existent-id")));
    qApp->processEvents();
    SUCCEED();
}

// void CalDavAccountListWidget::showConflictNotice(const QString &accountID)
TEST_F(test_caldavaccountlistwidget, showConflictNotice_EmptyAccountID)
{
    mWidget->showConflictNotice(QString());
    EXPECT_TRUE(mWidget->m_conflictNotices.isEmpty());
}

TEST_F(test_caldavaccountlistwidget, showConflictNotice_NonExistentAccountID)
{
    mWidget->showConflictNotice(QStringLiteral("non-existent-id"));
    EXPECT_TRUE(mWidget->m_conflictNotices.isEmpty());
}

// void CalDavAccountListWidget::rebuildCards()
TEST_F(test_caldavaccountlistwidget, rebuildCards)
{
    mWidget->rebuildCards();
    EXPECT_GE(mWidget->m_cardsLayout->count(), 1);
    QLayoutItem *item = mWidget->m_cardsLayout->itemAt(0);
    ASSERT_NE(item, nullptr);
    QWidget *w = item->widget();
    ASSERT_NE(w, nullptr);
    EXPECT_NE(qobject_cast<Dtk::Widget::DBackgroundGroup *>(w), nullptr);
}
