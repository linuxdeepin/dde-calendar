// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CALENDARPROGRAMEXITCONTROL_H
#define TEST_CALENDARPROGRAMEXITCONTROL_H

#include "calendarprogramexitcontrol.h"
#include "gtest/gtest.h"
#include <QObject>

class test_calendarprogramexitcontrol : public QObject, public::testing::Test
{
public:
    test_calendarprogramexitcontrol();
    ~test_calendarprogramexitcontrol();
protected:
    CalendarProgramExitControl *m_control = nullptr;
};

#endif // TEST_CALENDARPROGRAMEXITCONTROL_H
