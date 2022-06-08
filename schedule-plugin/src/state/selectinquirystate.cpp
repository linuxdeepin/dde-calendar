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
#include "selectinquirystate.h"

#include "../globaldef.h"
#include "../widget/repeatschedulewidget.h"
#include "../task/schedulebasetask.h"

selectInquiryState::selectInquiryState(scheduleBaseTask *task)
    : scheduleState(task)
{
}

Reply selectInquiryState::getReplyByIntent(bool isOK)
{
    Q_UNUSED(isOK)
    Reply reply;
    REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, true)
    return reply;
}

scheduleState::Filter_Flag selectInquiryState::eventFilter(const JsonData *jsonData)
{
    if (jsonData->getPropertyStatus() == JsonData::ALL
        || jsonData->getPropertyStatus() == JsonData::NEXT
        || jsonData->isVaild()
        || jsonData->getRepeatStatus() != JsonData::NONE) {
        return Filter_Flag::Fileter_Init;
    }
    if (jsonData->getPropertyStatus() == JsonData::LAST)
        return Fileter_Normal;

    if (jsonData->getDateTime().suggestDatetime.size() > 0
        || !jsonData->TitleName().isEmpty()) {
        return Fileter_Init;
    }

    bool showOpenWidget = m_localData->scheduleInfoVector().size() > ITEM_SHOW_NUM;
    const int showcount = showOpenWidget ? ITEM_SHOW_NUM : m_localData->scheduleInfoVector().size();
    if (jsonData->offset() > showcount) {
        return Fileter_Err;
    }
    return Fileter_Normal;
}

Reply selectInquiryState::ErrEvent()
{
    Reply reply;
    REPLY_ONLY_TTS(reply, QUERY_ERR_TTS, QUERY_ERR_TTS, true)
    return reply;
}

Reply selectInquiryState::normalEvent(const JsonData *jsonData)
{
    bool showOpenWidget = m_localData->scheduleInfoVector().size() > ITEM_SHOW_NUM;
    const int showcount = showOpenWidget ? ITEM_SHOW_NUM : m_localData->scheduleInfoVector().size();
    int offset = 0;
    if (jsonData->getPropertyStatus() == JsonData::LAST) {
        offset = showcount;
    } else {
        offset = jsonData->offset();
    }
    Reply m_reply;
    DSchedule::Ptr info = m_localData->scheduleInfoVector().at(offset - 1);

    return m_Task->getReplyBySelectSchedule(info);
}
