// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulemanagetask.h"

#include "../dbus/schedulesdbus.h"
#include "../globaldef.h"
#include "../widget/schedulelistwidget.h"
#include "../data/createjsondata.h"
#include "../data/queryjsondata.h"
#include "../data/canceljsondata.h"
#include "../data/changejsondata.h"
#include "../widget/viewschedulewidget.h"
#include "createscheduletask.h"
#include "queryscheduletask.h"
#include "cancelscheduletask.h"
#include "changescheduletask.h"

DWIDGET_USE_NAMESPACE

ScheduleManageTask *ScheduleManageTask::m_scheduleManageTask = nullptr;

ScheduleManageTask::ScheduleManageTask(QObject *parent)
    : QObject(parent)
    , m_dbus(new CSchedulesDBus(DBUS_SERVICE, DBUS_PATCH, QDBusConnection::sessionBus(), this))
{
    m_scheduleTaskMap[JSON_CREATE] = new createScheduleTask(m_dbus);
    m_scheduleTaskMap[JSON_VIEW] = new queryScheduleTask(m_dbus);
    m_scheduleTaskMap[JSON_CANCEL] = new cancelScheduleTask(m_dbus);
    m_scheduleTaskMap[JSON_CHANGE] = new changeScheduleTask(m_dbus);

    QMap<QString, scheduleBaseTask *>::Iterator inter = m_scheduleTaskMap.begin();
    for (; inter != m_scheduleTaskMap.end(); ++inter) {
        scheduleBaseTask *task = inter.value();
        connect(task, &scheduleBaseTask::signaleSendMessage, this, &ScheduleManageTask::signaleSendMessage);
    }
}

ScheduleManageTask::~ScheduleManageTask()
{
    QMap<QString, scheduleBaseTask *>::Iterator inter = m_scheduleTaskMap.begin();
    for (; inter != m_scheduleTaskMap.end(); ++inter) {
        scheduleBaseTask *task = inter.value();
        delete task;
    }
    m_scheduleTaskMap.clear();
}

ScheduleManageTask *ScheduleManageTask::getInstance()
{
    if (m_scheduleManageTask == nullptr) {
        m_scheduleManageTask = new ScheduleManageTask();
    }
    return m_scheduleManageTask;
}

void ScheduleManageTask::releaseInstance()
{
    if (m_scheduleManageTask != nullptr) {
        delete m_scheduleManageTask;
        m_scheduleManageTask = nullptr;
    }
}

void ScheduleManageTask::process(semanticAnalysisTask &semanticTask)
{
    if (m_scheduleTaskMap.contains(semanticTask.Intent())) {
        if (m_preScheduleTask != m_scheduleTaskMap[semanticTask.Intent()]) {
            if (m_preScheduleTask != nullptr)
                m_preScheduleTask->InitState(nullptr, true);
            m_preScheduleTask = m_scheduleTaskMap[semanticTask.Intent()];
        }
    }
    Reply reply;
    if (m_preScheduleTask == nullptr){
        REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, true);
    } else {
        reply =m_preScheduleTask->SchedulePress(semanticTask);
        connectHideEventToInitState(reply);
    }
    setReply(reply);
}

void ScheduleManageTask::slotReceivce(QVariant data, Reply *reply)
{
    Q_UNUSED(data);
    Q_UNUSED(reply);
}

void ScheduleManageTask::slotWidgetHideInitState()
{
    if (m_preScheduleTask != nullptr)
        m_preScheduleTask->InitState(nullptr, true);
}

void ScheduleManageTask::connectHideEventToInitState(Reply reply)
{
    //判断回复内容是否有回复窗口
    if(reply.getReplyWidget() !=nullptr){
        //转换为IconDFrame窗口
        IconDFrame *_iconWidget = qobject_cast<IconDFrame *>(reply.getReplyWidget());
        if(_iconWidget != nullptr){
            //如果转换成功则关联
            connect(_iconWidget, &IconDFrame::widgetIsHide,
                    this, &ScheduleManageTask::slotWidgetHideInitState, Qt::UniqueConnection);
        }
    }
}

Reply ScheduleManageTask::getReply() const
{
    return m_Reply;
}

void ScheduleManageTask::setReply(const Reply &reply)
{
    m_Reply = reply;
}
