// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "jobremindmanager.h"
#include "src/utils.h"
#include "dbus/dbusuiopenschedule.h"
#include "dbus/dbusnotify.h"
#include "csystemdtimercontrol.h"

#include <QTimer>
#include <QDebug>

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

JobRemindManager::JobRemindManager(QObject *parent)
    : QObject(parent)
{
    m_dbusuiopen = new DbusUIOpenSchedule("com.deepin.Calendar",
                                          "/com/deepin/Calendar",
                                          QDBusConnection::sessionBus(),
                                          this);
    m_dbusnotify = new DBusNotify("com.deepin.dde.Notification",
                                  "/com/deepin/dde/Notification",
                                  QDBusConnection::sessionBus(),
                                  this);
    //若没开启定时任务则开启定时任务
    CSystemdTimerControl systemdTimer;
    systemdTimer.startCalendarServiceSystemdTimer();
}

/**
 * @brief  CallUiOpenSchedule 激活日历前端使之跳转到日视图并弹出编辑框
 * @param job 日程信息
 */
void JobRemindManager::CallUiOpenSchedule(const Job &job)
{
    QJsonObject obj = Utils::JobToObject(job);
    QJsonDocument doc;
    doc.setObject(obj);
    QString strjson(doc.toJson(QJsonDocument::Compact));
    m_dbusuiopen->OpenSchedule(strjson);
}

/**
 * @brief  RemindJob 日程提醒
 * 日程相关信息获取调用dbus服务弹出提醒窗口，并将返回id作为key,job作为value
 * 插入m_notifymap，等待后续提醒弹窗被点击后操作
 * @param job 日程信息
 */
void JobRemindManager::RemindJob(const Job &job)
{
    if (!job.Remind.isEmpty()) {
        int nDays = GetRemindAdvanceDays(job.Remind);
        if (-1 != nDays) {
            qint64 duration = 0;
            bool bmax = GetRemindLaterDuration(job.RemindLaterCount, duration);
            QStringList actionlist;
            QVariantMap hints;
            QString cmd = QString("dbus-send --session --print-reply --dest=com.deepin.dataserver.Calendar "
                                  "/com/deepin/dataserver/Calendar com.deepin.dataserver.Calendar.notifyMsgHanding int64:%1 int64:%2 ")
                              .arg(job.ID)
                              .arg(job.RecurID);
            auto argMake = [&](int operationNum, const QString &text, const QString &transText) {
                actionlist << text << transText;
                hints.insert("x-deepin-action-" + text, QString("bash,-c,%1 int32:%2").arg(cmd).arg(operationNum));
            };

            QDateTime tm = QDateTime::currentDateTime();
            if (tm < job.Start) {
                //如果提醒规则大于3天且是第二次提醒
                if (nDays >= 3 && job.RemindLaterCount == 1) {
                    //default对应的是默认操作，也就是在点击空白区域会出发的操作
                    argMake(1, notifyActKeyDefault, "");
                    argMake(5, notifyActKeyClose, tr("Close", "button"));
                    //当前时间与开始时间间隔大于1天
                    if (tm < job.Start.addDays(-1))
                        argMake(4, notifyActKeyRemind1DayBefore, tr("One day before start"));

                } else if ((nDays == 1 || nDays == 2) && bmax) {
                    argMake(1, notifyActKeyDefault, "");
                    argMake(5, notifyActKeyClose, tr("Close", "button"));
                    argMake(3, notifyActKeyRemindTomorrow, tr("Remind me tomorrow"));

                } else {
                    argMake(1,  notifyActKeyDefault,            "");
                    argMake(5,  notifyActKeyClose,              tr("Close", "button"));
                    argMake(2,  notifyActKeyRemindLater,        tr("Remind me later"));
                    //后面的actions会在拉列表中显示
                    argMake(21, notifyActKeyRemindAfter15mins,  tr("15 mins later"));
                    argMake(22, notifyActKeyRemindAfter1hour,   tr("1 hour later"));
                    argMake(23, notifyActKeyRemindAfter4hours,  tr("4 hours later"));
                    argMake(3,  notifyActKeyRemindTomorrow,     tr("Tomorrow"));
                }
            } else {
                argMake(1, notifyActKeyDefault, "");
                argMake(5, notifyActKeyClose, tr("Close", "button"));
            }

            QString title(tr("Schedule Reminder"));
            QString body = GetRemindBody(job, QDateTime::currentDateTime());
            QString appicon("dde-calendar");
            QString appname("dde-calendar");
            quint32 replaces_id = 0;
            qint32 timeout = 0;
            QList<QVariant> argumentList;
            argumentList << appname << replaces_id << appicon << title << body << actionlist << hints << timeout;
            qDebug() << __FUNCTION__ << QString("remind now: %1, title:"
                                                " %2, body: %3")
                     .arg(QDateTime::currentDateTime().toString())
                     .arg(title)
                     .arg(body);
            int notifyid = m_dbusnotify->Notify(argumentList);
            //将获取到的通知弹框id保存
            emit saveNotifyID(job, notifyid);
        } else {
            qDebug() << __FUNCTION__ << QString("remind job failed id=%1").arg(job.ID);
        }
    }
}

//通知提示框交互处理
void JobRemindManager::notifyMsgHanding(const Job &job, const int operationNum)
{
    switch (operationNum) {
    case 1:
        //打开日历
        CallUiOpenSchedule(job);
        break;
    case 2://稍后提醒
    case 21://15min后提醒
    case 22://一个小时后提醒
    case 23://四个小时后提醒
    case 3://明天提醒
    case 4://提前一天提醒
        RemindJobLater(job, operationNum);
        break;
    default:
        break;
    }
}

void JobRemindManager::closeNotification(quint32 notifyID)
{
    m_dbusnotify->closeNotification(notifyID);
}

/**
 * @brief  GetRemindAdvanceDays 根据提醒规则获取提前提醒天数
 * @param remind 提醒规则
 * @return 返回天数值
 */
int JobRemindManager::GetRemindAdvanceDays(const QString &remind)
{
    int nDays = 0, min = 0;
    QRegExp reg("\\d;\\d\\d:\\d\\d");
    // QRegExp reg("\\d;\\d+?:\\d+?");
    if (reg.exactMatch(remind)) {
        int hour;
        int ret = sscanf(remind.toStdString().c_str(), "%d;%d:%d", &nDays, &hour, &min);
        if (-1 == ret) {
            nDays = -1;
        }
    } else {
        bool bok = false;
        min = remind.toInt(&bok);
        if (bok) {
            nDays = int(float(min) / float(24 * 60));
        } else {
            nDays = -1;
        }
    }

    return nDays;
}

/**
 * @brief  GetRemindLaterDuration 根据提醒次数获取下一次提醒距现在的时间间隔
 * @param count 该日程已经提醒的次数
 * @param duration 下一次提醒距离现在的时间间隔单位毫秒
 * @return bool 是否超过一小时
 */
bool JobRemindManager::GetRemindLaterDuration(int count, qint64 &duration)
{
    bool bmax = false;
    duration = (10 + ((count - 1) * 5)) * Minute; //下一次提醒距离现在的时间间隔，单位毫秒
    if (duration >= Hour) {
        bmax = true;
        duration = Hour;
    }
    return bmax;
}

/**
 * @brief  GetRemindBody 获取日程提醒内容
 * @param job 日程信息结构体
 * @param tm  查询时间
 * @return QString 提醒内容
 */
QString JobRemindManager::GetRemindBody(const Job &job, const QDateTime &tm)
{
    QString msgStart;
    QString msgEnd;
    msgStart = GetBodyTimePart(tm, job.Start, job.AllDay, true);
    msgEnd = GetBodyTimePart(tm, job.End, job.AllDay, false);
    quint32 startdayofyear = static_cast<quint32>(job.Start.date().dayOfYear());
    quint32 enddayofyear = static_cast<quint32>(job.End.date().dayOfYear());
    QString prefix;
    if (job.AllDay) {
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
            msgEnd = job.End.time().toString("HH:mm");
        }
        //展示日程的开始结束时间
        prefix = QString(tr("%1 to %2")).arg(msgStart).arg(msgEnd);
    }
    //日程时间+title
    QString strBody = QString("%1 %2").arg(prefix).arg(job.Title);

    return strBody;
}

/**
 * @brief  RemindJobLater 稍后提醒
 * @param job 日程信息结构体
 */
void JobRemindManager::RemindJobLater(const Job &job, const int operationNum)
{
    CSystemdTimerControl systemdTimerControl;
    SystemDInfo info;
    info.jobID = job.ID;
    //如果是稍后提醒则设置对应的重复次数
    if (operationNum == 2) {
        info.laterCount = job.RemindLaterCount;
    } else {
        //如果不是稍后提醒，因为次数没有增加所以停止任务的时候需要加一以保证能够停止上次的任务
        info.laterCount = job.RemindLaterCount + 1;
    }
    info.triggerTimer = job.RemidTime;
    info.recurID = job.RecurID;
    //停止相应的任务
    systemdTimerControl.stopSystemdTimerByJobInfo(info);

    if (operationNum != 2) {
        //如果不是稍后提醒，还原成原来的提醒次数
        info.laterCount--;
    }
    QVector<SystemDInfo> infoVector;
    infoVector.append(info);
    //开启新任务
    systemdTimerControl.buildingConfiggure(infoVector);
}

QString JobRemindManager::GetBodyTimePart(const QDateTime &nowtime, const QDateTime &jobtime, bool allday, bool isstart)
{
    Q_UNUSED(isstart);
    //ToDo 需确认规则，需根据isstart确认是否为开始时间单独处理
    QString strmsg;
    qint64 diff = nowtime.daysTo(jobtime); //jobtime只可能大于等于当前remind任务执行的当前时间
    if (allday) {
        //全天日程，只展示日期，即date
        //日程开始时间距离现在超过两天
        strmsg.append(jobtime.date().toString(Qt::LocalDate));
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

/**
 * @brief  UpdateRemindJobs 提醒日程更新槽
 * @param jobs 待提醒日程集合
 */
void JobRemindManager::UpdateRemindJobs(const QList<Job> &jobs)
{
    CSystemdTimerControl systemdTimerControl;
    //清空日程提醒
    systemdTimerControl.stopAllRemindSystemdTimer();
    systemdTimerControl.removeRemindFile();

    QVector<SystemDInfo> infoVector{};
    foreach (auto job, jobs) {
        SystemDInfo info;
        info.jobID = job.ID;
        info.laterCount = job.RemindLaterCount;
        info.triggerTimer = job.RemidTime;
        info.recurID = job.RecurID;
        infoVector.append(info);
    }
    systemdTimerControl.buildingConfiggure(infoVector);
}

//Job被改变删除稍后提醒队列里对应id的Job
void JobRemindManager::NotifyJobsChanged(const QList<Job> &jobs)
{
    if (jobs.size() == 0)
        return;
    CSystemdTimerControl systemdTimerControl;

    QVector<SystemDInfo> infoVector{};
    foreach (auto job, jobs) {
        SystemDInfo info;
        info.jobID = job.ID;
        info.laterCount = job.RemindLaterCount;
        info.triggerTimer = job.RemidTime;
        infoVector.append(info);
    }
    systemdTimerControl.stopSystemdTimerByJobInfos(infoVector);
}
