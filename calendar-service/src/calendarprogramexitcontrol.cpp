#include "calendarprogramexitcontrol.h"

#include <QCoreApplication>
#include <QTimer>

bool CalendarProgramExitControl::m_clientIsOpen = false;
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
    //3秒后退出,防止程序频繁的开启关闭
    QTimer::singleShot(3000, [=] {
        readWriteLock.lockForWrite();
        --m_excNum;
        if (m_excNum < 1 && !m_clientIsOpen) {
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

bool CalendarProgramExitControl::getClientIsOpen()
{
    return m_clientIsOpen;
}

void CalendarProgramExitControl::setClientIsOpen(bool clientIsOpen)
{
    m_clientIsOpen = clientIsOpen;
}

CalendarProgramExitControl::CalendarProgramExitControl()
{

}
