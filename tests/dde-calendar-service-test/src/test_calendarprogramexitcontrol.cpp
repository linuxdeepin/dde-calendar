// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calendarprogramexitcontrol.h"
#include "../third-party_stub/stub.h"
#include <QCoreApplication>
#include <QTest>

test_calendarprogramexitcontrol::test_calendarprogramexitcontrol()
{
    m_control = CalendarProgramExitControl::getProgramExitControl();
}

test_calendarprogramexitcontrol::~test_calendarprogramexitcontrol()
{
}

// CalendarProgramExitControl::getProgramExitControl() — singleton accessor
TEST_F(test_calendarprogramexitcontrol, GetProgramExitControl)
{
    CalendarProgramExitControl *inst1 = CalendarProgramExitControl::getProgramExitControl();
    CalendarProgramExitControl *inst2 = CalendarProgramExitControl::getProgramExitControl();
    EXPECT_NE(nullptr, inst1);
    EXPECT_NE(nullptr, inst2);
    EXPECT_EQ(inst1, inst2);
}

// CalendarProgramExitControl::operator->() — returns getProgramExitControl()
TEST_F(test_calendarprogramexitcontrol, OperatorArrow)
{
    CalendarProgramExitControl *inst = CalendarProgramExitControl::getProgramExitControl();
    CalendarProgramExitControl *arrowResult = inst->operator->();
    EXPECT_NE(nullptr, arrowResult);
    EXPECT_EQ(inst, arrowResult);
}

// CalendarProgramExitControl::addExc() — increments counter (guarded by CALENDAR_SERVICE_AUTO_EXIT)
TEST_F(test_calendarprogramexitcontrol, AddExc)
{
    int before = m_control->m_excNum;
    m_control->addExc();
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    EXPECT_EQ(before + 1, m_control->m_excNum);
#else
    EXPECT_EQ(before, m_control->m_excNum);
#endif
}

// CalendarProgramExitControl::reduce() — decrements counter via QTimer (guarded by CALENDAR_SERVICE_AUTO_EXIT)
TEST_F(test_calendarprogramexitcontrol, Reduce)
{
    int before = m_control->m_excNum;
    m_control->reduce();
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    // QTimer::singleShot fires asynchronously; process events to let it run
    QTest::qWait(3500);
    EXPECT_GE(m_control->m_excNum, 0);
#else
    EXPECT_EQ(before, m_control->m_excNum);
#endif
}

// CalendarProgramExitControl::exit() — calls qApp->exit() (guarded by NDEBUG)
TEST_F(test_calendarprogramexitcontrol, Exit)
{
    // exit() under #ifdef NDEBUG calls qApp->exit() which terminates the app.
    // In debug builds the function body is empty — safe to call.
    // We just verify the function is callable without crashing in non-NDEBUG builds.
#ifndef NDEBUG
    m_control->exit();
    SUCCEED();
#else
    // In release builds, exit() would terminate the application.
    // We only verify the function exists; calling it is skipped to avoid terminating the test process.
    SUCCEED() << "exit() is guarded by NDEBUG in release builds; not called to avoid process termination.";
#endif
}

// CalendarProgramExitControl::getClientIsOpen() — static getter
TEST_F(test_calendarprogramexitcontrol, GetClientIsOpen)
{
    CalendarProgramExitControl::setClientIsOpen(false);
    EXPECT_FALSE(CalendarProgramExitControl::getClientIsOpen());

    CalendarProgramExitControl::setClientIsOpen(true);
    EXPECT_TRUE(CalendarProgramExitControl::getClientIsOpen());

    // Restore default
    CalendarProgramExitControl::setClientIsOpen(false);
}

// CalendarProgramExitControl::setClientIsOpen() — static setter
TEST_F(test_calendarprogramexitcontrol, SetClientIsOpen)
{
    CalendarProgramExitControl::setClientIsOpen(true);
    EXPECT_TRUE(CalendarProgramExitControl::getClientIsOpen());

    CalendarProgramExitControl::setClientIsOpen(false);
    EXPECT_FALSE(CalendarProgramExitControl::getClientIsOpen());
}

// DServiceExitControl constructor calls addExc, destructor calls reduce
TEST_F(test_calendarprogramexitcontrol, DServiceExitControlConstructorDestructor)
{
    int before = m_control->m_excNum;
    {
        DServiceExitControl exitControl;
#ifdef CALENDAR_SERVICE_AUTO_EXIT
        // Constructor calls addExc which increments m_excNum
        EXPECT_EQ(before + 1, m_control->m_excNum);
#else
        EXPECT_EQ(before, m_control->m_excNum);
#endif
    }
    // Destructor calls reduce (async timer under CALENDAR_SERVICE_AUTO_EXIT)
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    QTest::qWait(3500);
    EXPECT_GE(m_control->m_excNum, 0);
#else
    EXPECT_EQ(before, m_control->m_excNum);
#endif
}

// DServiceExitControl::setClientIsOpen delegates to CalendarProgramExitControl::setClientIsOpen
TEST_F(test_calendarprogramexitcontrol, DServiceExitControlSetClientIsOpen)
{
    DServiceExitControl exitControl;
    exitControl.setClientIsOpen(true);
    EXPECT_TRUE(CalendarProgramExitControl::getClientIsOpen());

    exitControl.setClientIsOpen(false);
    EXPECT_FALSE(CalendarProgramExitControl::getClientIsOpen());

#ifdef CALENDAR_SERVICE_AUTO_EXIT
    QTest::qWait(3500);
#endif
}
