// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QFileDevice>

const QString UPLOADTASK_SERVICE = "uploadNetWorkAccountData_calendar.service";
const QString UPLOADTASK_TIMER = "uploadNetWorkAccountData_calendar.timer";

CSystemdTimerControl::CSystemdTimerControl(QObject *parent)
    : QObject(parent)
{
    qCDebug(ServiceLogger) << "Creating CSystemdTimerControl";
    createPath();
}

CSystemdTimerControl::~CSystemdTimerControl()
{
    qCDebug(ServiceLogger) << "Destroying CSystemdTimerControl";
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
        // 构建提醒命令
        QString remindCMD = QString("dbus-send --session --print-reply --dest=com.deepin.dataserver.Calendar "
                                    "/com/deepin/dataserver/Calendar/AccountManager "
                                    "com.deepin.dataserver.Calendar.AccountManager.remindJob string:%1 string:%2")
                            .arg(info.accountID)
                            .arg(info.alarmID);

        // 使用模板方法创建服务和定时器
        createServiceFromTemplate(fileNameList.last(), remindCMD);
        createTimerFromTemplate(fileNameList.last(), info.triggerTimer);
    }
    // 使用新的QProcess方式
    execSystemdCommand(QStringList() << "daemon-reload");
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
    if (timerName.isEmpty()) {
        qCDebug(ServiceLogger) << "No timers to start";
        return;
    }

    // 构建完整的单元名称列表
    QStringList stopUnits;
    QStringList startUnits;
    foreach (auto str, timerName) {
        stopUnits << QString("%1.timer").arg(str);
        startUnits << QString("%1.timer").arg(str);
    }

    // 批量停止所有定时器（一条命令）
    execSystemdCommand(QStringList() << "stop" << stopUnits);

    // 批量启动所有定时器（一条命令）
    execSystemdCommand(QStringList() << "start" << startUnits);
}

void CSystemdTimerControl::stopSystemdTimer(const QStringList &timerName)
{
    qCInfo(ServiceLogger) << "Stopping systemd timers. TimerNames:" << timerName.join(",");
    if (timerName.isEmpty()) {
        qCDebug(ServiceLogger) << "No timers to stop";
        return;
    }

    // 构建完整的单元名称列表
    QStringList stopUnits;
    foreach (auto str, timerName) {
        stopUnits << QString("%1.timer").arg(str);
    }

    // 批量停止所有定时器（一条命令）
    execSystemdCommand(QStringList() << "stop" << stopUnits);
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
    qCInfo(ServiceLogger) << "Stopping all remind systemd timer for accountID:" << accountID;
    // 注意：这里使用通配符停止所有匹配的定时器
    execSystemdCommand(QStringList() << "stop" << QString("calendar-remind-%1-*.timer").arg(accountID.mid(0,8)));
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
    qCInfo(ServiceLogger) << "Starting calendar service systemd timer";
    // 清理玲珑包的残留
    QFile(m_systemdPath + "com.dde.calendarserver.calendar.service").remove();
    QFile(m_systemdPath + "com.dde.calendarserver.calendar.timer").remove();
    QFileInfo fileInfo(m_systemdPath + "timers.target.wants/com.dde.calendarserver.calendar.timer");
    //如果没有设置定时任务则开启定时任务
    if (!fileInfo.exists()) {
        execSystemdCommand(QStringList() << "enable" << "com.dde.calendarserver.calendar.timer");
        execSystemdCommand(QStringList() << "start" << "com.dde.calendarserver.calendar.timer");
    }
}

void CSystemdTimerControl::startDownloadTask(const QString &accountID, const int minute)
{
    qCInfo(ServiceLogger) << "Starting download task for accountID:" << accountID << "Minute:" << minute;
    {
        //.service - 使用模板创建
        QString remindCMD = QString("dbus-send --session --print-reply --dest=com.deepin.dataserver.Calendar "
                                    "/com/deepin/dataserver/Calendar/AccountManager "
                                    "com.deepin.dataserver.Calendar.AccountManager.downloadByAccountID string:%1 ")
                            .arg(accountID);
        QString serviceName = accountID + "_calendar";
        createServiceFromTemplate(serviceName, remindCMD);
    }

    {
        //timer - 使用周期性定时器
        QString timerName = accountID + "_calendar";
        createPeriodicTimerFromTemplate(timerName, minute);

        const QString accountTimer = accountID + "_calendar.timer";
        execSystemdCommand(QStringList() << "enable" << accountTimer);
        execSystemdCommand(QStringList() << "start" << accountTimer);
    }
}

void CSystemdTimerControl::stopDownloadTask(const QString &accountID)
{
    qCInfo(ServiceLogger) << "Stopping download task for accountID:" << accountID;
    QString timerName = accountID + "_calendar.timer";
    execSystemdCommand(QStringList() << "stop" << timerName);
    QFile::remove(m_systemdPath + accountID + "_calendar.timer");
    QFile::remove(m_systemdPath + accountID + "_calendar.service");
}

void CSystemdTimerControl::startUploadTask(const int minute)
{
    qCInfo(ServiceLogger) << "Starting upload task. Minute:" << minute;
    {
        //如果定时器为激活状态则退出
        QStringList checkArgs;
        checkArgs << "is-active" << UPLOADTASK_SERVICE;
        QString isActive = execSystemdCommand(checkArgs);
        if (isActive.trimmed() == "active") {
            qCInfo(ServiceLogger) << "Upload task is already active, skipping start.";
            return;
        }
    }
    {
        //.service - 使用模板创建
        QString remindCMD = QString("dbus-send --session --print-reply --dest=com.deepin.dataserver.Calendar "
                                    "/com/deepin/dataserver/Calendar/AccountManager "
                                    "com.deepin.dataserver.Calendar.AccountManager.uploadNetWorkAccountData ");
        QString serviceName = UPLOADTASK_SERVICE.chopped(8); // 移除 ".service" 后缀
        createServiceFromTemplate(serviceName, remindCMD);
    }

    {
        //timer - 使用周期性定时器
        QString timerName = UPLOADTASK_TIMER.chopped(6); // 移除 ".timer" 后缀
        createPeriodicTimerFromTemplate(timerName, minute);

        qCInfo(ServiceLogger) << "Enabling and starting upload timer:" << UPLOADTASK_TIMER;
        execSystemdCommand(QStringList() << "enable" << UPLOADTASK_TIMER);
        execSystemdCommand(QStringList() << "start" << UPLOADTASK_TIMER);
    }
}

void CSystemdTimerControl::stopUploadTask()
{
    qCInfo(ServiceLogger) << "Stopping upload task";
    execSystemdCommand(QStringList() << "stop" << UPLOADTASK_TIMER);
    
    QFile::remove(m_systemdPath + UPLOADTASK_TIMER);
    QFile::remove(m_systemdPath + UPLOADTASK_SERVICE);
}

void CSystemdTimerControl::createPath()
{
    qCInfo(ServiceLogger) << "Creating systemd path";
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

QString CSystemdTimerControl::execDirectCommand(const QString &program, const QStringList &args)
{
    qCDebug(ServiceLogger) << "Executing command:" << program << "with args:" << args;
    QProcess process;

    // 直接启动程序，不通过 shell
    process.start(program, args);

    // 设置超时时间（30秒）
    if (!process.waitForStarted(5000)) {
        qCWarning(ServiceLogger) << "Failed to start process:" << program << "Error:" << process.errorString();
        return QString();
    }

    if (!process.waitForFinished(30000)) {
        qCWarning(ServiceLogger) << "Process timeout:" << program;
        process.terminate();
        process.waitForFinished(1000);
        return QString();
    }

    QString strResult = QString::fromUtf8(process.readAllStandardOutput());
    qCDebug(ServiceLogger) << "Command output:" << strResult;
    return strResult;
}

QString CSystemdTimerControl::execSystemdCommand(const QStringList &args)
{
    // 使用 systemctl --user 模式
    QStringList fullArgs;
    fullArgs << "--user" << args;
    return execDirectCommand("systemctl", fullArgs);
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
QString CSystemdTimerControl::loadTemplateFile(const QString &templatePath)
{
    qCDebug(ServiceLogger) << "Loading template file from resources:" << templatePath;
    QFile file(":/" + templatePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(ServiceLogger) << "Failed to open template file:" << templatePath;
        return QString();
    }
    
    QString content = QString::fromLatin1(file.readAll());
    file.close();
    qCDebug(ServiceLogger) << "Template loaded successfully, size:" << content.size();
    return content;
}

void CSystemdTimerControl::createServiceFromTemplate(const QString &name, const QString &command)
{
    qCInfo(ServiceLogger) << "Creating service from template for name:" << name;
    QString fileName = m_systemdPath + name + ".service";
    
    // Load template and replace placeholder
    QString templateContent = loadTemplateFile("systemd-service-template.service");
    if (templateContent.isEmpty()) {
        qCWarning(ServiceLogger) << "Template is empty, using fallback";
        templateContent = "[Unit]\nDescription=Calendar Schedule Task\n[Service]\nExecStart=@COMMAND@\n";
    }
    
    // Replace @COMMAND@ placeholder
    QString content = templateContent.replace("@COMMAND@", command);
    qCDebug(ServiceLogger) << "Creating service file:" << fileName << "with command:" << command;
    createFile(fileName, content);
}

void CSystemdTimerControl::createTimerFromTemplate(const QString &name, const QDateTime &triggerTimer)
{
    qCInfo(ServiceLogger) << "Creating timer from template for name:" << name << "TriggerTimer:" << triggerTimer.toString("yyyy-MM-dd hh:mm:ss");
    QString fileName = m_systemdPath + name + ".timer";
    
    // Load template and replace placeholder
    QString templateContent = loadTemplateFile("systemd-timer-template.timer");
    if (templateContent.isEmpty()) {
        qCWarning(ServiceLogger) << "Template is empty, using fallback";
        templateContent = "[Unit]\nDescription=Calendar Schedule Timer\n[Timer]\nAccuracySec=1ms\nRandomizedDelaySec=0\nOnCalendar=@SCHEDULE@\n[Install]\nWantedBy=timers.target\n";
    }
    
    // Replace @SCHEDULE@ placeholder
    QString schedule = triggerTimer.toString("yyyy-MM-dd hh:mm:ss");
    QString content = templateContent.replace("@SCHEDULE@", schedule);
    qCDebug(ServiceLogger) << "Creating timer file:" << fileName << "with schedule:" << schedule;
    createFile(fileName, content);
}

void CSystemdTimerControl::createPeriodicTimerFromTemplate(const QString &name, int intervalMinutes)
{
    qCInfo(ServiceLogger) << "Creating periodic timer from template for name:" << name << "Interval:" << intervalMinutes << "minutes";
    QString fileName = m_systemdPath + name + ".timer";
    
    // Create periodic timer content (based on old implementation)
    QString content;
    content += "[Unit]\n";
    content += "Description=Calendar Periodic Task\n";
    content += "[Timer]\n";
    content += "OnActiveSec=1s\n";
    content += QString("OnUnitInactiveSec=%1m\n").arg(intervalMinutes);
    content += "AccuracySec=1us\n";
    content += "RandomizedDelaySec=0\n";
    content += "[Install]\n";
    content += "WantedBy=timers.target\n";
    
    qCDebug(ServiceLogger) << "Creating periodic timer file:" << fileName << "with interval:" << intervalMinutes << "minutes";
    createFile(fileName, content);
}
