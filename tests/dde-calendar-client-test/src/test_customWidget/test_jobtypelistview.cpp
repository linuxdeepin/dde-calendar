// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_jobtypelistview.h"
#include <QHoverEvent>
#include <QTest>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionViewItem>
#include <QStandardItemModel>
#include <QToolTip>
#include <DStyledItemDelegate>
#include <DStyle>
#include "../dialog_stub.h"

test_jobtypelistview::test_jobtypelistview()
{
    mWidget = new JobTypeListView();
}

test_jobtypelistview::~test_jobtypelistview()
{
    delete mWidget;
    mWidget = nullptr;
}

// JobTypeListView(QWidget *parent) — constructor (implicit in SetUp)
TEST_F(test_jobtypelistview, constructor)
{
    EXPECT_NE(mWidget, nullptr);
}

// ~JobTypeListView — destructor (implicit in TearDown)

// void JobTypeListView::initUI()
TEST_F(test_jobtypelistview, initUI)
{
    mWidget->initUI();
}

// bool JobTypeListView::viewportEvent(QEvent *event) — HoverLeave no items
TEST_F(test_jobtypelistview, viewportEvent_HoverLeave_noHover)
{
    QHoverEvent e(QEvent::HoverLeave, QPointF(0, 0), QPointF(0, 0));
    mWidget->viewportEvent(&e);
}

// bool JobTypeListView::viewportEvent(QEvent *event) — HoverLeave with hover index
TEST_F(test_jobtypelistview, viewportEvent_HoverLeave_withIndex)
{
    DScheduleType info;
    mWidget->addJobTypeItem(info);
    mWidget->m_iIndexCurrentHover = 0;
    QHoverEvent e(QEvent::HoverLeave, QPointF(0, 0), QPointF(0, 0));
    mWidget->viewportEvent(&e);
    EXPECT_EQ(mWidget->m_iIndexCurrentHover, -1);
}

// bool JobTypeListView::viewportEvent(QEvent *event) — HoverMove
TEST_F(test_jobtypelistview, viewportEvent_HoverMove)
{
    QHoverEvent e(QEvent::HoverMove, QPointF(1, 1), QPointF(0, 0));
    mWidget->viewportEvent(&e);
}

// bool JobTypeListView::viewportEvent(QEvent *event) — HoverEnter
TEST_F(test_jobtypelistview, viewportEvent_HoverEnter)
{
    QHoverEvent e(QEvent::HoverEnter, QPointF(1, 1), QPointF(0, 0));
    mWidget->viewportEvent(&e);
}

// bool JobTypeListView::updateJobType() — no account returns false
TEST_F(test_jobtypelistview, updateJobType_noAccount)
{
    EXPECT_FALSE(mWidget->updateJobType());
}

// void JobTypeListView::updateCalendarAccount(QString account_id)
TEST_F(test_jobtypelistview, updateCalendarAccount)
{
    mWidget->updateCalendarAccount("nonexistent_account");
}

// bool JobTypeListView::canAdd() — no account returns false
TEST_F(test_jobtypelistview, canAdd_noAccount)
{
    EXPECT_FALSE(mWidget->canAdd());
}

// void JobTypeListView::setItemEnabled(bool b)
TEST_F(test_jobtypelistview, setItemEnabled)
{
    mWidget->setItemEnabled(true);
    mWidget->setItemEnabled(false);
}

// int JobTypeListView::addJobTypeItem(const DScheduleType &info)
TEST_F(test_jobtypelistview, addJobTypeItem)
{
    DScheduleType info;
    mWidget->addJobTypeItem(info);
}

// void JobTypeListView::slotAddScheduleType() — no account returns early
TEST_F(test_jobtypelistview, slotAddScheduleType_noAccount)
{
    mWidget->slotAddScheduleType();
}

// void JobTypeListView::slotImportScheduleType() — no account returns early
TEST_F(test_jobtypelistview, slotImportScheduleType_noAccount)
{
    mWidget->slotImportScheduleType();
}

// void JobTypeListView::slotExportScheduleType() — no hovered item returns early
TEST_F(test_jobtypelistview, slotExportScheduleType_noItem)
{
    mWidget->slotExportScheduleType();
}

// void JobTypeListView::slotUpdateJobType() — invalid index returns early
TEST_F(test_jobtypelistview, slotUpdateJobType_invalidIndex)
{
    mWidget->slotUpdateJobType();
}

// void JobTypeListView::slotDeleteJobType() — no hovered item returns early
TEST_F(test_jobtypelistview, slotDeleteJobType_noItem)
{
    mWidget->slotDeleteJobType();
}

// void JobTypeListView::slotDeleteJobType() — with hovered item but no account
TEST_F(test_jobtypelistview, slotDeleteJobType_withItemNoAccount)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    mWidget->m_iIndexCurrentHover = 0;
    DScheduleType info;
    mWidget->addJobTypeItem(info);
    mWidget->slotDeleteJobType();
}

// void JobTypeListView::slotUpdateJobType() — with valid index but no account
TEST_F(test_jobtypelistview, slotUpdateJobType_validIndexNoAccount)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    DScheduleType info;
    mWidget->addJobTypeItem(info);
    mWidget->slotUpdateJobType();
}

// JobTypeListViewStyle::paint() — draw line path
TEST_F(test_jobtypelistview, style_paint_drawLine)
{
    JobTypeListViewStyle style(mWidget);
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem;
    item->setData(true, Qt::UserRole + 4); // RoleJobTypeLine
    model.appendRow(item);
    QModelIndex idx = model.index(0, 0);

    QPixmap pix(200, 50);
    pix.fill(Qt::white);
    QPainter painter(&pix);
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 200, 50);
    style.paint(&painter, opt, idx);
    painter.end();
}

// JobTypeListViewStyle::paint() — normal item path with DScheduleType data
TEST_F(test_jobtypelistview, style_paint_normalItem)
{
    JobTypeListViewStyle style(mWidget);
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem;
    DScheduleType info;
    info.setDisplayName("TestType");
    info.setColorCode("#FF0000");
    item->setData(QVariant::fromValue(info), Qt::UserRole + 2); // RoleJobTypeInfo
    item->setData(false, Qt::UserRole + 4); // RoleJobTypeLine
    model.appendRow(item);
    QModelIndex idx = model.index(0, 0);

    QPixmap pix(200, 50);
    pix.fill(Qt::white);
    QPainter painter(&pix);
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 200, 50);
    opt.state = QStyle::State_Enabled;
    style.paint(&painter, opt, idx);
    painter.end();
}

// initUI entered lambda — invalid index
TEST_F(test_jobtypelistview, enteredSignal_invalidIndex)
{
    emit mWidget->entered(QModelIndex());
}

// initUI entered lambda — valid index with DScheduleType data
TEST_F(test_jobtypelistview, enteredSignal_validIndex)
{
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem;
    DScheduleType info;
    info.setDisplayName("TestType");
    item->setData(QVariant::fromValue(info), Qt::UserRole + 2); // RoleJobTypeInfo
    model.appendRow(item);
    QModelIndex idx = model.index(0, 0);
    emit mWidget->entered(idx);
}

// initUI entered lambda — valid index with long display name (tooltip path)
TEST_F(test_jobtypelistview, enteredSignal_longName)
{
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem;
    DScheduleType info;
    info.setDisplayName("ThisIsAVeryLongScheduleTypeNameThatExceedsWidth");
    item->setData(QVariant::fromValue(info), Qt::UserRole + 2); // RoleJobTypeInfo
    model.appendRow(item);
    QModelIndex idx = model.index(0, 0);
    mWidget->resize(50, 50);
    emit mWidget->entered(idx);
}

// viewportEvent HoverMove with item — triggers createActionButton lambda
TEST_F(test_jobtypelistview, viewportEvent_HoverMove_withItem)
{
    mWidget->setStyle(new DStyle);
    mWidget->resize(400, 200);
    mWidget->show();
    DScheduleType info;
    mWidget->addJobTypeItem(info);
    QTest::qWait(50);
    QHoverEvent e(QEvent::HoverMove, QPointF(10, 23), QPointF(-1, -1));
    mWidget->viewportEvent(&e);
    mWidget->hide();
}

// Free functions covered indirectly:
// icsDialogDirectory() — called by slotImportScheduleType/slotExportScheduleType
// isCalDavRemoteScheduleType() — called by addJobTypeItem
// updateLastIcsDirectory() — called by slotImportScheduleType/slotExportScheduleType
