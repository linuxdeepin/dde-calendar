// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendarprogramexitcontrol.h"
#include "commondef.h"

#include <QCoreApplication>
#include <QTimer>

bool CalendarProgramExitControl::m_clientIsOpen = false;
CalendarProgramExitControl *CalendarProgramExitControl::getProgramExitControl()
{
    // qCDebug(ServiceLogger) << "Getting program exit control instance";
    static CalendarProgramExitControl programExitControl;
    return  &programExitControl;
}

CalendarProgramExitControl *CalendarProgramExitControl::operator->() const
{
    // qCDebug(ServiceLogger) << "Using operator-> to get program exit control";
    return getProgramExitControl();
}

void CalendarProgramExitControl::addExc()
{
    qCDebug(ServiceLogger) << "Adding execution counter";
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    qCDebug(ServiceLogger) << "Auto exit enabled, incrementing execution number";
    readWriteLock.lockForWrite();
    ++m_excNum;
    qCDebug(ServiceLogger) << "Execution number incremented to:" << m_excNum;
    readWriteLock.unlock();
#endif
}

void CalendarProgramExitControl::reduce()
{
    qCDebug(ServiceLogger) << "Reducing execution counter";
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    qCDebug(ServiceLogger) << "Auto exit enabled, scheduling counter reduction";
    //3秒后退出,防止程序频繁的开启关闭
    QTimer::singleShot(3000, [=] {
        readWriteLock.lockForWrite();
        --m_excNum;
        if (m_excNum < 1 && !m_clientIsOpen) {
            qCDebug(ServiceLogger) << "Conditions met for exit: excNum <1 and client not open";
            exit();
        }
        readWriteLock.unlock();
    });
#endif
}

void CalendarProgramExitControl::exit()
{
    qCDebug(ServiceLogger) << "Exit requested";
#ifdef NDEBUG
    qCDebug(ServiceLogger) << "Release build, executing application exit";
    qApp->exit();
#endif
}

bool CalendarProgramExitControl::getClientIsOpen()
{
    // qCDebug(ServiceLogger) << "Getting client open status:" << m_clientIsOpen;
    return m_clientIsOpen;
}

void CalendarProgramExitControl::setClientIsOpen(bool clientIsOpen)
{
    // qCDebug(ServiceLogger) << "Setting client open status to:" << clientIsOpen;
    m_clientIsOpen = clientIsOpen;
}

CalendarProgramExitControl::CalendarProgramExitControl()
{
    qCDebug(ServiceLogger) << "Creating CalendarProgramExitControl instance";
}
