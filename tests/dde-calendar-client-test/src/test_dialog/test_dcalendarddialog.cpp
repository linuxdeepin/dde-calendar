// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dialog/dcalendarddialog.h"
#include "calendarmanage.h"

#include <gtest/gtest.h>

class TestCalendarDDialog : public DCalendarDDialog
{
public:
    using DCalendarDDialog::DCalendarDDialog;
    using DCalendarDDialog::setTimeFormat;

    QString timeFormat() const
    {
        return m_timeFormat;
    }
};

TEST(test_dcalendarddialog, usesTimeShowTypeForInitialTimeFormat)
{
    const QString oldTimeFormat = CalendarManager::getInstance()->getTimeFormat();
    const int oldTimeShowType = CalendarManager::getInstance()->getTimeShowType();
    CalendarManager::getInstance()->setTimeFormatChanged(0);
    CalendarManager::getInstance()->setTimeShowType(1, false);

    TestCalendarDDialog dialog;

    EXPECT_EQ(dialog.timeFormat(), QString("AP h:mm"));

    CalendarManager::getInstance()->setTimeFormatChanged(oldTimeFormat == "hh:mm" ? 1 : 0);
    CalendarManager::getInstance()->setTimeShowType(oldTimeShowType, false);
}

TEST(test_dcalendarddialog, keepsTimeShowTypeWhenShortTimeFormatChanges)
{
    const QString oldTimeFormat = CalendarManager::getInstance()->getTimeFormat();
    const int oldTimeShowType = CalendarManager::getInstance()->getTimeShowType();
    CalendarManager::getInstance()->setTimeShowType(1, false);
    TestCalendarDDialog dialog;

    dialog.setTimeFormat(0);

    EXPECT_EQ(dialog.timeFormat(), QString("AP h:mm"));

    CalendarManager::getInstance()->setTimeFormatChanged(oldTimeFormat == "hh:mm" ? 1 : 0);
    CalendarManager::getInstance()->setTimeShowType(oldTimeShowType, false);
}
