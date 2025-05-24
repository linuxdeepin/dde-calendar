// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dalarmmanager.h"

#include "commondef.h"
#include "csystemdtimercontrol.h"
#include "dbus/dbusnotify.h"
#include <QLoggingCategory>

// Add logging category
Q_LOGGING_CATEGORY(alarmLog, "dde.calendar.alarm")

#define Millisecond 1
#define Second 1000 * Millisecond
#define Minute 60 * Second
#define Hour 60 * Minute

static QString notifyActKeyDefault("default");
static QString notifyActKeyClose("close");
static QString notifyActKeyRemindLater("later");
static QString notifyActKeyRemindAfter15mins("later-15mins");
static QString notifyActKeyRemindAfter1hour("later-1hour");
static QString notifyActKeyRemindAfter4hours("later-4hours");
static QString notifyActKeyRemind1DayBefore("one-day-before");
static QString notifyActKeyRemindTomorrow("tomorrow");
static QString layoutHM("15:04");

DAlarmManager::DAlarmManager(QObject *parent)
    : QObject(parent)
{
    qCDebug(alarmLog) << "Initializing DAlarmManager";
    m_dbusnotify = new DBusNotify("org.deepin.dde.Notification1",
                                  "/org/deepin/dde/Notification1",
                                  "org.deepin.dde.Notification1",
                                  QDBusConnection::sessionBus(),
                                  this);
    if (!m_dbusnotify->isValid()) {
        qCWarning(alarmLog) << "Failed to initialize DBusNotify with org.deepin.dde.Notification1, trying fallback";
        delete m_dbusnotify;
        m_dbusnotify = new DBusNotify("com.deepin.dde.Notification",
                                      "/com/deepin/dde/Notification",
                                      "com.deepin.dde.Notification",
                                      QDBusConnection::sessionBus(),
                                      this);
        if (!m_dbusnotify->isValid()) {
            qCCritical(alarmLog) << "Failed to initialize DBusNotify with fallback path";
        } else {
            qCInfo(alarmLog) << "Successfully initialized DBusNotify with fallback path";
        }
    } else {
        qCInfo(alarmLog) << "Successfully initialized DBusNotify";
    }

    //若没开启定时任务则开启定时任务
    CSystemdTimerControl systemdTimer;
    qCDebug(alarmLog) << "Starting calendar service systemd timer";
    systemdTimer.startCalendarServiceSystemdTimer();
}

void DAlarmManager::updateRemind(const DRemindData::List &remindList)
{
    qCDebug(alarmLog) << "Updating remind list, size:" << remindList.size();
    if (remindList.size() == 0) {
        qCDebug(alarmLog) << "Empty remind list, skipping update";
        return;
    }
    QString &&accountID = remindList.at(0)->accountID();
    qCDebug(alarmLog) << "Processing reminders for account:" << accountID;
    
    CSystemdTimerControl systemdTimerControl;
    qCDebug(alarmLog) << "Stopping all remind systemd timers for account:" << accountID;
    systemdTimerControl.stopAllRemindSystemdTimer(accountID);
    systemdTimerControl.removeRemindFile(accountID);

    QVector<SystemDInfo> infoVector {};
    foreach (auto remind, remindList) {
        SystemDInfo info;
        info.accountID = remind->accountID();
        info.alarmID = remind->alarmID();
        info.laterCount = remind->remindCount();
        info.triggerTimer = remind->dtRemind();
        infoVector.append(info);
    }
    qCDebug(alarmLog) << "Building configuration for" << infoVector.size() << "reminders";
    systemdTimerControl.buildingConfiggure(infoVector);
}

void DAlarmManager::notifyJobsChanged(const DRemindData::List &remindList)
{
    qCDebug(alarmLog) << "Notify jobs changed, list size:" << remindList.size();
    if (remindList.size() == 0) {
        qCDebug(alarmLog) << "Empty remind list, skipping notification";
        return;
    }
    CSystemdTimerControl systemdTimerControl;

    QVector<SystemDInfo> infoVector {};
    foreach (auto remind, remindList) {
        SystemDInfo info;
        info.accountID = remind->accountID();
        info.alarmID = remind->alarmID();
        info.laterCount = remind->remindCount();
        info.triggerTimer = remind->dtRemind();
        infoVector.append(info);
    }
    qCDebug(alarmLog) << "Stopping systemd timers for" << infoVector.size() << "jobs";
    systemdTimerControl.stopSystemdTimerByJobInfos(infoVector);
}

void DAlarmManager::notifyMsgHanding(const DRemindData::Ptr &remindData, const int operationNum)
{
    qCDebug(alarmLog) << "Handling notification message for alarm:" << remindData->alarmID() << "operation:" << operationNum;
    switch (operationNum) {
    case 1:
        //打开日历
        qCDebug(alarmLog) << "Opening calendar UI for alarm:" << remindData->alarmID();
        emit signalCallOpenCalendarUI(remindData->alarmID());
        break;
    case 2: //稍后提醒
    case 21: //15min后提醒
    case 22: //一个小时后提醒
    case 23: //四个小时后提醒
    case 3: //明天提醒
    case 4: //提前一天提醒
        qCDebug(alarmLog) << "Setting reminder for later, operation:" << operationNum;
        remindLater(remindData, operationNum);
        break;
    default:
        qCWarning(alarmLog) << "Unknown operation number:" << operationNum;
        break;
    }
}

void DAlarmManager::remindLater(const DRemindData::Ptr &remindData, const int operationNum)
{
    qCDebug(alarmLog) << "Setting reminder for later, alarm:" << remindData->alarmID() << "operation:" << operationNum;
    CSystemdTimerControl systemdTimerControl;
    SystemDInfo info;
    info.accountID = remindData->accountID();
    info.alarmID = remindData->alarmID();
    //如果是稍后提醒则设置对应的重复次数
    if (operationNum == 2) {
        info.laterCount = remindData->remindCount();
    } else {
        //如果不是稍后提醒，因为次数没有增加所以停止任务的时候需要加一以保证能够停止上次的任务
        info.laterCount = remindData->remindCount() + 1;
    }
    info.triggerTimer = remindData->dtRemind();
    //停止相应的任务
    qCDebug(alarmLog) << "Stopping systemd timer for job:" << info.alarmID;
    systemdTimerControl.stopSystemdTimerByJobInfo(info);

    if (operationNum != 2) {
        //如果不是稍后提醒，还原成原来的提醒次数
        info.laterCount--;
    }
    QVector<SystemDInfo> infoVector;
    infoVector.append(info);
    //开启新任务
    qCDebug(alarmLog) << "Building new configuration for reminder";
    systemdTimerControl.buildingConfiggure(infoVector);
}

int DAlarmManager::remindJob(const DRemindData::Ptr &remindData, const DSchedule::Ptr &schedule)
{
    //如果没有提醒
    if (schedule->alarms().size() == 0) {
        qCWarning(alarmLog) << "Remind job failed, no alarms found for schedule:" << schedule->uid();
        return 0;
    }

    KCalendarCore::Alarm::Ptr alarm = schedule->alarms().at(0);
    qCDebug(alarmLog) << "Processing remind job for alarm:" << remindData->alarmID();

    int nDays = -(alarm->startOffset().asDays());

    qint64 duration = 0;
    bool bmax = getRemindLaterDuration(remindData->remindCount(), duration);
    QStringList actionlist;
    QVariantMap hints;
    QString cmd = QString("dbus-send --session --print-reply --dest=com.deepin.dataserver.Calendar "
                          "/com/deepin/dataserver/Calendar/AccountManager "
                          "com.deepin.dataserver.Calendar.AccountManager.notifyMsgHanding string:%1 string:%2")
                  .arg(remindData->accountID())
                  .arg(remindData->alarmID());
    auto argMake = [&](int operationNum, const QString & text, const QString & transText) {
        actionlist << text << transText;
        hints.insert("x-deepin-action-" + text, QString("/bin/bash,-c,%1 int32:%2").arg(cmd).arg(operationNum));
    };

    QDateTime tm = QDateTime::currentDateTime();
    if (tm < schedule->dtStart()) {
        //如果提醒规则大于3天且是第二次提醒
        if (nDays >= 3 && remindData->remindCount() == 1) {
            qCDebug(alarmLog) << "Setting up notification for multi-day reminder";
            //default对应的是默认操作，也就是在点击空白区域会出发的操作
            argMake(1, notifyActKeyDefault, "");
            argMake(5, notifyActKeyClose, tr("Close", "button"));
            //当前时间与开始时间间隔大于1天
            if (tm < schedule->dtStart().addDays(-1))
                argMake(4, notifyActKeyRemind1DayBefore, tr("One day before start"));

        } else if ((nDays == 1 || nDays == 2) && bmax) {
            qCDebug(alarmLog) << "Setting up notification for 1-2 day reminder";
            argMake(1, notifyActKeyDefault, "");
            argMake(5, notifyActKeyClose, tr("Close", "button"));
            argMake(3, notifyActKeyRemindTomorrow, tr("Remind me tomorrow"));

        } else {
            qCDebug(alarmLog) << "Setting up notification with multiple remind options";
            argMake(1, notifyActKeyDefault, "");
            argMake(5, notifyActKeyClose, tr("Close", "button"));
            argMake(2, notifyActKeyRemindLater, tr("Remind me later"));
            //后面的actions会在拉列表中显示
            argMake(21, notifyActKeyRemindAfter15mins, tr("15 mins later"));
            argMake(22, notifyActKeyRemindAfter1hour, tr("1 hour later"));
            argMake(23, notifyActKeyRemindAfter4hours, tr("4 hours later"));
            argMake(3, notifyActKeyRemindTomorrow, tr("Tomorrow"));
        }
    } else {
        qCDebug(alarmLog) << "Setting up basic notification options";
        argMake(1, notifyActKeyDefault, "");
        argMake(5, notifyActKeyClose, tr("Close", "button"));
    }

    QString title(tr("Schedule Reminder"));
    QString body = getRemindBody(schedule);
    QString appicon("dde-calendar");
    QString appname("dde-calendar");
    quint32 replaces_id = 0;
    qint32 timeout = 0;
    QList<QVariant> argumentList;
    argumentList << appname << replaces_id << appicon << title << body << actionlist << hints << timeout;
    qCDebug(alarmLog) << "Sending notification for schedule:" << schedule->uid() 
                      << "at:" << QDateTime::currentDateTime().toString();
    int notifyid = m_dbusnotify->Notify(argumentList);
    qCDebug(alarmLog) << "Notification sent with ID:" << notifyid;
    return notifyid;
}

DBusNotify *DAlarmManager::getdbusnotify()
{
    return  m_dbusnotify;
}

QString DAlarmManager::getRemindBody(const DSchedule::Ptr &schedule)
{
    QDateTime tm = QDateTime::currentDateTime();
    QString msgStart;
    QString msgEnd;
    msgStart = getBodyTimePart(tm, schedule->dtStart(), schedule->allDay(), true);
    msgEnd = getBodyTimePart(tm, schedule->dtEnd(), schedule->allDay(), false);
    quint32 startdayofyear = static_cast<quint32>(schedule->dtStart().date().dayOfYear());
    quint32 enddayofyear = static_cast<quint32>(schedule->dtEnd().date().dayOfYear());
    QString prefix;
    if (schedule->allDay()) {
        //全天日程
        if (startdayofyear == enddayofyear) {
            //非跨天日程，只展示开始时间
            prefix = msgStart;
        } else {
            //跨天日程，展示整个日程的时间
            prefix = QString(tr("%1 to %2")).arg(msgStart).arg(msgEnd);
        }
    } else {
        //非全天日程
        if (startdayofyear == enddayofyear) {
            //非跨天日程，GetBodyTimePart已经返回了日程的日期，即date，所以，这里只需要日程的结束时间，即time
            msgEnd = schedule->dtEnd().time().toString("HH:mm");
        }
        //展示日程的开始结束时间
        prefix = QString(tr("%1 to %2")).arg(msgStart).arg(msgEnd);
    }
    //日程时间+title
    QString strBody = QString("%1 %2").arg(prefix).arg(schedule->summary());

    return strBody;
}

QString DAlarmManager::getBodyTimePart(const QDateTime &nowtime, const QDateTime &jobtime, bool allday, bool isstart)
{
    Q_UNUSED(isstart);
    //ToDo 需确认规则，需根据isstart确认是否为开始时间单独处理
    QString strmsg;
    qint64 diff = nowtime.daysTo(jobtime); //jobtime只可能大于等于当前remind任务执行的当前时间
    if (allday) {
        //全天日程，只展示日期，即date
        //日程开始时间距离现在超过两天
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        strmsg.append(jobtime.date().toString(QLocale().dateFormat(QLocale::ShortFormat)));
#else
        strmsg.append(jobtime.date().toString(Qt::LocalDate));
#endif
        if (diff == 0) {
            //日程开始时间是今天
            strmsg = tr("Today");
        } else if (diff == 1) {
            //日程开始时间是明天
            strmsg = tr("Tomorrow");
        }
    } else {
        //非全天日程，展示日期和时间，即date time
        //日程开始时间距离现在超过两天
        strmsg.append(QString(" %1").arg(jobtime.toString("yyyy/MM/dd HH:mm")));
        if (diff == 0) {
            //日程开始时间是今天，
            strmsg = tr("Today") + " " + jobtime.time().toString("HH:mm");
        } else if (diff == 1) {
            //日程开始时间是明天
            strmsg = tr("Tomorrow") + " " + jobtime.time().toString("HH:mm");
        }
    }

    return strmsg;
}

int DAlarmManager::getRemindLaterDuration(int count, qint64 &duration)
{
    bool bmax = false;
    duration = (10 + ((count - 1) * 5)) * Minute; //下一次提醒距离现在的时间间隔，单位毫秒
    if (duration >= Hour) {
        bmax = true;
        duration = Hour;
    }
    return bmax;
}
