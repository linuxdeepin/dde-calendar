// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendarprogramexitcontrol.h"

#include <QCoreApplication>
#include <QTimer>

CalendarProgramExitControl *CalendarProgramExitControl::getProgramExitControl()
{
    static CalendarProgramExitControl programExitControl;
    return  &programExitControl;
}

CalendarProgramExitControl *CalendarProgramExitControl::operator->() const
{
    return getProgramExitControl();
}

void CalendarProgramExitControl::addExc()
{
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    readWriteLock.lockForWrite();
    ++m_excNum;
    readWriteLock.unlock();
#endif
}

void CalendarProgramExitControl::reduce()
{
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    //1秒后退出,防止程序频繁的开启关闭
    QTimer::singleShot(1000,[=]{
    readWriteLock.lockForWrite();
    --m_excNum;
    if ( m_excNum < 1 ) {
       exit();
    }
    readWriteLock.unlock();
    });
#endif
}

void CalendarProgramExitControl::exit()
{
    qApp->exit();
}

CalendarProgramExitControl::CalendarProgramExitControl()
{

}
