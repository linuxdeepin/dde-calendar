/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "dbusdatagetthread.h"

#include "schedulesdbus.h"

DbusDataGetThread::DbusDataGetThread(CSchedulesDBus *_DataManage, QObject *parent)
    :QObject (parent),
     m_work(new DataGetWork(_DataManage))
{
    m_work->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, m_work, &QObject::deleteLater);
    connect(this,&DbusDataGetThread::getScheduleInfo,
            m_work,&DataGetWork::getScheduleInfo);
    connect(this,&DbusDataGetThread::getLunarInfo,
            m_work,&DataGetWork::getLunarInfo);
    connect(this,&DbusDataGetThread::getFestivalInfo,
            m_work,&DataGetWork::getFestivalInfo);

    connect(m_work,&DataGetWork::signalGetScheduleSuccess,
            this,&DbusDataGetThread::signalGetScheduleSuccess);

    workerThread.start();
}

DbusDataGetThread::~DbusDataGetThread()
{
    workerThread.quit();
    workerThread.wait();
}

DataGetWork::DataGetWork(CSchedulesDBus *_DataManage)
    :m_DataManage(_DataManage)
{
    Q_ASSERT(m_DataManage != nullptr);
}

DataGetWork::~DataGetWork()
{

}

void DataGetWork::getScheduleInfo(YearScheduleInfo *ScheduleInfo)
{
    QDate firstDay = ScheduleInfo->m_firstDay[1];
    QDate endDay = firstDay.addDays(41);
    ScheduleInfo->m_monthInfo.clear();
    QVector<ScheduleDateRangeInfo> out;

    for (int i = 0 ; i < 12 ; ++i) {
        out.clear();
        firstDay = ScheduleInfo->m_firstDay[i+1];
        endDay  = firstDay.addDays(41);
        m_DataManage->GetJobs(firstDay.year(),
                              firstDay.month(),
                              firstDay.day(),
                              endDay.year(),
                              endDay.month(),
                              endDay.day(),
                              out);
        ScheduleInfo->m_monthInfo[i+1] = out;
    }
    emit signalGetScheduleSuccess();
}

void DataGetWork::getLunarInfo(YearLunar *lunarInfo)
{
    Q_UNUSED(lunarInfo);

}

void DataGetWork::getFestivalInfo(YearFestival *festivalInfo)
{
    Q_UNUSED(festivalInfo);
}

