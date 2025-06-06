// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "createschedulewidget.h"

#include "buttonwidget.h"
#include "../globaldef.h"
#include "dscheduledatamanager.h"
#include "commondef.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QProcess>
#include <QPushButton>
#include <QDBusMessage>
#include <QDBusConnection>

createSchedulewidget::createSchedulewidget(QWidget *parent)
    : IconDFrame(parent)
    , m_scheduleDtailInfo(new DSchedule)
    , m_scheduleitemwidget(new scheduleitemwidget(this))
{
    connect(m_scheduleitemwidget, &scheduleitemwidget::signalItemPress, this, &createSchedulewidget::slotItemPress);
}

DSchedule::Ptr createSchedulewidget::getScheduleDtailInfo()
{
    return m_scheduleDtailInfo;
}

void createSchedulewidget::setTitleName(const QString &titleName)
{
    m_titleName = titleName;
}

void createSchedulewidget::setDateTime(QDateTime begintime, QDateTime endtime)
{
    qCDebug(CommonLogger) << "Setting schedule time range - Begin:" << begintime << "End:" << endtime;
    m_BeginDateTime = begintime;
    m_EndDateTime = endtime;
}

void createSchedulewidget::setRpeat(int rpeat)
{
    m_rpeat = rpeat;
}

void createSchedulewidget::setschedule()
{
    qCDebug(CommonLogger) << "Creating schedule with title:" << m_titleName;
    QDateTime m_beginTime;
    QDateTime m_endTime;
    m_beginTime = m_BeginDateTime;
    m_endTime = m_EndDateTime;

    m_scheduleDtailInfo->setDtStart(m_beginTime);
    m_scheduleDtailInfo->setDtEnd(m_endTime);
    m_scheduleDtailInfo->setSummary(m_titleName);
    m_scheduleDtailInfo->setScheduleTypeID("403bf009-2005-4679-9c76-e73d9f83a8b4");
    m_scheduleDtailInfo->setAllDay(false);

    m_scheduleDtailInfo->setAlarmType(DSchedule::Alarm_Begin);
    switch (m_rpeat) {
    case 1:
        qCDebug(CommonLogger) << "Setting daily repeat rule";
        m_scheduleDtailInfo->setRRuleType(DSchedule::RRule_Day);
        break;
    case 2:
        qCDebug(CommonLogger) << "Setting workday repeat rule";
        m_scheduleDtailInfo->setRRuleType(DSchedule::RRule_Work);
        break;
    case 3:
        qCDebug(CommonLogger) << "Setting weekly repeat rule";
        m_scheduleDtailInfo->setRRuleType(DSchedule::RRule_Week);
        break;
    case 4:
        qCDebug(CommonLogger) << "Setting monthly repeat rule";
        m_scheduleDtailInfo->setRRuleType(DSchedule::RRule_Month);
        break;
    case 5:
        qCDebug(CommonLogger) << "Setting yearly repeat rule";
        m_scheduleDtailInfo->setRRuleType(DSchedule::RRule_Year);
        break;
    default:
        qCDebug(CommonLogger) << "Setting no repeat rule";
        m_scheduleDtailInfo->setRRuleType(DSchedule::RRule_None);
        break;
    }
    if (m_scheduleDtailInfo->getRRuleType() != DSchedule::RRule_None) {
        qCDebug(CommonLogger) << "Setting infinite repeat duration";
        //结束重复于类型为：永不
        m_scheduleDtailInfo->recurrence()->setDuration(-1);
    }
}

void createSchedulewidget::scheduleEmpty(bool isEmpty)
{
    m_scheduleEmpty = isEmpty;
}

void createSchedulewidget::updateUI(const QString &scheduleID)
{
    qCDebug(CommonLogger) << "Updating UI for schedule ID:" << scheduleID;
    if (m_scheduleEmpty) {
        qCDebug(CommonLogger) << "Schedule is empty, fetching from DBus";
        //获取筛选到的日程信息
        getCreatScheduleFromDbus(scheduleID);
        //如果筛选到的日程不为空，则展示日程插件
        if (!m_scheduleInfo.isEmpty()) {
            qCDebug(CommonLogger) << "Creating UI with" << m_scheduleInfo.size() << "schedules";
            QVBoxLayout *mainlayout = new QVBoxLayout();
            m_scheduleitemwidget->setScheduleDtailInfo(m_scheduleInfo);
            m_scheduleitemwidget->addscheduleitem();
            mainlayout->addWidget(m_scheduleitemwidget);
            setCenterLayout(mainlayout);
        } else {
            qCritical(CommonLogger) << "No matching schedule found in database for ID:" << scheduleID;
        }
    } else {
        qCDebug(CommonLogger) << "Creating UI with confirmation buttons";
        //是否创建日程
        QVBoxLayout *mainlayout = new QVBoxLayout();
        buttonwidget *button = new buttonwidget(this);
        button->addbutton(CANCEL_BUTTON_STRING, true, buttonwidget::ButtonRecommend);
        button->addbutton(CONFIRM_BUTTON_STRING);
        connect(button, &buttonwidget::buttonClicked, this, &createSchedulewidget::slotsbuttonchance);
        m_scheduleitemwidget->setScheduleDtailInfo(m_scheduleInfo);
        m_scheduleitemwidget->addscheduleitem();
        mainlayout->addWidget(m_scheduleitemwidget);
        mainlayout->addSpacing(4);
        mainlayout->addWidget(button);
        setCenterLayout(mainlayout);
    }
}

bool createSchedulewidget::buttonclicked()
{
    return m_buttonclicked;
}

void createSchedulewidget::slotsbuttonchance(int index, const QString &text)
{
    Q_UNUSED(text)
    if (index == 0) {
        m_buttonclicked = false;
    }
    if (index == 1) {
        m_buttonclicked = true;
    }
}

void createSchedulewidget::slotItemPress(const DSchedule::Ptr &info)
{
    qCDebug(CommonLogger) << "Schedule item pressed - ID:" << info->uid() << "Title:" << info->summary();
    QProcess proc;
    qCDebug(CommonLogger) << "Launching calendar program";
    proc.startDetached(PROCESS_OPEN_CALENDAR_PROGRAM, QString(PROCESS_OPEN_CALENDAR_ARGUMENTS).split(" "));
    QThread::msleep(750);
    QString schedulestr;
    DSchedule::toJsonString(info, schedulestr);
    qCDebug(CommonLogger) << "Sending OpenSchedule DBus message";
    QDBusMessage message = QDBusMessage::createMethodCall(DBUS_CALENDAR_SERVICE,
                                                          DBUS_CALENDAR_PATCH,
                                                          DBUS_CALENDAR_INTFACE,
                                                          "OpenSchedule");
    message << schedulestr;
    //发送消息
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
}

void createSchedulewidget::getCreatScheduleFromDbus(const QString &scheduleID)
{
    qCDebug(CommonLogger) << "Fetching schedule from DBus - ID:" << scheduleID;
    DSchedule::Ptr schedule = DScheduleDataManager::getInstance()->queryScheduleByScheduleID(scheduleID);
    m_scheduleInfo.append(schedule);
}
