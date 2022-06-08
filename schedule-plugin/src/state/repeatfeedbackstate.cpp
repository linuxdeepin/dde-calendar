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
#include "repeatfeedbackstate.h"
#include "../globaldef.h"
#include "../task/schedulebasetask.h"
#include "../data/clocaldata.h"
#include "../data/changejsondata.h"

repeatfeedbackstate::repeatfeedbackstate(scheduleBaseTask *task)
    : scheduleState(task)
{
}

Reply repeatfeedbackstate::getReplyByIntent(bool isOK)
{
    if (isOK) {
        Q_UNUSED(isOK)
        Reply reply;
        REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, true)
        return reply;
    } else {
        return m_Task->InitState(nullptr);
    }
}

scheduleState::Filter_Flag repeatfeedbackstate::eventFilter(const JsonData *jsonData)
{
    if (jsonData->getPropertyStatus() == JsonData::NEXT
        //如果语义包含时间则为修改初始状态
        || jsonData->getDateTime().suggestDatetime.size() > 0
        // 如果语义包含内容则为修改初始状态
        || !jsonData->TitleName().isEmpty()
        //如果语义包含重复类型则为修改初始状态
        || jsonData->getRepeatStatus() != JsonData::NONE) {
        return Fileter_Init;
    }
    if (jsonData->getPropertyStatus() == JsonData::ALL
        || jsonData->getPropertyStatus() == JsonData::PRO_THIS) {
        return Fileter_Normal;
    }
    if (jsonData->getPropertyStatus() == JsonData::LAST
        || jsonData->offset() > 0) {
        return Fileter_Err;
    }
    Filter_Flag result = changeDateErrJudge(jsonData, Fileter_Init);
    return result;
}

Reply repeatfeedbackstate::ErrEvent()
{
    Reply reply;
    REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, true)
    return reply;
}

Reply repeatfeedbackstate::normalEvent(const JsonData *jsonData)
{
    bool isOnlyOne = true;
    if (jsonData->getPropertyStatus() == JsonData::ALL) {
        isOnlyOne = false;
    }
    return m_Task->repeatScheduleHandle(m_localData->SelectInfo(), isOnlyOne);
}
