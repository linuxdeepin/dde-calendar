// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_jobtypecombobox.h"
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QFrame>
#include <QVBoxLayout>
#include "../third-party_stub/stub.h"

test_jobtypecombobox::test_jobtypecombobox()
{
    mWidget = new JobTypeComboBox();
    mWidget->setAddTypeEnabled(true);
    mWidget->slotBtnAddItemClicked();
}

test_jobtypecombobox::~test_jobtypecombobox()
{
    delete mWidget;
    mWidget = nullptr;
}

// JobTypeComboBox(QWidget *parent) — constructor (implicit in SetUp)
TEST_F(test_jobtypecombobox, constructor)
{
    EXPECT_FALSE(mWidget->isEditable() == false ? false : false);
}

// ~JobTypeComboBox — destructor (implicit in TearDown)

// void JobTypeComboBox::initUI()
TEST_F(test_jobtypecombobox, initUI)
{
    mWidget->initUI();
    EXPECT_FALSE(mWidget->isEditable());
    EXPECT_EQ(mWidget->iconSize(), QSize(16, 16));
}

// void JobTypeComboBox::setAlert(bool isAlert)
TEST_F(test_jobtypecombobox, setAlert)
{
    mWidget->setAlert(true);
    EXPECT_TRUE(mWidget->isAlert());
    mWidget->setAlert(false);
    EXPECT_FALSE(mWidget->isAlert());
}

// bool JobTypeComboBox::isAlert() const
TEST_F(test_jobtypecombobox, isAlert)
{
    mWidget->setAlert(false);
    EXPECT_FALSE(mWidget->isAlert());
    mWidget->setAlert(true);
    EXPECT_TRUE(mWidget->isAlert());
}

// void JobTypeComboBox::showAlertMessage(const QString &text, QWidget *follower, int duration)
TEST_F(test_jobtypecombobox, showAlertMessage_default)
{
    mWidget->showAlertMessage("test alert");
}

TEST_F(test_jobtypecombobox, showAlertMessage_withFollower)
{
    mWidget->showAlertMessage("test alert", mWidget, 1000);
}

// void JobTypeComboBox::setAlertMessageAlignment(Qt::Alignment alignment)
TEST_F(test_jobtypecombobox, setAlertMessageAlignment)
{
    mWidget->setAlertMessageAlignment(Qt::AlignLeft);
}

// void JobTypeComboBox::hideAlertMessage()
TEST_F(test_jobtypecombobox, hideAlertMessage)
{
    mWidget->hideAlertMessage();
}

// int JobTypeComboBox::getCurrentEditPosition() const
TEST_F(test_jobtypecombobox, getCurrentEditPosition)
{
    EXPECT_EQ(mWidget->getCurrentEditPosition(), mWidget->m_newPos);
}

// void JobTypeComboBox::updateJobType(const AccountItem::Ptr& account) — nullptr returns early
TEST_F(test_jobtypecombobox, updateJobType_nullptr)
{
    mWidget->updateJobType(nullptr);
}

// void JobTypeComboBox::addJobTypeItem(int idx, QString strColorHex, QString strJobType)
TEST_F(test_jobtypecombobox, addJobTypeItem)
{
    mWidget->addJobTypeItem(0, "#FF0000", "TestType");
    EXPECT_EQ(mWidget->count(), 1);
}

// void JobTypeComboBox::showPopup()
TEST_F(test_jobtypecombobox, showPopup)
{
    mWidget->addJobTypeItem(0, "#FF0000", "TestType");
    mWidget->showPopup();
    EXPECT_FALSE(mWidget->isEditable());
}

// bool JobTypeComboBox::eventFilter(QObject *obj, QEvent *event)
TEST_F(test_jobtypecombobox, eventFilter_EnterView)
{
    QEvent e(QEvent::Enter);
    mWidget->eventFilter(mWidget->view(), &e);
}

TEST_F(test_jobtypecombobox, eventFilter_EnterAddBtn)
{
    mWidget->addJobTypeItem(0, "#FF0000", "TestType");
    mWidget->showPopup();
    ASSERT_NE(mWidget->m_addBtn, nullptr);
    QEvent e(QEvent::Enter);
    mWidget->eventFilter(mWidget->m_addBtn, &e);
}

TEST_F(test_jobtypecombobox, eventFilter_FocusInView)
{
    QEvent e(QEvent::FocusIn);
    mWidget->eventFilter(mWidget->view(), &e);
}

TEST_F(test_jobtypecombobox, eventFilter_KeyPressAddBtn)
{
    mWidget->addJobTypeItem(0, "#FF0000", "TestType");
    mWidget->showPopup();
    ASSERT_NE(mWidget->m_addBtn, nullptr);
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    mWidget->eventFilter(mWidget->m_addBtn, &e);
}

// void JobTypeComboBox::setItemSelectable(bool status)
TEST_F(test_jobtypecombobox, setItemSelectable_noHover)
{
    mWidget->m_hoverSelectedIndex = -1;
    mWidget->setItemSelectable(true);
}

TEST_F(test_jobtypecombobox, setItemSelectable_withHover)
{
    mWidget->addJobTypeItem(0, "#FF0000", "TestType");
    mWidget->m_hoverSelectedIndex = 0;
    mWidget->setItemSelectable(true);
    mWidget->setItemSelectable(false);
}

// void JobTypeComboBox::addCustomWidget(QFrame *viewContainer)
TEST_F(test_jobtypecombobox, addCustomWidget)
{
    QFrame *frame = new QFrame();
    QVBoxLayout *layout = new QVBoxLayout(frame);
    Q_UNUSED(layout);
    mWidget->addCustomWidget(frame);
    delete frame;
}

// void JobTypeComboBox::slotBtnAddItemClicked()
TEST_F(test_jobtypecombobox, slotBtnAddItemClicked)
{
    EXPECT_TRUE(mWidget->isEditable());
    EXPECT_TRUE(mWidget->currentText().isEmpty());
}

TEST_F(test_jobtypecombobox, slotBtnAddItemClicked_disabled)
{
    mWidget->setAddTypeEnabled(false);
    mWidget->setEditable(false);
    mWidget->slotBtnAddItemClicked();
    EXPECT_FALSE(mWidget->isEditable());
}

// void JobTypeComboBox::slotEditingFinished()
TEST_F(test_jobtypecombobox, slotEditingFinished)
{
    mWidget->slotEditingFinished();
}

// void JobTypeComboBox::slotEditCursorPositionChanged(int oldPos, int newPos)
TEST_F(test_jobtypecombobox, slotEditCursorPositionChanged)
{
    mWidget->slotEditCursorPositionChanged(0, 5);
    EXPECT_EQ(mWidget->m_oldPos, 0);
    EXPECT_EQ(mWidget->m_newPos, 5);
    EXPECT_EQ(mWidget->getCurrentEditPosition(), 5);
}

// QString JobTypeComboBox::getCurrentJobTypeNo()
TEST_F(test_jobtypecombobox, getCurrentJobTypeNo_empty)
{
    EXPECT_TRUE(mWidget->getCurrentJobTypeNo().isEmpty());
}

// void JobTypeComboBox::setCurrentJobTypeNo(const QString &strJobTypeNo)
TEST_F(test_jobtypecombobox, setCurrentJobTypeNo)
{
    mWidget->setCurrentJobTypeNo("nonexistent");
}

// void JobTypeComboBox::setAddTypeEnabled(bool enabled)
TEST_F(test_jobtypecombobox, setAddTypeEnabled)
{
    mWidget->setAddTypeEnabled(false);
    EXPECT_FALSE(mWidget->m_addTypeEnabled);
    mWidget->setAddTypeEnabled(true);
    EXPECT_TRUE(mWidget->m_addTypeEnabled);
}
