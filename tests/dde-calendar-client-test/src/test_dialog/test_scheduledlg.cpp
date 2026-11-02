// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_scheduledlg.h"

#include "../dialog_stub.h"

#include <QEvent>
#include <QApplication>
#include <QShowEvent>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QDateTime>
#include <QDate>

static void stub_resize(void *) {}

static void setResizeStub(Stub &stub)
{
    typedef void (*fptr_resize)(CScheduleDlg *);
    fptr_resize p_resize = (fptr_resize)(&CScheduleDlg::resize);
    stub.set(p_resize, stub_resize);
}

test_scheduledlg::test_scheduledlg()
{
    mScheduleDlg = new CScheduleDlg(1);
}

test_scheduledlg::~test_scheduledlg()
{
    delete mScheduleDlg;
    mScheduleDlg = nullptr;
}

// CScheduleDlg(int type)
TEST_F(test_scheduledlg, Constructor)
{
    CScheduleDlg dlg(1);
    CScheduleDlg dlg2(0);
}

// ~CScheduleDlg()
TEST_F(test_scheduledlg, Destructor)
{
    CScheduleDlg *dlg = new CScheduleDlg(1);
    delete dlg;
}

// void CScheduleDlg::initUI()
TEST_F(test_scheduledlg, initUI)
{
    mScheduleDlg->initUI();
}

// void CScheduleDlg::initConnection()
TEST_F(test_scheduledlg, initConnection)
{
    mScheduleDlg->initConnection();
}

// void CScheduleDlg::initDateEdit()
TEST_F(test_scheduledlg, initDateEdit)
{
    mScheduleDlg->initDateEdit();
}

// void CScheduleDlg::initJobTypeComboBox()
TEST_F(test_scheduledlg, initJobTypeComboBox)
{
    GTEST_SKIP() << "Requires gAccountManager->getAccountItemByAccountId()";
}

// void CScheduleDlg::initRmindRpeatUI()
TEST_F(test_scheduledlg, initRmindRpeatUI)
{
    GTEST_SKIP() << "Requires m_scheduleDataInfo set via setData()";
}

// void CScheduleDlg::setTheMe(const int type)
TEST_F(test_scheduledlg, setTheMe)
{
    mScheduleDlg->setTheMe(1);
    mScheduleDlg->setTheMe(2);
}

// void CScheduleDlg::setWidgetEnabled(bool enabled)
TEST_F(test_scheduledlg, setWidgetEnabled)
{
    mScheduleDlg->setWidgetEnabled(true);
    mScheduleDlg->setWidgetEnabled(false);
}

// void CScheduleDlg::setAllDay(bool flag)
TEST_F(test_scheduledlg, setAllDay)
{
    mScheduleDlg->setAllDay(true);
    mScheduleDlg->setAllDay(false);
}

// void CScheduleDlg::setOkBtnEnabled()
TEST_F(test_scheduledlg, setOkBtnEnabled)
{
    mScheduleDlg->setOkBtnEnabled();
}

// void CScheduleDlg::setShowState(bool isLunar)
TEST_F(test_scheduledlg, setShowState)
{
    mScheduleDlg->setShowState(true);
    mScheduleDlg->setShowState(false);
}

// void CScheduleDlg::setTabFouseOrder()
TEST_F(test_scheduledlg, setTabFouseOrder)
{
    mScheduleDlg->setTabFouseOrder();
}

// void CScheduleDlg::setData(const DSchedule::Ptr &info)
TEST_F(test_scheduledlg, setData)
{
    GTEST_SKIP() << "Requires gAccountManager->getLocalAccountItem()";
}

// void CScheduleDlg::setDate(const QDateTime &date)
TEST_F(test_scheduledlg, setDate)
{
    QDateTime dt = QDateTime::currentDateTime();
    mScheduleDlg->setDate(dt);
}

// bool CScheduleDlg::isShowLunar()
TEST_F(test_scheduledlg, isShowLunar)
{
    mScheduleDlg->isShowLunar();
}

// bool CScheduleDlg::canWriteCurrentCalDavCollection() const
TEST_F(test_scheduledlg, canWriteCurrentCalDavCollection)
{
    EXPECT_TRUE(mScheduleDlg->canWriteCurrentCalDavCollection());
}

// bool CScheduleDlg::clickOkBtn()
TEST_F(test_scheduledlg, clickOkBtn)
{
    EXPECT_FALSE(mScheduleDlg->clickOkBtn());
}

// bool CScheduleDlg::selectScheduleType()
TEST_F(test_scheduledlg, selectScheduleType)
{
    GTEST_SKIP() << "Requires m_accountItem and createSchedule()";
}

// void CScheduleDlg::createSchedule(const QString &id)
TEST_F(test_scheduledlg, createSchedule)
{
    GTEST_SKIP() << "Requires m_accountItem and CScheduleOperation";
}

// void CScheduleDlg::resetColor(AccountItem::Ptr accountItem)
TEST_F(test_scheduledlg, resetColor)
{
    GTEST_SKIP() << "Requires CConfigSettings::getInstance()";
}

// void CScheduleDlg::resize()
TEST_F(test_scheduledlg, resize)
{
    GTEST_SKIP() << "Requires gAccountManager->getIsSupportUid()";
}

// void CScheduleDlg::slotBtClick(int buttonIndex, const QString &buttonName)
TEST_F(test_scheduledlg, slotBtClick)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    mScheduleDlg->slotBtClick(0, "ok");
    mScheduleDlg->slotBtClick(1, "ok");
}

// void CScheduleDlg::slotTextChange()
TEST_F(test_scheduledlg, slotTextChange)
{
    mScheduleDlg->slotTextChange();
}

// void CScheduleDlg::slotendrepeatTextchange()
TEST_F(test_scheduledlg, slotendrepeatTextchange)
{
    mScheduleDlg->slotendrepeatTextchange();
}

// void CScheduleDlg::slotBDateEidtInfo(const QDate &date)
TEST_F(test_scheduledlg, slotBDateEidtInfo)
{
    mScheduleDlg->slotBDateEidtInfo(QDate::currentDate());
    mScheduleDlg->slotBDateEidtInfo(QDate::currentDate().addDays(1));
}

// void CScheduleDlg::slotBeginTimeChange()
TEST_F(test_scheduledlg, slotBeginTimeChange)
{
    mScheduleDlg->slotBeginTimeChange();
}

// void CScheduleDlg::slotEndDateChange(const QDate &date)
TEST_F(test_scheduledlg, slotEndDateChange)
{
    mScheduleDlg->slotEndDateChange(QDate::currentDate());
}

// void CScheduleDlg::slotEndTimeChange()
TEST_F(test_scheduledlg, slotEndTimeChange)
{
    mScheduleDlg->slotEndTimeChange();
}

// void CScheduleDlg::slotallDayStateChanged(int state)
TEST_F(test_scheduledlg, slotallDayStateChanged)
{
    mScheduleDlg->slotallDayStateChanged(0);
    mScheduleDlg->slotallDayStateChanged(1);
}

// void CScheduleDlg::slotbRpeatactivated(int index)
TEST_F(test_scheduledlg, slotbRpeatactivated)
{
    Stub stub;
    setResizeStub(stub);
    mScheduleDlg->slotbRpeatactivated(0);
    mScheduleDlg->slotbRpeatactivated(1);
}

// void CScheduleDlg::sloteRpeatactivated(int index)
TEST_F(test_scheduledlg, sloteRpeatactivated)
{
    mScheduleDlg->sloteRpeatactivated(0);
    mScheduleDlg->sloteRpeatactivated(1);
    mScheduleDlg->sloteRpeatactivated(2);
}

// void CScheduleDlg::slotTypeRpeatactivated(int index)
TEST_F(test_scheduledlg, slotTypeRpeatactivated)
{
    Stub stub;
    setResizeStub(stub);
    mScheduleDlg->slotTypeRpeatactivated(0);
    mScheduleDlg->slotTypeRpeatactivated(1);
}

// void CScheduleDlg::slotBtnAddItemClicked()
TEST_F(test_scheduledlg, slotBtnAddItemClicked)
{
    Stub stub;
    setResizeStub(stub);
    mScheduleDlg->slotBtnAddItemClicked();
}

// void CScheduleDlg::slotJobComboBoxEditingFinished()
TEST_F(test_scheduledlg, slotJobComboBoxEditingFinished)
{
    mScheduleDlg->m_typeComBox->setEditable(true);
    mScheduleDlg->slotJobComboBoxEditingFinished();
}

// void CScheduleDlg::slotTypeEditTextChanged(const QString &text)
TEST_F(test_scheduledlg, slotTypeEditTextChanged)
{
    mScheduleDlg->slotTypeEditTextChanged("test");
}

// void CScheduleDlg::slotAccountStateChange()
TEST_F(test_scheduledlg, slotAccountStateChange)
{
    mScheduleDlg->slotAccountStateChange();
}

// void CScheduleDlg::slotAccountUpdate()
TEST_F(test_scheduledlg, slotAccountUpdate)
{
    GTEST_SKIP() << "Requires gAccountManager->getAccountList()";
}

// void CScheduleDlg::slotAccoutBoxActivated(const QString &accountID)
TEST_F(test_scheduledlg, slotAccoutBoxActivated)
{
    GTEST_SKIP() << "Requires gAccountManager->getAccountItemByAccountId()";
}

// void CScheduleDlg::slotRadioBtnClicked(int btnId)
TEST_F(test_scheduledlg, slotRadioBtnClicked)
{
    Stub stub;
    setResizeStub(stub);
    mScheduleDlg->slotRadioBtnClicked(0);
    mScheduleDlg->slotRadioBtnClicked(1);
}

// void CScheduleDlg::updateDateTimeFormat()
TEST_F(test_scheduledlg, updateDateTimeFormat)
{
    mScheduleDlg->updateDateTimeFormat();
}

// void CScheduleDlg::updateIsOneMoreDay(const QDateTime &begin, const QDateTime &end)
TEST_F(test_scheduledlg, updateIsOneMoreDay)
{
    QDateTime begin = QDateTime::currentDateTime();
    mScheduleDlg->updateIsOneMoreDay(begin, begin.addDays(2));
    mScheduleDlg->updateIsOneMoreDay(begin, begin.addSecs(3600));
}

// void CScheduleDlg::updateEndTimeList(const QTime &begin, bool isShowTimeInterval)
TEST_F(test_scheduledlg, updateEndTimeList)
{
    mScheduleDlg->updateEndTimeList(QTime(10, 0), true);
    mScheduleDlg->updateEndTimeList(QTime(10, 0), false);
}

// void CScheduleDlg::updateEndTimeListAndTimeDiff(const QDateTime &begin, const QDateTime &end)
TEST_F(test_scheduledlg, updateEndTimeListAndTimeDiff)
{
    QDateTime begin = QDateTime::currentDateTime();
    mScheduleDlg->updateEndTimeListAndTimeDiff(begin, begin.addSecs(3600));
}

// void CScheduleDlg::updateRepeatCombox(bool isLunar)
TEST_F(test_scheduledlg, updateRepeatCombox)
{
    Stub stub;
    setResizeStub(stub);
    mScheduleDlg->updateRepeatCombox(true);
    mScheduleDlg->updateRepeatCombox(false);
}

// void CScheduleDlg::showEvent(QShowEvent *event)
TEST_F(test_scheduledlg, showEvent)
{
    Stub stub;
    setResizeStub(stub);
    QShowEvent event;
    QApplication::sendEvent(mScheduleDlg, &event);
}

// void CScheduleDlg::closeEvent(QCloseEvent *event)
TEST_F(test_scheduledlg, closeEvent)
{
    QCloseEvent event;
    QApplication::sendEvent(mScheduleDlg, &event);
}

// bool CScheduleDlg::eventFilter(QObject *watched, QEvent *event)
TEST_F(test_scheduledlg, eventFilter)
{
    QKeyEvent keyReturn(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    EXPECT_TRUE(mScheduleDlg->eventFilter(mScheduleDlg->m_textEdit, &keyReturn));

    QKeyEvent keyTab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    EXPECT_TRUE(mScheduleDlg->eventFilter(mScheduleDlg->m_textEdit, &keyTab));

    QKeyEvent keyA(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    mScheduleDlg->eventFilter(mScheduleDlg->m_textEdit, &keyA);
}

// void CScheduleDlg::changeEvent(QEvent *event)
TEST_F(test_scheduledlg, changeEvent)
{
    QEvent event(QEvent::FontChange);
    QApplication::sendEvent(mScheduleDlg, &event);
}

// void CScheduleDlg::signalLogout(DAccount::Type type)
TEST_F(test_scheduledlg, signalLogout)
{
    GTEST_SKIP() << "Requires gUosAccountItem (gAccountManager->getUnionAccountItem())";
}
