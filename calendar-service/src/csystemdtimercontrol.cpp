// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "csystemdtimercontrol.h"

#include "commondef.h"
#include "units.h"
#include <QDebug>
#include <QLoggingCategory>

#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QFile>

const QString UPLOADTASK_SERVICE = "uploadNetWorkAccountData_calendar.service";
const QString UPLOADTASK_TIMER = "uploadNetWorkAccountData_calendar.timer";

CSystemdTimerControl::CSystemdTimerControl(QObject *parent)
    : QObject(parent)
{
    createPath();
}

CSystemdTimerControl::~CSystemdTimerControl()
{
}

void CSystemdTimerControl::buildingConfiggure(const QVector<SystemDInfo> &infoVector)
{
    qCInfo(ServiceLogger) << "Building systemd configuration for " << infoVector.size() << " info(s).";
    if (infoVector.size() == 0) {
        qCWarning(ServiceLogger) << "No systemd info provided, skipping build.";
        return;
    }
    QStringList fileNameList{};
    foreach (auto info, infoVector) {
        fileNameList.append(QString("calendar-remind-%1-%2-%3").arg(info.accountID.mid(0,8)).arg(info.alarmID.mid(0, 8)).arg(info.laterCount));
        createService(fileNameList.last(), info);
        createTimer(fileNameList.last(), info.triggerTimer);
    }
    execLinuxCommand("systemctl --user daemon-reload");
    startSystemdTimer(fileNameList);
}

void CSystemdTimerControl::stopSystemdTimerByJobInfos(const QVector<SystemDInfo> &infoVector)
{
    qCInfo(ServiceLogger) << "Stopping systemd timers for " << infoVector.size() << " job infos.";
    QStringList fileNameList;
    foreach (auto info, infoVector) {
        fileNameList.append(QString("calendar-remind-%1-%2-%3").arg(info.accountID.mid(0,8)).arg(info.alarmID.mid(0, 8)).arg(info.laterCount));
    }
    stopSystemdTimer(fileNameList);
}

void CSystemdTimerControl::stopSystemdTimerByJobInfo(const SystemDInfo &info)
{
    qCInfo(ServiceLogger) << "Stopping systemd timer for single job info. AccountID:" << info.accountID << "AlarmID:" << info.alarmID << "LaterCount:" << (info.laterCount - 1);
    QStringList fileName;
    //停止刚刚提醒的稍后提醒，所以需要对提醒次数减一
    QString name = QString("calendar-remind-%1-%2-%3").arg(info.accountID.mid(0,8)).arg(info.alarmID).arg(info.laterCount - 1);
    qCDebug(ServiceLogger) << "Stopping timer for single job info. Name:" << name;
    fileName << name;
    stopSystemdTimer(fileName);
}

void CSystemdTimerControl::startSystemdTimer(const QStringList &timerName)
{
    qCInfo(ServiceLogger) << "Starting systemd timers. TimerNames:" << timerName.join(",");
    QString command_stop("systemctl --user stop ");
    foreach (auto str, timerName) {
        command_stop += QString(" %1.timer").arg(str);
    }
    qCDebug(ServiceLogger) << "Stopping timers (if active) via command:" << command_stop;
    execLinuxCommand(command_stop);

    QString command("systemctl --user start ");
    foreach (auto str, timerName) {
        command += QString(" %1.timer").arg(str);
    }
    qCDebug(ServiceLogger) << "Starting timers via command:" << command;
    execLinuxCommand(command);
}

void CSystemdTimerControl::stopSystemdTimer(const QStringList &timerName)
{
    qCInfo(ServiceLogger) << "Stopping systemd timers. TimerNames:" << timerName.join(",");
    QString command("systemctl --user stop ");
    foreach (auto str, timerName) {
        command += QString(" %1.timer").arg(str);
    }
    qCDebug(ServiceLogger) << "Stopping timers via command:" << command;
    execLinuxCommand(command);
}

void CSystemdTimerControl::removeFile(const QStringList &fileName)
{
    qCInfo(ServiceLogger) << "Removing file(s):" << fileName.join(",");
    foreach (auto f, fileName) {
        qCDebug(ServiceLogger) << "Removing file:" << f;
        QFile::remove(f);
    }
}

void CSystemdTimerControl::stopAllRemindSystemdTimer(const QString &accountID)
{
    execLinuxCommand(QString("systemctl --user stop calendar-remind-%1-*.timer").arg(accountID.mid(0,8)));
}

void CSystemdTimerControl::removeRemindFile(const QString &accountID)
{
    qCInfo(ServiceLogger) << "Removing remind file(s) for accountID:" << accountID;
    QDir dir(m_systemdPath);
    if (dir.exists()) {
        QStringList filters;
        filters << QString("calendar-remind-%1*").arg(accountID.mid(0,8));
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        dir.setNameFilters(filters);
        qCDebug(ServiceLogger) << "Removing files matching filter:" << filters.join(",") << " in dir:" << m_systemdPath;
        for (uint i = 0; i < dir.count(); ++i) {
            qCDebug(ServiceLogger) << "Removing file:" << (m_systemdPath + dir[i]);
            QFile::remove(m_systemdPath + dir[i]);
        }
    } else {
        qCWarning(ServiceLogger) << "Directory does not exist:" << m_systemdPath;
    }
}

void CSystemdTimerControl::startCalendarServiceSystemdTimer()
{
    // 清理玲珑包的残留
    QFile(m_systemdPath + "com.dde.calendarserver.calendar.service").remove();
    QFile(m_systemdPath + "com.dde.calendarserver.calendar.timer").remove();
    QFileInfo fileInfo(m_systemdPath + "timers.target.wants/com.dde.calendarserver.calendar.timer");
    //如果没有设置定时任务则开启定时任务
    if (!fileInfo.exists()) {
        execLinuxCommand("systemctl --user enable com.dde.calendarserver.calendar.timer");
        execLinuxCommand("systemctl --user start com.dde.calendarserver.calendar.timer");
    }
}

void CSystemdTimerControl::startDownloadTask(const QString &accountID, const int minute)
{
    qCInfo(ServiceLogger) << "Starting download task for accountID:" << accountID << "Minute:" << minute;
    {
        //.service
        QString fileName;
        QString remindCMD = QString("dbus-send --session --print-reply --dest=com.deepin.dataserver.Calendar "
                                    "/com/deepin/dataserver/Calendar/AccountManager "
                                    "com.deepin.dataserver.Calendar.AccountManager.downloadByAccountID string:%1 ")
                            .arg(accountID);
        fileName = m_systemdPath + accountID + "_calendar.service";
        qCDebug(ServiceLogger) << "Creating download service file:" << fileName << "Command:" << remindCMD;
        QString content;
        content += "[Unit]\n";
        content += "Description = schedule download task.\n";
        content += "[Service]\n";
        content += QString("ExecStart = /bin/bash -c \"%1\"\n").arg(remindCMD);
        content += "[Install]\n";
        content += "WantedBy=user-session.target\n";
        createFile(fileName, content);
    }

    {
        //timer
        QString fileName;
        fileName = m_systemdPath + accountID + "_calendar.timer";
        qCDebug(ServiceLogger) << "Creating download timer file:" << fileName << "Minute:" << minute;
        QString content;
        content += "[Unit]\n";
        content += "Description = schedule download task.\n";
        content += "[Timer]\n";
        content += "OnActiveSec = 1s\n";
        content += QString("OnUnitInactiveSec = %1m\n").arg(minute);
        content += "AccuracySec = 1us\n";
        content += "RandomizedDelaySec = 0\n";
        content += "[Install]\n";
        content += "WantedBy = timers.target\n";
        createFile(fileName, content);

        const QString accountTimer = accountID + "_calendar.timer";
        execLinuxCommand("systemctl --user enable " + accountTimer);
        execLinuxCommand("systemctl --user start " + accountTimer);
    }
}

void CSystemdTimerControl::stopDownloadTask(const QString &accountID)
{
    QString fileName;
    fileName = m_systemdPath + accountID + "_calendar.timer";
    QString command("systemctl --user stop ");
    command += accountID + "_calendar.timer";
    execLinuxCommand(command);
    QFile::remove(fileName);
    QString fileServiceName = m_systemdPath + accountID + "_calendar.service";
    QFile::remove(fileServiceName);
}

void CSystemdTimerControl::startUploadTask(const int minute)
{
    qCInfo(ServiceLogger) << "Starting upload task. Minute:" << minute;
    {
        //如果定时器为激活状态则退出
        QString cmd = "systemctl --user is-active " + UPLOADTASK_SERVICE;
        qCDebug(ServiceLogger) << "Checking if upload task is active via command:" << cmd;
        QString isActive = execLinuxCommand(cmd);
        if (isActive == "active") {
            qCInfo(ServiceLogger) << "Upload task is already active, skipping start.";
            return;
        }
    }
    {
        //.service
        QString fileName;
        QString remindCMD = QString("dbus-send --session --print-reply --dest=com.deepin.dataserver.Calendar "
                                    "/com/deepin/dataserver/Calendar/AccountManager "
                                    "com.deepin.dataserver.Calendar.AccountManager.uploadNetWorkAccountData ");
        fileName = m_systemdPath + UPLOADTASK_SERVICE;
        qCDebug(ServiceLogger) << "Creating upload service file:" << fileName << "Command:" << remindCMD;
        QString content;
        content += "[Unit]\n";
        content += "Description = schedule uploadNetWorkAccountData task.\n";
        content += "[Service]\n";
        content += QString("ExecStart = /bin/bash -c \"%1\"\n").arg(remindCMD);
        createFile(fileName, content);
    }

    {
        //timer
        QString fileName;
        fileName = m_systemdPath + UPLOADTASK_TIMER;
        qCDebug(ServiceLogger) << "Creating upload timer file:" << fileName << "Minute:" << minute;
        QString content;
        content += "[Unit]\n";
        content += "Description = schedule uploadNetWorkAccountData task.\n";
        content += "[Timer]\n";
        content += "OnActiveSec = 1s\n";
        content += QString("OnUnitInactiveSec = %1m\n").arg(minute);
        content += "AccuracySec = 1us\n";
        content += "RandomizedDelaySec = 0\n";
        createFile(fileName, content);

        qCInfo(ServiceLogger) << "Enabling and starting upload timer:" << UPLOADTASK_TIMER;
        execLinuxCommand("systemctl --user enable " + UPLOADTASK_TIMER);
        execLinuxCommand("systemctl --user start " + UPLOADTASK_TIMER);
    }
}

void CSystemdTimerControl::stopUploadTask()
{
    QString fileName;
    fileName = m_systemdPath + UPLOADTASK_TIMER;
    QString command("systemctl --user stop ");
    command += UPLOADTASK_TIMER;
    execLinuxCommand(command);
    QFile::remove(fileName);
    QString fileServiceName = m_systemdPath + UPLOADTASK_SERVICE;
    QFile::remove(fileServiceName);
}

void CSystemdTimerControl::createPath()
{
    m_systemdPath = getHomeConfigPath().append("/systemd/user/");
    // 如果位于玲珑环境, 更改systemd path路径
    QString linglongAppID = qgetenv("LINGLONG_APPID");
    if (!linglongAppID.isEmpty()) {
        m_systemdPath = "/run/host/rootfs" + m_systemdPath;
        qCInfo(ServiceLogger) << "In Linglong environment, change the systemd path to " << m_systemdPath;
    }
    QDir dir;
    // 如果该路径不存在，则创建该文件夹
    if (!dir.exists(m_systemdPath)) {
        qCInfo(ServiceLogger) << "Creating systemd user directory:" << m_systemdPath;
        dir.mkpath(m_systemdPath);
    } else {
        qCInfo(ServiceLogger) << "Systemd user directory already exists:" << m_systemdPath;
    }
}

QString CSystemdTimerControl::execLinuxCommand(const QString &command)
{
    qCDebug(ServiceLogger) << "Executing linux command:" << command;
    QProcess process;
    process.start("/bin/bash", QStringList() << "-c" << command);
    process.waitForFinished();
    QString strResult = process.readAllStandardOutput();
    qCDebug(ServiceLogger) << "Command output:" << strResult;
    return strResult;
}

void CSystemdTimerControl::createService(const QString &name, const SystemDInfo &info)
{
    qCInfo(ServiceLogger) << "Creating service for name:" << name << "AccountID:" << info.accountID << "AlarmID:" << info.alarmID;
    QString fileName;
    QString remindCMD = QString("dbus-send --session --print-reply --dest=com.deepin.dataserver.Calendar "
                                "/com/deepin/dataserver/Calendar/AccountManager "
                                "com.deepin.dataserver.Calendar.AccountManager.remindJob string:%1 string:%2")
                        .arg(info.accountID)
                        .arg(info.alarmID);
    fileName = m_systemdPath + name + ".service";
    qCDebug(ServiceLogger) << "Creating service file:" << fileName << "Command:" << remindCMD;
    QString content;
    content += "[Unit]\n";
    content += "Description = schedule reminder task.\n";
    content += "[Service]\n";
    content += QString("ExecStart = /bin/bash -c \"%1\"\n").arg(remindCMD);
    createFile(fileName, content);
}

void CSystemdTimerControl::createTimer(const QString &name, const QDateTime &triggerTimer)
{
    qCInfo(ServiceLogger) << "Creating timer for name:" << name << "TriggerTimer:" << triggerTimer.toString("yyyy-MM-dd hh:mm:ss");
    QString fileName;
    fileName = m_systemdPath + name + ".timer";
    qCDebug(ServiceLogger) << "Creating timer file:" << fileName;
    QString content;
    content += "[Unit]\n";
    content += "Description = schedule reminder task.\n";
    content += "[Timer]\n";
    content += "AccuracySec = 1ms\n";
    content += "RandomizedDelaySec = 0\n";
    content += QString("OnCalendar = %1 \n").arg(triggerTimer.toString("yyyy-MM-dd hh:mm:ss"));
    createFile(fileName, content);
}

void CSystemdTimerControl::createFile(const QString &fileName, const QString &content)
{
    qCDebug(ServiceLogger) << "Creating file:" << fileName << "Content:" << content;
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    file.write(content.toLatin1());
    file.close();
}
