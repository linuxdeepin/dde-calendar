// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_scheduletypeeditdlg.h"

test_scheduletypeeditdlg::test_scheduletypeeditdlg()
{
    mDlgNew = new ScheduleTypeEditDlg(ScheduleTypeEditDlg::DialogNewType);
    mDlgImport = new ScheduleTypeEditDlg(ScheduleTypeEditDlg::DialogImportType);
}

test_scheduletypeeditdlg::~test_scheduletypeeditdlg()
{
    delete mDlgNew;
    mDlgNew = nullptr;
    delete mDlgImport;
    mDlgImport = nullptr;
}

// ScheduleTypeEditDlg(DialogType type, QWidget *parent = nullptr)
TEST_F(test_scheduletypeeditdlg, Constructor)
{
    ScheduleTypeEditDlg dlgNew(ScheduleTypeEditDlg::DialogNewType);
    ScheduleTypeEditDlg dlgImport(ScheduleTypeEditDlg::DialogImportType);
}

// ~ScheduleTypeEditDlg()
TEST_F(test_scheduletypeeditdlg, Destructor)
{
    ScheduleTypeEditDlg *dlg = new ScheduleTypeEditDlg(ScheduleTypeEditDlg::DialogNewType);
    delete dlg;
}

// void ScheduleTypeEditDlg::init()
TEST_F(test_scheduletypeeditdlg, init)
{
    mDlgNew->init();
    mDlgImport->init();
}

// void ScheduleTypeEditDlg::initView()
TEST_F(test_scheduletypeeditdlg, initView)
{
    mDlgNew->initView();
    mDlgImport->initView();
}

// void ScheduleTypeEditDlg::initData()
TEST_F(test_scheduletypeeditdlg, initData)
{
    mDlgNew->initData();
    mDlgImport->initData();
}

// DScheduleType ScheduleTypeEditDlg::newJsonType()
TEST_F(test_scheduletypeeditdlg, newJsonType)
{
    DScheduleType result = mDlgNew->newJsonType();
    SUCCEED();
}

// void ScheduleTypeEditDlg::setAccount(AccountItem::Ptr account)
TEST_F(test_scheduletypeeditdlg, setAccount)
{
    GTEST_SKIP() << "Requires AccountItem::Ptr and CConfigSettings::getInstance()";
}

// void ScheduleTypeEditDlg::slotEditTextChanged(const QString &strName)
TEST_F(test_scheduletypeeditdlg, slotEditTextChanged_Normal)
{
    mDlgNew->m_typeText = "";
    mDlgNew->m_lineEdit->setText("TestName");
    mDlgNew->slotEditTextChanged("TestName");
    EXPECT_EQ(mDlgNew->m_typeText, QString("TestName"));
    EXPECT_EQ(mDlgNew->m_jobTypeNew.displayName(), QString("TestName"));
}

TEST_F(test_scheduletypeeditdlg, slotEditTextChanged_Newline)
{
    mDlgNew->m_typeText = "abc";
    mDlgNew->slotEditTextChanged("ab\ncd");
    EXPECT_EQ(mDlgNew->m_lineEdit->text(), QString("abcd"));
}

TEST_F(test_scheduletypeeditdlg, slotEditTextChanged_ExceedsMaxLength)
{
    mDlgNew->m_typeText = "oldtext";
    QString longText = "123456789012345678901";  // 21 chars
    mDlgNew->slotEditTextChanged(longText);
    EXPECT_EQ(mDlgNew->m_lineEdit->text(), QString("oldtext"));
}

TEST_F(test_scheduletypeeditdlg, slotEditTextChanged_Empty)
{
    mDlgNew->m_typeText = "oldtext";
    mDlgNew->slotEditTextChanged("");
    EXPECT_FALSE(mDlgNew->m_lineEdit->isAlert());
}

TEST_F(test_scheduletypeeditdlg, slotEditTextChanged_WhitespaceOnly)
{
    mDlgNew->m_typeText = "   ";
    mDlgNew->m_lineEdit->setText("   ");
    mDlgNew->slotEditTextChanged("   ");
    EXPECT_TRUE(mDlgNew->m_lineEdit->isAlert());
}

// void ScheduleTypeEditDlg::slotFocusChanged(bool onFocus)
TEST_F(test_scheduletypeeditdlg, slotFocusChanged_LostFocusEmpty)
{
    mDlgNew->m_lineEdit->setText("");
    mDlgNew->slotFocusChanged(false);
}

TEST_F(test_scheduletypeeditdlg, slotFocusChanged_GotFocus)
{
    mDlgNew->slotFocusChanged(true);
}

// void ScheduleTypeEditDlg::slotBtnCancel()
TEST_F(test_scheduletypeeditdlg, slotBtnCancel)
{
    mDlgNew->slotBtnCancel();
}

// void ScheduleTypeEditDlg::slotBtnNext()
TEST_F(test_scheduletypeeditdlg, slotBtnNext)
{
    mDlgNew->slotBtnNext();
}

// void ScheduleTypeEditDlg::slotEditingFinished()
TEST_F(test_scheduletypeeditdlg, slotEditingFinished_Empty)
{
    mDlgNew->m_lineEdit->setText("");
    mDlgNew->slotEditingFinished();
    EXPECT_TRUE(mDlgNew->m_lineEdit->isAlert());
}

TEST_F(test_scheduletypeeditdlg, slotEditingFinished_NonEmpty)
{
    mDlgNew->m_lineEdit->setText("test");
    mDlgNew->slotEditingFinished();
    EXPECT_FALSE(mDlgNew->m_lineEdit->isAlert());
}

// QString ScheduleTypeEditDlg::getIcsFile()
TEST_F(test_scheduletypeeditdlg, getIcsFile)
{
    mDlgImport->m_fileEdit->setText("/tmp/test.ics");
    EXPECT_EQ(mDlgImport->getIcsFile(), QString("/tmp/test.ics"));
}

// void ScheduleTypeEditDlg::slotCheckConfirmBtn()
TEST_F(test_scheduletypeeditdlg, slotCheckConfirmBtn_EmptyName)
{
    mDlgNew->m_lineEdit->setText("");
    mDlgNew->slotCheckConfirmBtn();
    EXPECT_FALSE(mDlgNew->getButton(1)->isEnabled());
}

TEST_F(test_scheduletypeeditdlg, slotCheckConfirmBtn_ValidName)
{
    mDlgNew->m_lineEdit->setText("TestName");
    mDlgNew->m_lineEdit->setAlert(false);
    mDlgNew->slotCheckConfirmBtn();
    EXPECT_TRUE(mDlgNew->getButton(1)->isEnabled());
}

TEST_F(test_scheduletypeeditdlg, slotCheckConfirmBtn_ImportEmptyFile)
{
    mDlgImport->m_lineEdit->setText("TestName");
    mDlgImport->m_lineEdit->setAlert(false);
    mDlgImport->m_fileEdit->setText("");
    mDlgImport->slotCheckConfirmBtn();
    EXPECT_FALSE(mDlgImport->getButton(1)->isEnabled());
}

TEST_F(test_scheduletypeeditdlg, slotCheckConfirmBtn_ImportValidFile)
{
    mDlgImport->m_lineEdit->setText("TestName");
    mDlgImport->m_lineEdit->setAlert(false);
    mDlgImport->m_fileEdit->setText("/tmp/test.ics");
    mDlgImport->slotCheckConfirmBtn();
    EXPECT_TRUE(mDlgImport->getButton(1)->isEnabled());
}

// ScheduleTypeEditDlg(QWidget *parent = nullptr)
TEST_F(test_scheduletypeeditdlg, Constructor_QWidget)
{
    ScheduleTypeEditDlg dlg(nullptr);
    SUCCEED();
}

// ScheduleTypeEditDlg(const DScheduleType &jobTypeOld, QWidget *parent = nullptr)
TEST_F(test_scheduletypeeditdlg, Constructor_DScheduleType)
{
    DScheduleType jobTypeOld;
    jobTypeOld.setDisplayName("TestType");
    ScheduleTypeEditDlg dlg(jobTypeOld);
    SUCCEED();
}
