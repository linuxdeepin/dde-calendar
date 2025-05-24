// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendarprogramexitcontrol.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QCoreApplication>
#include <QTimer>

Q_LOGGING_CATEGORY(ExitControlLog, "calendar.exitcontrol")

bool CalendarProgramExitControl::m_clientIsOpen = false;
CalendarProgramExitControl *CalendarProgramExitControl::getProgramExitControl()
{
    static CalendarProgramExitControl programExitControl;
    qCDebug(ExitControlLog) << "Getting program exit control instance";
    return  &programExitControl;
}

CalendarProgramExitControl *CalendarProgramExitControl::operator->() const
{
    return getProgramExitControl();
}

void CalendarProgramExitControl::addExc()
{
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    qCDebug(ExitControlLog) << "Adding execution count";
    readWriteLock.lockForWrite();
    ++m_excNum;
    qCDebug(ExitControlLog) << "Current execution count:" << m_excNum;
    readWriteLock.unlock();
#endif
}

void CalendarProgramExitControl::reduce()
{
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    qCDebug(ExitControlLog) << "Scheduling execution count reduction";
    //3秒后退出,防止程序频繁的开启关闭
    QTimer::singleShot(3000, [=] {
        readWriteLock.lockForWrite();
        --m_excNum;
        qCDebug(ExitControlLog) << "Reduced execution count to:" << m_excNum;
        if (m_excNum < 1 && !m_clientIsOpen) {
            qCInfo(ExitControlLog) << "No active executions and client closed, initiating exit";
            exit();
        }
        readWriteLock.unlock();
    });
#endif
}

void CalendarProgramExitControl::exit()
{
#ifdef NDEBUG
    qCInfo(ExitControlLog) << "Exiting application";
    qApp->exit();
#endif
}

bool CalendarProgramExitControl::getClientIsOpen()
{
    qCDebug(ExitControlLog) << "Getting client open status:" << m_clientIsOpen;
    return m_clientIsOpen;
}

void CalendarProgramExitControl::setClientIsOpen(bool clientIsOpen)
{
    qCDebug(ExitControlLog) << "Setting client open status to:" << clientIsOpen;
    m_clientIsOpen = clientIsOpen;
}

CalendarProgramExitControl::CalendarProgramExitControl()
{
    qCDebug(ExitControlLog) << "Initializing program exit control";
}
