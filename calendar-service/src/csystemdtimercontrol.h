// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CSYSTEMDTIMERCONTROL_H
#define CSYSTEMDTIMERCONTROL_H

#include <QObject>
#include <QDateTime>
#include <QVector>

struct SystemDInfo {
    QString accountID = ""; //帐户ID
    QString alarmID = ""; //提醒编号id
    qint64      laterCount = 0;         //稍后提醒次数
    QDateTime   triggerTimer;           //触发时间
};

/**
 * @brief The CSystemdTimerControl class
 * systemd timer 控制类
 */
class CSystemdTimerControl : public QObject
{
    Q_OBJECT
public:
    explicit CSystemdTimerControl(QObject *parent = nullptr);
    ~CSystemdTimerControl();

    /**
     * @brief buildingConfiggure       根据日程信息集，创建相关的systemd配置文件,并开启定时任务
     * @param infoVector
     * @param removeStale               是否清理帐户下不在infoVector中的旧提醒任务文件
     *                                  (仅在全量刷新提醒时使用，增量添加任务时必须为false)
     * @return                          配置是否有变化（无变化时不执行daemon-reload，避免打扰其它定时器）
     */
    bool buildingConfiggure(const QVector<SystemDInfo> &infoVector, bool removeStale = false);

    /**
     * @brief stopSystemdTimerByJobInfos        根据日程信息集，停止相应的任务
     * @param infoVector
     */
    void stopSystemdTimerByJobInfos(const QVector<SystemDInfo> &infoVector);

    /**
     * @brief stopSystemdTimerByJobInfo     根据日程信息，停止相关任务
     * @param info
     */
    void stopSystemdTimerByJobInfo(const SystemDInfo &info);
    /**
     * @brief startSystemdTimer        开启定时任务
     * @param timerName         定时任务.timer名称集合
     */
    void startSystemdTimer(const QStringList &timerName);
    /**
     * @brief stopSystemdTimer         停止定时任务
     * @param timerName         定时任务.timer名称集合
     */
    void stopSystemdTimer(const QStringList &timerName);

    /**
     * @brief stopAllRemindSystemdTimer 停止所有的日程定时任务
     */
    void stopAllRemindSystemdTimer(const QString &accountID);

    /**
     * @brief removeRemindFile      移除日程定时任务相关文件
     */
    void removeRemindFile(const QString &accountID);

    /**
     * @brief startCalendarServiceSystemdTimer      开启日程后端定时器
     */
    void startCalendarServiceSystemdTimer();

    //开启定时下载任务
    void startDownloadTask(const QString &accountID, const int minute);
    void stopDownloadTask(const QString &accountID);
    //开启定时上传任务
    void startUploadTask(const int minute);
    void stopUploadTask();

private:
    /**
     * @brief removeFile        移除.service和.timer文件
     * @param fileName
     */
    void removeFile(const QStringList &fileName);
    /**
     * @brief createPath
     * 创建systemd文件路径
     */
    void createPath();

    /**
     * @brief execLinuxCommand      执行linux命令
     * @param command
     * @return
     */
    QString execLinuxCommand(const QString &command);

    /**
     * @brief createService
     * 创建 .service文件
     * @return                  文件内容是否有变化
     */
    bool createService(const QString &name, const SystemDInfo &info);
    /**
     * @brief createTimer
     * 创建 .timer文件
     * @return                  文件内容是否有变化
     */
    bool createTimer(const QString &name, const QDateTime &triggerTimer);

    /**
     * @brief createFile        创建文件，内容未变化时不写入
     * @param fileName          文件名称
     * @param content           内容
     * @return                  文件内容是否有变化
     */
    bool createFile(const QString &fileName, const QString &content);

    /**
     * @brief removeStaleRemindFiles    清理帐户下已不需要的提醒任务文件(不在保留名单中的)
     * @param accountID                 帐户ID
     * @param keepFileNames             需要保留的文件名集合(不含后缀)
     * @return                          是否有文件被删除
     */
    bool removeStaleRemindFiles(const QString &accountID, const QStringList &keepFileNames);
signals:

public slots:
private:
    QString m_systemdPath;
};

#endif // CSYSTEMDTIMERCONTROL_H
