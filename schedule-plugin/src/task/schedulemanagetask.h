// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULEMANAGETASK_H
#define SCHEDULEMANAGETASK_H

#include <QObject>

#include "../interface/reply.h"
#include "semanticanalysistask.h"
#include "../dbus/schedulesdbus.h"
#include "../widget/icondframe.h"
#include "../data/schedulestructs.h"
#include "../widget/createschedulewidget.h"
#include "../interface/reply.h"
#include "schedulebasetask.h"

//class widgetStrategy;
class CSchedulesDBus;
class ScheduleManageTask : public QObject
{
    Q_OBJECT
public:
    static ScheduleManageTask *getInstance();
    static void releaseInstance();

    void process(semanticAnalysisTask &semanticTask);

    Reply getReply() const;
    void setReply(const Reply &reply);

private:
    explicit ScheduleManageTask(QObject *parent = nullptr);
    ~ScheduleManageTask();
signals:
    void signaleSendMessage(Reply reply);
public slots:
    void slotReceivce(QVariant data, Reply *reply);
    /**
     * @brief slotWidgetHideInitState   窗口隐藏时状态初始化
     */
    void slotWidgetHideInitState();
private:
    static ScheduleManageTask *m_scheduleManageTask;
    /**
     * @brief connectHideEventToInitState       窗口隐藏绑定状态初始化
     * @param reply         回复
     */
    void connectHideEventToInitState(Reply reply);
private:
    CSchedulesDBus *m_dbus {nullptr};
    QMap<QString, scheduleBaseTask *> m_scheduleTaskMap;
    Reply m_Reply;
    scheduleBaseTask *m_preScheduleTask {nullptr};
};

#endif // SCHEDULEMANAGETASK_H
