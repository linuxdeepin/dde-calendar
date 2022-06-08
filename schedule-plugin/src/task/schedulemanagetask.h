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
#ifndef SCHEDULEMANAGETASK_H
#define SCHEDULEMANAGETASK_H

#include "../interface/reply.h"
#include "semanticanalysistask.h"
#include "../widget/icondframe.h"
#include "dschedule.h"
#include "../widget/createschedulewidget.h"
#include "schedulebasetask.h"
#include "accountitem.h"

#include <QObject>

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
    /**
     * @brief connectHideEventToInitState       窗口隐藏绑定状态初始化
     * @param reply         回复
     */
    void connectHideEventToInitState(Reply reply);
private:
    AccountItem::Ptr m_account;
    QMap<QString, scheduleBaseTask *> m_scheduleTaskMap;
    Reply m_Reply;
    scheduleBaseTask *m_preScheduleTask {nullptr};
};

#endif // SCHEDULEMANAGETASK_H
