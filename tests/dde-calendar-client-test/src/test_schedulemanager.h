// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_SCHEDULEMANAGER_H
#define TEST_SCHEDULEMANAGER_H

#include "schedulemanager.h"
#include "gtest/gtest.h"
#include "../../third-party_stub/stub.h"
#include <QObject>

class test_schedulemanager : public QObject, public::testing::Test
{
public:
    test_schedulemanager();
    ~test_schedulemanager();
protected:
    ScheduleManager *manager = nullptr;
};

#endif // TEST_SCHEDULEMANAGER_H
