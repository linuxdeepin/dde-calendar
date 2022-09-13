// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cancelscheduletask.h"
#include "../data/canceljsondata.h"
#include "../globaldef.h"
#include "../widget/schedulelistwidget.h"
#include "../widget/repeatschedulewidget.h"
#include "queryscheduleproxy.h"
#include "../state/queryschedulestate.h"
#include "../state/selectinquirystate.h"
#include "../state/repeatfeedbackstate.h"
#include "../data/clocaldata.h"
#include "../state/confirwfeedbackstate.h"

cancelScheduleTask::cancelScheduleTask(CSchedulesDBus *dbus)
    : scheduleBaseTask(dbus, new queryScheduleState(dbus, this))
{
}

void cancelScheduleTask::slotSelectScheduleIndex(int index)
{
    scheduleState *currentState = getCurrentState();
    CLocalData *localData = currentState->getLocalData();
    if (!(localData->scheduleInfoVector().size() < index)) {
        Reply reply = getReplyBySelectSchedule(localData->scheduleInfoVector().at(index - 1));
        updateState();
        emit signaleSendMessage(reply);
    }
}

void cancelScheduleTask::slotButtonCheckNum(int index, const QString &text, const int buttonCount)
{
    Q_UNUSED(text);
    Reply reply;
    scheduleState *currentState = getCurrentState();
    if (buttonCount == 2) {
        if (index == 1) {
            reply = confirwScheduleHandle(currentState->getLocalData()->SelectInfo());
        }
    }
    if (buttonCount == 3) {
        if (index == 1) {
            reply = repeatScheduleHandle(currentState->getLocalData()->SelectInfo(), false);
        }
        if (index == 2) {
            reply = repeatScheduleHandle(currentState->getLocalData()->SelectInfo(), true);
        }
    }
    if (index == 0) {
        reply = InitState(nullptr, true);
    } else {
        InitState(nullptr, true);
    }
    emit signaleSendMessage(reply);
}

scheduleState *cancelScheduleTask::getCurrentState()
{
    scheduleState *currentState = m_State;
    while (currentState->getNextState() != nullptr) {
        currentState = currentState->getNextState();
    }
    return currentState;
}

Reply cancelScheduleTask::getFeedbackByQuerySchedule(const QVector<ScheduleDtailInfo> &infoVector)
{
    Reply m_reply;
    scheduleState *nextState = nullptr;
    scheduleState *currentState = getCurrentState();
    if (infoVector.size() == 0) {
        QString m_TTSMessage;
        QString m_DisplyMessage;
        m_TTSMessage = NO_SCHEDULE_TTS;
        m_DisplyMessage = NO_SCHEDULE_TTS;
        REPLY_ONLY_TTS(m_reply, m_TTSMessage, m_DisplyMessage, true);
        currentState->setNextState(nextState);
    } else if (infoVector.size() == 1) {
        m_reply = getReplyBySelectSchedule(infoVector.at(0));
    } else {
        nextState = new selectInquiryState(m_dbus, this);
        CLocalData *m_Data = new CLocalData();
        m_Data->setScheduleInfoVector(infoVector);
        nextState->setLocalData(m_Data);
        m_reply = getListScheduleReply(infoVector);
        currentState->setNextState(nextState);
    }
    return m_reply;
}

Reply cancelScheduleTask::getReplyBySelectSchedule(const ScheduleDtailInfo &info)
{
    Reply m_reply;
    CLocalData *m_Data = new CLocalData();
    scheduleState *nextState = nullptr;
    scheduleState *currentState = getCurrentState();
    m_Data->setSelectInfo(info);
    if (info.rpeat == 0) {
        nextState = new confirwFeedbackState(m_dbus, this);
        m_reply = getConfirwScheduleReply(info);
    } else {
        nextState = new repeatfeedbackstate(m_dbus, this);
        m_reply = getRepeatReply(info);
    }
    nextState->setLocalData(m_Data);
    currentState->setNextState(nextState);
    return m_reply;
}

Reply cancelScheduleTask::InitState(const JsonData *jsonData, bool isUpdateState)
{
    Reply m_reply;
    scheduleState *nextState = new queryScheduleState(m_dbus, this);
    scheduleState *currentState = getCurrentState();
    currentState->setNextState(nextState);
    if (jsonData != nullptr) {
        m_reply = nextState->process(jsonData);
    } else {
        REPLY_ONLY_TTS(m_reply, CANCEL_DELETION_TTS, CANCEL_DELETION_TTS, true);
    }
    if (isUpdateState) {
        updateState();
    }
    return m_reply;
}

Reply cancelScheduleTask::repeatScheduleHandle(const ScheduleDtailInfo &info, bool isOnlyOne)
{
    deleteRepeatSchedule(info, isOnlyOne);
    Reply reply;
    REPLY_ONLY_TTS(reply, CONFIRM_DELETION_TTS, CONFIRM_DELETION_TTS, true);
    scheduleState *nextState = new queryScheduleState(m_dbus, this);
    scheduleState *currentState = getCurrentState();
    currentState->setNextState(nextState);
    return reply;
}

Reply cancelScheduleTask::confirwScheduleHandle(const ScheduleDtailInfo &info)
{
    deleteOrdinarySchedule(info);
    Reply reply;
    REPLY_ONLY_TTS(reply, CONFIRM_DELETION_TTS, CONFIRM_DELETION_TTS, true);
    scheduleState *nextState = new queryScheduleState(m_dbus, this);
    scheduleState *currentState = getCurrentState();
    currentState->setNextState(nextState);
    return reply;
}

Reply cancelScheduleTask::confirmInfo(bool isOK)
{
    if (isOK) {
        scheduleState *currentState = getCurrentState();
        return confirwScheduleHandle(currentState->getLocalData()->SelectInfo());
    } else {
        return InitState(nullptr);
    }
}

QWidget *cancelScheduleTask::createRepeatWidget(const ScheduleDtailInfo &info)
{
    repeatScheduleWidget *repeatWidget = new repeatScheduleWidget(repeatScheduleWidget::Operation_Cancel, repeatScheduleWidget::Widget_Repeat);
    repeatWidget->setSchedule(info);
    connect(repeatWidget, &repeatScheduleWidget::signalButtonCheckNum, this, &cancelScheduleTask::slotButtonCheckNum);
    return repeatWidget;
}

QWidget *cancelScheduleTask::createConfirmWidget(const ScheduleDtailInfo &info)
{
    repeatScheduleWidget *cwidget = new repeatScheduleWidget(repeatScheduleWidget::Operation_Cancel, repeatScheduleWidget::Widget_Confirm);
    cwidget->setSchedule(info);
    connect(cwidget, &repeatScheduleWidget::signalButtonCheckNum, this, &cancelScheduleTask::slotButtonCheckNum);
    return cwidget;
}

Reply cancelScheduleTask::getListScheduleReply(const QVector<ScheduleDtailInfo> &infoVector)
{
    scheduleListWidget *m_viewWidget = new scheduleListWidget();
    m_viewWidget->setScheduleInfoVector(infoVector);
    connect(m_viewWidget, &scheduleListWidget::signalSelectScheduleIndex, this, &cancelScheduleTask::slotSelectScheduleIndex);
    QString m_TTSMessage;
    QString m_DisplyMessage;
    m_TTSMessage = CANCEL_SELECT_TTS;
    m_DisplyMessage = CANCEL_SELECT_TTS;
    Reply reply;
    REPLY_WIDGET_TTS(reply, m_viewWidget, m_TTSMessage, m_DisplyMessage, false);
    return reply;
}

Reply cancelScheduleTask::getConfirwScheduleReply(const ScheduleDtailInfo &info)
{
    QString m_TTSMessage;
    QString m_DisplyMessage;
    m_TTSMessage = CONFIRM_SCHEDULE_CANCEL_TTS;
    m_DisplyMessage = CONFIRM_SCHEDULE_CANCEL_TTS;
    QWidget *m_confirwWidget = createConfirmWidget(info);
    Reply reply;
    REPLY_WIDGET_TTS(reply, m_confirwWidget, m_TTSMessage, m_DisplyMessage, false);
    return reply;
}

Reply cancelScheduleTask::getRepeatReply(const ScheduleDtailInfo &info)
{
    QString m_TTSMessage;
    QString m_DisplyMessage;
    m_TTSMessage = REPEST_SCHEDULE_CANCEL_TTS;
    m_DisplyMessage = REPEST_SCHEDULE_CANCEL_TTS;
    QWidget *m_repeatReply = createRepeatWidget(info);
    Reply reply;
    REPLY_WIDGET_TTS(reply, m_repeatReply, m_TTSMessage, m_DisplyMessage, false);
    return reply;
}

void cancelScheduleTask::deleteRepeatSchedule(const ScheduleDtailInfo &info, bool isOnlyOne)
{
    if (isOnlyOne) {
        ScheduleDtailInfo newschedule;
        m_dbus->GetJob(info.id, newschedule);
        newschedule.ignore.append(info.beginDateTime);
        m_dbus->UpdateJob(newschedule);
    } else {
        if (info.RecurID == 0) {
            m_dbus->DeleteJob(info.id);
        } else {
            ScheduleDtailInfo newschedule;
            m_dbus->GetJob(info.id, newschedule);
            newschedule.enddata.type = 2;
            newschedule.enddata.date = info.beginDateTime.addDays(-1);
            m_dbus->UpdateJob(newschedule);
        }
    }
}

void cancelScheduleTask::deleteOrdinarySchedule(const ScheduleDtailInfo &info)
{
    m_dbus->DeleteJob(info.id);
}
