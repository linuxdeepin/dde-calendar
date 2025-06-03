// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "selectinquirystate.h"
#include "commondef.h"

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
        qCDebug(CommonLogger) << "Event filter: Initial state detected due to property status or repeat status";
        return Filter_Flag::Fileter_Init;
    }

    if (jsonData->getPropertyStatus() == JsonData::LAST) {
        qCDebug(CommonLogger) << "Event filter: Normal state detected for LAST property status";
        return Fileter_Normal;
    }

    if (jsonData->getDateTime().suggestDatetime.size() > 0
        || !jsonData->TitleName().isEmpty()) {
        qCDebug(CommonLogger) << "Event filter: Initial state detected due to datetime or title";
        return Fileter_Init;
    }

    bool showOpenWidget = m_localData->scheduleInfoVector().size() > ITEM_SHOW_NUM;
    const int showcount = showOpenWidget ? ITEM_SHOW_NUM : m_localData->scheduleInfoVector().size();
    if (jsonData->offset() > showcount) {
        qCDebug(CommonLogger) << "Event filter: Error state detected - offset exceeds show count";
        return Fileter_Err;
    }

    qCDebug(CommonLogger) << "Event filter: Normal state detected";
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
        qCDebug(CommonLogger) << "Processing LAST property status, using showcount as offset:" << showcount;
        offset = showcount;
    } else {
        offset = jsonData->offset();
        qCDebug(CommonLogger) << "Using provided offset:" << offset;
    }
    Reply m_reply;
    DSchedule::Ptr info = m_localData->scheduleInfoVector().at(offset - 1);
    qCDebug(CommonLogger) << "Getting reply for selected schedule with ID:" << info->uid();
    return m_Task->getReplyBySelectSchedule(info);
}
