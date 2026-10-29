// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cdialogiconbutton.h"
#include "../dialog_stub.h"
#include "widget/cschedulebasewidget.h"
#include "dialog/timejumpdialog.h"

test_cdialogiconbutton::test_cdialogiconbutton(QObject *parent) : QObject(parent)
{
}

// Stub for CScheduleBaseWidget::getSelectDate (static method)
QDate stub_getSelectDate()
{
    return QDate::currentDate();
}

// Stub for TimeJumpDialog::showPopup
void stub_showPopup(void *obj, const QDate &date, int x, int y)
{
    Q_UNUSED(obj)
    Q_UNUSED(date)
    Q_UNUSED(x)
    Q_UNUSED(y)
}

// CDialogIconButton::CDialogIconButton(QWidget *parent)
TEST_F(test_cdialogiconbutton, Constructor)
{
    EXPECT_NE(mWidget->m_dialog, nullptr);
    EXPECT_TRUE(mWidget->isFlat());
}

// void CDialogIconButton::initView()
TEST_F(test_cdialogiconbutton, initView)
{
    mWidget->initView();
    EXPECT_NE(mWidget->m_dialog, nullptr);
    EXPECT_TRUE(mWidget->isFlat());
}

// void CDialogIconButton::slotIconClicked()
TEST_F(test_cdialogiconbutton, slotIconClicked)
{
    Stub stub;

    typedef QDate (*fptr_getSelectDate)();
    fptr_getSelectDate f_getSelectDate = (fptr_getSelectDate)(&CScheduleBaseWidget::getSelectDate);
    stub.set(f_getSelectDate, stub_getSelectDate);

    typedef void (*fptr_showPopup)(TimeJumpDialog*, const QDate&, int, int);
    fptr_showPopup f_showPopup = reinterpret_cast<fptr_showPopup>(static_cast<void (TimeJumpDialog::*)(const QDate&, int, int)>(&TimeJumpDialog::showPopup));
    stub.set(f_showPopup, stub_showPopup);

    mWidget->slotIconClicked();
    SUCCEED();
}
