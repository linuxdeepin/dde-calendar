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
#include "queryschedulestate.h"
#include "../task/queryscheduleproxy.h"
#include "../task/schedulebasetask.h"
#include "dschedule.h"
#include "../data/changejsondata.h"
#include "../globaldef.h"
#include "../data/clocaldata.h"

queryScheduleState::queryScheduleState(scheduleBaseTask *task)
    : scheduleState(task)
{
}

Reply queryScheduleState::getReplyByIntent(bool isOK)
{
    Q_UNUSED(isOK);
    return ErrEvent();
}

Reply queryScheduleState::ErrEvent()
{
    Reply reply;
    REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, false)
    return reply;
}

Reply queryScheduleState::normalEvent(const JsonData *jsonData)
{
    DSchedule::List m_scheduleInfo {};
    JsonData *queryData = const_cast<JsonData *>(jsonData);
    queryScheduleProxy m_querySchedule(queryData);
    m_scheduleInfo = m_querySchedule.scheduleMapToList(m_querySchedule.querySchedule());
    if (m_querySchedule.getTimeIsExpired()) {
        return m_Task->overdueScheduleProcess();
    } else {
        changejsondata *mchangeJsonData = dynamic_cast<changejsondata *>(queryData);
        if (mchangeJsonData != nullptr) {
            if (m_localData.isNull())
                m_localData = CLocalData::Ptr(new CLocalData());
            if (mchangeJsonData->toDateTime().suggestDatetime.size() > 0) {
                m_localData->setToTime(mchangeJsonData->toDateTime());
            }
            if (!mchangeJsonData->toPlaceStr().isEmpty())
                m_localData->setToTitleName(mchangeJsonData->toPlaceStr());
        }
        return m_Task->getFeedbackByQuerySchedule(m_scheduleInfo);
    }
}

scheduleState::Filter_Flag queryScheduleState::eventFilter(const JsonData *jsonData)
{
    if (jsonData->getPropertyStatus() == JsonData::LAST
        || jsonData->getPropertyStatus() == JsonData::PRO_THIS)
        return Fileter_Err;
    if (jsonData->offset() > -1 && jsonData->getPropertyStatus() == JsonData::PRO_NONE)
        return Fileter_Err;
    Filter_Flag result = changeDateErrJudge(jsonData, Fileter_Normal);
    return result;
}
