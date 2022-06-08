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
#include "schedulestate.h"

#include "../task/schedulebasetask.h"
#include "../data/changejsondata.h"
#include "../globaldef.h"
#include "../state/queryschedulestate.h"

scheduleState::scheduleState(scheduleBaseTask *task)
    : m_Task(task)
{
}

scheduleState::~scheduleState()
{
}

Reply scheduleState::process(const JsonData *jsonData)
{
    Reply reply;
    //如果时间无效
    if (jsonData->getDateTimeInvalid()) {
        scheduleState *nextState = new queryScheduleState(m_Task);
        setNextState(nextState);
        REPLY_ONLY_TTS(reply, DATETIME_ERR_TTS, DATETIME_ERR_TTS, true);
        return reply;
    }
    switch (eventFilter(jsonData)) {
    case Fileter_Err: {
        reply = ErrEvent();
    } break;
    case Fileter_Normal: {
        reply = normalEvent(jsonData);
    } break;
    case Fileter_Init: {
        reply = initEvent(jsonData);
    } break;
    }
    return reply;
}

void scheduleState::setNextState(scheduleState *nextState)
{
    m_nextState = nextState;
}

scheduleState *scheduleState::getNextState() const
{
    return m_nextState;
}

void scheduleState::setLocalData(const CLocalData::Ptr &localData)
{
    m_localData = localData;
}

CLocalData::Ptr scheduleState::getLocalData() const
{
    return m_localData;
}

Reply scheduleState::initEvent(const JsonData *jsonData)
{
    return m_Task->InitState(jsonData);
}

scheduleState::Filter_Flag scheduleState::changeDateErrJudge(const JsonData *jsonData, const scheduleState::Filter_Flag &defaultflag)
{
    Filter_Flag resultFlag {defaultflag};
    //如果只有修改信息没有被修改信息返回错误
    JsonData *queryData = const_cast<JsonData *>(jsonData);
    changejsondata *mchangeJsonData = dynamic_cast<changejsondata *>(queryData);
    if (mchangeJsonData != nullptr) {
        //是否含有修改信息
        bool hasChangeToData = !mchangeJsonData->toPlaceStr().isEmpty() || mchangeJsonData->toDateTime().suggestDatetime.size() > 0;
        //是否不包含需要修改的信息
        bool noChangeDate = mchangeJsonData->fromDateTime().suggestDatetime.size() == 0
                            && mchangeJsonData->TitleName().isEmpty();
        if (hasChangeToData && noChangeDate) {
            resultFlag = Filter_Flag::Fileter_Err;
        }
    }
    return resultFlag;
}
