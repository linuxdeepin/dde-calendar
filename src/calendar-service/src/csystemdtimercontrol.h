// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
     */
    void buildingConfiggure(const QVector<SystemDInfo> &infoVector);

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
     * @brief loadTemplateFile   从Qt资源文件加载模板内容
     * @param templatePath       模板文件路径
     * @return                   模板内容
     */
    QString loadTemplateFile(const QString &templatePath);

    /**
     * @brief createServiceFromTemplate   从模板创建服务文件
     * @param name                         服务名称
     * @param command                      要执行的命令
     */
    void createServiceFromTemplate(const QString &name, const QString &command);

    /**
     * @brief createTimerFromTemplate   从模板创建定时器文件（一次性提醒）
     * @param name                      定时器名称
     * @param triggerTimer              触发时间
     */
    void createTimerFromTemplate(const QString &name, const QDateTime &triggerTimer);

    /**
     * @brief createPeriodicTimerFromTemplate   从模板创建周期性定时器文件（用于下载/上传任务）
     * @param name                              定时器名称
     * @param minuteMinutes                     重复间隔（分钟）
     */
    void createPeriodicTimerFromTemplate(const QString &name, int intervalMinutes);

    /**
     * @brief execSystemdCommand   执行systemctl命令，使用QProcess直接调用，不依赖shell
     * @param args                systemctl命令的参数列表
     * @return                    命令输出
     */
    QString execSystemdCommand(const QStringList &args);

    /**
     * @brief execDirectCommand    直接执行命令，不使用shell
     * @param program              程序名
     * @param args                 参数列表
     * @return                     命令输出
     */
    QString execDirectCommand(const QString &program, const QStringList &args = QStringList());

    /**
     * @brief createFile        创建文件
     * @param fileName          文件名称
     * @param content           内容
     * @note 此方法为内部辅助方法，供模板方法使用
     */
    void createFile(const QString &fileName, const QString &content);
signals:

public slots:
private:
    QString m_systemdPath;
};

#endif // CSYSTEMDTIMERCONTROL_H
