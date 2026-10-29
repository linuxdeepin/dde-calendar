// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cbuttonbox.h"

#include <QTest>
#include <QFocusEvent>
#include <QToolButton>

//CButtonBox::CButtonBox(QWidget *parent)
TEST_F(test_cbuttonbox, constructor)
{
    CButtonBox *box = new CButtonBox();
    EXPECT_EQ(box->focusPolicy(), Qt::TabFocus);
    delete box;
}

//void CButtonBox::focusInEvent(QFocusEvent *event)
TEST_F(test_cbuttonbox, focusInEvent_ActiveWindow)
{
    QFocusEvent event(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
    QApplication::sendEvent(m_buttonBox, &event);
    SUCCEED();
}

TEST_F(test_cbuttonbox, focusInEvent_NonActiveWindow)
{
    QFocusEvent event(QEvent::FocusIn, Qt::TabFocusReason);
    QApplication::sendEvent(m_buttonBox, &event);
    SUCCEED();
}

TEST_F(test_cbuttonbox, focusInEvent_MouseReason)
{
    QFocusEvent event(QEvent::FocusIn, Qt::MouseFocusReason);
    QApplication::sendEvent(m_buttonBox, &event);
    SUCCEED();
}

//void CButtonBox::focusOutEvent(QFocusEvent *event)
TEST_F(test_cbuttonbox, focusOutEvent_TabReason)
{
    QFocusEvent event(QEvent::FocusOut, Qt::TabFocusReason);
    QApplication::sendEvent(m_buttonBox, &event);
    SUCCEED();
}

TEST_F(test_cbuttonbox, focusOutEvent_NonTabReason)
{
    QFocusEvent event(QEvent::FocusOut, Qt::MouseFocusReason);
    QApplication::sendEvent(m_buttonBox, &event);
    SUCCEED();
}
