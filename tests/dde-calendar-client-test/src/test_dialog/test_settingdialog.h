// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_SETTINGDIALOG_H
#define TEST_SETTINGDIALOG_H

#include <QObject>
#include <gtest/gtest.h>
#include "dialog/settingdialog.h"

class test_settingdialog : public QObject, public testing::Test
{
public:
    test_settingdialog();
    ~test_settingdialog();
protected:
    CSettingDialog *mDialog = nullptr;
    bool mConstructed = false;
};

#endif // TEST_SETTINGDIALOG_H
