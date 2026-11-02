// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CSYSTEMDTIMERCONTROL_H
#define TEST_CSYSTEMDTIMERCONTROL_H

#include "csystemdtimercontrol.h"
#include "gtest/gtest.h"
#include <QObject>

class test_csystemdtimercontrol : public QObject, public::testing::Test
{
public:
    test_csystemdtimercontrol();
    ~test_csystemdtimercontrol();
protected:
    CSystemdTimerControl *m_timerControl = nullptr;
};

#endif // TEST_CSYSTEMDTIMERCONTROL_H
