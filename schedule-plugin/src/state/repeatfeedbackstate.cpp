// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "repeatfeedbackstate.h"
#include "commondef.h"
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
        qCDebug(CommonLogger) << "Getting reply by intent - returning error TTS reply";
        Q_UNUSED(isOK)
        Reply reply;
        REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, true)
        return reply;
    } else {
        qCDebug(CommonLogger) << "Getting reply by intent - transitioning to init state";
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
        qCDebug(CommonLogger) << "Event filter: Normal state - Property status:" << jsonData->getPropertyStatus();
        return Fileter_Normal;
    }

    if (jsonData->getPropertyStatus() == JsonData::LAST
        || jsonData->offset() > 0) {
        qCDebug(CommonLogger) << "Event filter: Error state - Property status:" << jsonData->getPropertyStatus()
                             << "Offset:" << jsonData->offset();
        return Fileter_Err;
    }

    Filter_Flag result = changeDateErrJudge(jsonData, Fileter_Init);
    qCDebug(CommonLogger) << "Event filter: Date error judgment result:" << result;
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
        qCDebug(CommonLogger) << "Processing repeat schedule for all instances";
        isOnlyOne = false;
    } else {
        qCDebug(CommonLogger) << "Processing repeat schedule for single instance";
    }

    return m_Task->repeatScheduleHandle(m_localData->SelectInfo(), isOnlyOne);
}
