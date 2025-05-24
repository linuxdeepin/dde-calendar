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

Q_LOGGING_CATEGORY(SystemdTimerLog, "calendar.systemdtimer")

const QString UPLOADTASK_SERVICE = "uploadNetWorkAccountData_calendar.service";
const QString UPLOADTASK_TIMER = "uploadNetWorkAccountData_calendar.timer";

CSystemdTimerControl::CSystemdTimerControl(QObject *parent)
    : QObject(parent)
{
    qCDebug(SystemdTimerLog) << "Initializing systemd timer control";
    createPath();
}

CSystemdTimerControl::~CSystemdTimerControl()
{
    qCDebug(SystemdTimerLog) << "Destroying systemd timer control";
}

void CSystemdTimerControl::buildingConfiggure(const QVector<SystemDInfo> &infoVector)
{
    qCDebug(SystemdTimerLog) << "Building configuration for" << infoVector.size() << "timers";
    if (infoVector.size() == 0) {
        qCWarning(SystemdTimerLog) << "Empty info vector, skipping configuration";
        return;
    }
    QStringList fileNameList{};
    foreach (auto info, infoVector) {
        fileNameList.append(QString("calendar-remind-%1-%2-%3").arg(info.accountID.mid(0,8)).arg(info.alarmID.mid(0, 8)).arg(info.laterCount));
        createService(fileNameList.last(), info);
        createTimer(fileNameList.last(), info.triggerTimer);
    }
    execLinuxCommand("systemctl --user daemon-reload");
    qCDebug(SystemdTimerLog) << "Starting systemd timers:" << fileNameList;
    startSystemdTimer(fileNameList);
}

void CSystemdTimerControl::stopSystemdTimerByJobInfos(const QVector<SystemDInfo> &infoVector)
{
    qCDebug(SystemdTimerLog) << "Stopping systemd timers for" << infoVector.size() << "jobs";
    QStringList fileNameList;
    foreach (auto info, infoVector) {
        fileNameList.append(QString("calendar-remind-%1-%2-%3").arg(info.accountID.mid(0,8)).arg(info.alarmID.mid(0, 8)).arg(info.laterCount));
    }
    stopSystemdTimer(fileNameList);
}

void CSystemdTimerControl::stopSystemdTimerByJobInfo(const SystemDInfo &info)
{
    qCDebug(SystemdTimerLog) << "Stopping systemd timer for job" << info.alarmID;
    QStringList fileName;
    //停止刚刚提醒的稍后提醒，所以需要对提醒次数减一
    fileName << QString("calendar-remind-%1-%2-%3").arg(info.accountID.mid(0,8)).arg(info.alarmID).arg(info.laterCount - 1);
    stopSystemdTimer(fileName);
}

void CSystemdTimerControl::startSystemdTimer(const QStringList &timerName)
{
    qCDebug(SystemdTimerLog) << "Starting systemd timers:" << timerName;
    QString command_stop("systemctl --user stop ");
    foreach (auto str, timerName) {
        command_stop += QString(" %1.timer").arg(str);
    }
    execLinuxCommand(command_stop);

    QString command("systemctl --user start ");
    foreach (auto str, timerName) {
        command += QString(" %1.timer").arg(str);
    }
    QString result = execLinuxCommand(command);
    if (!result.isEmpty()) {
        qCWarning(SystemdTimerLog) << "Command output:" << result;
    }
}

void CSystemdTimerControl::stopSystemdTimer(const QStringList &timerName)
{
    qCDebug(SystemdTimerLog) << "Stopping systemd timers:" << timerName;
    QString command("systemctl --user stop ");
    foreach (auto str, timerName) {
        command += QString(" %1.timer").arg(str);
    }
    QString result = execLinuxCommand(command);
    if (!result.isEmpty()) {
        qCWarning(SystemdTimerLog) << "Command output:" << result;
    }
}

void CSystemdTimerControl::removeFile(const QStringList &fileName)
{
    qCDebug(SystemdTimerLog) << "Removing files:" << fileName;
    foreach (auto f, fileName) {
        if (!QFile::remove(f)) {
            qCWarning(SystemdTimerLog) << "Failed to remove file:" << f;
        }
    }
}

void CSystemdTimerControl::stopAllRemindSystemdTimer(const QString &accountID)
{
    qCDebug(SystemdTimerLog) << "Stopping all remind timers for account:" << accountID;
    execLinuxCommand(QString("systemctl --user stop calendar-remind-%1-*.timer").arg(accountID.mid(0,8)));
}

void CSystemdTimerControl::removeRemindFile(const QString &accountID)
{
    qCDebug(SystemdTimerLog) << "Removing remind files for account:" << accountID;
    QDir dir(m_systemdPath);
    if (dir.exists()) {
        QStringList filters;
        filters << QString("calendar-remind-%1*").arg(accountID.mid(0,8));
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        dir.setNameFilters(filters);
        for (uint i = 0; i < dir.count(); ++i) {
            QFile::remove(m_systemdPath + dir[i]);
        }
        qCDebug(SystemdTimerLog) << "Removed" << dir.count() << "files";
    }
}

void CSystemdTimerControl::startCalendarServiceSystemdTimer()
{
    qCDebug(SystemdTimerLog) << "Starting calendar service systemd timer";
    // 清理玲珑包的残留
    QFile(m_systemdPath + "com.dde.calendarserver.calendar.service").remove();
    QFile(m_systemdPath + "com.dde.calendarserver.calendar.timer").remove();
    QFileInfo fileInfo(m_systemdPath + "timers.target.wants/com.dde.calendarserver.calendar.timer");
    //如果没有设置定时任务则开启定时任务
    if (!fileInfo.exists()) {
        qCInfo(SystemdTimerLog) << "Timer not found, enabling and starting calendar service timer";
        execLinuxCommand("systemctl --user enable com.dde.calendarserver.calendar.timer");
        execLinuxCommand("systemctl --user start com.dde.calendarserver.calendar.timer");
    }
}

void CSystemdTimerControl::startDownloadTask(const QString &accountID, const int minute)
{
    qCDebug(SystemdTimerLog) << "Starting download task for account:" << accountID << "interval:" << minute << "minutes";
    {
        //.service
        QString fileName;
        QString remindCMD = QString("dbus-send --session --print-reply --dest=com.deepin.dataserver.Calendar "
                                    "/com/deepin/dataserver/Calendar/AccountManager "
                                    "com.deepin.dataserver.Calendar.AccountManager.downloadByAccountID string:%1 ")
                            .arg(accountID);
        fileName = m_systemdPath + accountID + "_calendar.service";
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
    qCInfo(SystemdTimerLog) << "Download task started successfully";
}

void CSystemdTimerControl::stopDownloadTask(const QString &accountID)
{
    qCDebug(SystemdTimerLog) << "Stopping download task for account:" << accountID;
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
    qCDebug(SystemdTimerLog) << "Starting upload task with interval:" << minute << "minutes";
    {
        //如果定时器为激活状态则退出
        QString cmd = "systemctl --user is-active " + UPLOADTASK_SERVICE;
        QString isActive = execLinuxCommand(cmd);
        if (isActive == "active") {
            qCInfo(SystemdTimerLog) << "Upload task already active, skipping";
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
        QString content;
        content += "[Unit]\n";
        content += "Description = schedule uploadNetWorkAccountData task.\n";
        content += "[Timer]\n";
        content += "OnActiveSec = 1s\n";
        content += QString("OnUnitInactiveSec = %1m\n").arg(minute);
        content += "AccuracySec = 1us\n";
        content += "RandomizedDelaySec = 0\n";
        createFile(fileName, content);

        execLinuxCommand("systemctl --user enable " + UPLOADTASK_TIMER);
        execLinuxCommand("systemctl --user start " + UPLOADTASK_TIMER);
    }
    qCInfo(SystemdTimerLog) << "Upload task started successfully";
}

void CSystemdTimerControl::stopUploadTask()
{
    qCDebug(SystemdTimerLog) << "Stopping upload task";
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
    qCDebug(SystemdTimerLog) << "Creating systemd path";
    m_systemdPath = getHomeConfigPath().append("/systemd/user/");
    // 如果位于玲珑环境, 更改systemd path路径
    QString linglongAppID = qgetenv("LINGLONG_APPID");
    if (!linglongAppID.isEmpty()) {
        m_systemdPath = "/run/host/rootfs" + m_systemdPath;
        qCInfo(SystemdTimerLog) << "In Linglong environment, change the systemd path to " << m_systemdPath;
    }
    QDir dir;
    // 如果该路径不存在，则创建该文件夹
    if (!dir.exists(m_systemdPath)) {
        qCInfo(SystemdTimerLog) << "Creating directory:" << m_systemdPath;
        dir.mkpath(m_systemdPath);
    }
}

QString CSystemdTimerControl::execLinuxCommand(const QString &command)
{
    qCDebug(SystemdTimerLog) << "Executing command:" << command;
    QProcess process;
    process.start("/bin/bash", QStringList() << "-c" << command);
    process.waitForFinished();
    QString strResult = process.readAllStandardOutput();
    if (!strResult.isEmpty()) {
        qCDebug(SystemdTimerLog) << "Command output:" << strResult;
    }
    return strResult;
}

void CSystemdTimerControl::createService(const QString &name, const SystemDInfo &info)
{
    qCDebug(SystemdTimerLog) << "Creating service for:" << name << "account:" << info.accountID;
    QString fileName;
    QString remindCMD = QString("dbus-send --session --print-reply --dest=com.deepin.dataserver.Calendar "
                                "/com/deepin/dataserver/Calendar/AccountManager "
                                "com.deepin.dataserver.Calendar.AccountManager.remindJob string:%1 string:%2")
                        .arg(info.accountID)
                        .arg(info.alarmID);
    fileName = m_systemdPath + name + ".service";
    QString content;
    content += "[Unit]\n";
    content += "Description = schedule reminder task.\n";
    content += "[Service]\n";
    content += QString("ExecStart = /bin/bash -c \"%1\"\n").arg(remindCMD);
    createFile(fileName, content);
}

void CSystemdTimerControl::createTimer(const QString &name, const QDateTime &triggerTimer)
{
    qCDebug(SystemdTimerLog) << "Creating timer:" << name << "trigger time:" << triggerTimer;
    QString fileName;
    fileName = m_systemdPath + name + ".timer";
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
    qCDebug(SystemdTimerLog) << "Creating file:" << fileName;
    QFile file;
    file.setFileName(fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qCWarning(SystemdTimerLog) << "Failed to open file for writing:" << fileName;
        return;
    }
    if (file.write(content.toLatin1()) == -1) {
        qCWarning(SystemdTimerLog) << "Failed to write content to file:" << fileName;
    }
    file.close();
}
