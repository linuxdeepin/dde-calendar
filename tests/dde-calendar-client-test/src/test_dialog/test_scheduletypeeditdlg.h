// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_SCHEDULETYPEEDITDLG_H
#define TEST_SCHEDULETYPEEDITDLG_H

#include <QObject>
#include <gtest/gtest.h>
#include "dialog/scheduletypeeditdlg.h"

class test_scheduletypeeditdlg : public QObject, public testing::Test
{
public:
    test_scheduletypeeditdlg();
    ~test_scheduletypeeditdlg();
protected:
    ScheduleTypeEditDlg *mDlgNew = nullptr;
    ScheduleTypeEditDlg *mDlgImport = nullptr;
};

#endif // TEST_SCHEDULETYPEEDITDLG_H
