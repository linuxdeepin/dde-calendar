// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULERDATABASE_H
#define SCHEDULERDATABASE_H
#include "src/commondatastruct.h"
#include "src/scheduledatainfo.h"
#include "gtest/gtest_prod.h"

#include <QObject>
#include <QSqlDatabase>
#include <QDateTime>

class SchedulerDatabase : public QObject
{
    Q_OBJECT
public:
    explicit SchedulerDatabase(QObject *parent = nullptr);
    void DeleteJob(qint64 id);
    QString GetJob(qint64 id);
    qint64 CreateJob(const Job &job);
    qint64 UpdateJob(const QString &jobInfo);
    bool UpdateJobIgnore(const QString &strignore, qint64 id);
    QList<Job> GetAllOriginJobs();
    QList<Job> GetAllOriginJobs(const QString &key, const QString &strsort = QString());
    QList<Job> GetAllOriginJobsWithRule(const QString &key, const QString &rules);
    QList<Job> GetJobsContainRemind();
    /**
     * @brief saveRemindJob     存储提醒日程的相关信息
     * @param job               提醒日程
     */
    void saveRemindJob(const Job &job);

    /**
     * @brief updateRemindJob       更新对应的稍后提醒日程
     * @param job                   日程信息
     */
    void updateRemindJob(const Job &job);

    /**
     * @brief deleteRemindJobs      根据日程id删除提醒日程信息
     * @param Ids                   日程id集
     */
    void deleteRemindJobs(const QList<qlonglong> &Ids);

    /**
     * @brief deleteRemindJobs      根据日程id和重复id删除提醒日程信息
     * @param jobID                 日程id
     * @param recurid               重复id
     */
    void deleteRemindJobs(const qlonglong &jobID, const qint64 recurid);

    /**
     * @brief getValidRemindJob     获取未提醒的稍后提醒日程
     * @return
     */
    QList<Job> getValidRemindJob();

    /**
     * @brief clearRemindJobDatabase    清空稍后提醒表
     */
    void clearRemindJobDatabase();

    /**
     * @brief getRemindJob      根据日程id和重复id获取对应的提醒日程
     * @param id                日程id
     * @param recurid           重复id
     * @return
     */
    Job getRemindJob(const qint64 id, const qint64 recurid);

    /**
     * @brief getRemindJob      根据日程id获取对应的提醒日程
     * @param id                日程id
     * @return
     */
    QList<Job> getRemindJob(const qint64 id);

    /**
     * @brief getNotifyID       获取桌面顶部通知ID
     * @param id                日程ID
     * @return                  通知ID
     */
    QVector<int> getNotifyID(const qint64 id);

    /**
     * @brief getNotifyID   获取桌面顶部通知ID
     * @param jobID         日程ID
     * @param recurid       重复id
     * @return
     */
    int getNotifyID(const qint64 jobID, const qint64 recurid);

    /**
     * @brief updateNotifyID        更新桌面顶部通知ID
     * @param jobID                 日程ID
     * @param notifyid              通知ID
     */
    void updateNotifyID(const Job &job, int notifyid);

    /**
     * @brief getJobTypeByTypeNo    根据类型编号获取日程类型信息
     * @param iTypeNo               日程类型编号
     * @param jobType               日程类型信息
     * @return bool                 操作结果
     */
    bool getJobTypeByTypeNo(int iTypeNo, JobTypeInfo &jobType);
    /**
     * @brief getJobTypeList        获取日程类型列表
     * @return
     */
    bool getJobTypeList(QList<JobTypeInfo> &lstJobType);

    /**
     * @brief isJobTypeUsed        查询日程类型是否被使用
     * @return
     */
    bool isJobTypeUsed(int iTypeNo);
    /**
     * @brief DeleteJobsByJobType        删除使用指定日程类型的全部日程
     * @return
     */
    bool DeleteJobsByJobType(int iTypeNo);

    /**
    * @brief getJobIDByJobType          根据日程类型获取对应的日程编号
    * @param iTypeNo                    日程类型
    * @return
    */
    QVector<qint64> getJobIDByJobType(int iTypeNo);
    /**
     * @brief addJobType            新增日程类型
     * @param iTypeNo               日程类型编码
     * @param strTypeName           日程类型名称
     * @param iColorTypeNo          日程类型对应颜色编码
     * @param iAuthority            日程类型读写权限
     */
    bool addJobType(const int &iTypeNo, const QString &strTypeName, const int &iColorTypeNo, int iAuthority);
    /**
     * @brief updateJobType         更新日程类型
     * @param iTypeNo               日程类型编码
     * @param strTypeName           日程类型名称
     * @param iColorTypeNo          日程类型对应颜色编码
     */
    bool updateJobType(const int &iTypeNo, const QString &strTypeName, const int &iColorTypeNo);
    /**
     * @brief deleteJobType         删除日程类型
     * @param strTypeNo             日程类型编码
     */
    bool deleteJobType(const int &iTypeNo);

    /**
     * @brief getColorTypeList      获取颜色类型列表
     * @param lstColorType          颜色类型列表
     * @return                      操作结果
     */
    bool getColorTypeList(QList<JobTypeColorInfo> &lstColorType);
    /**
     * @brief addColorType          新增颜色类型
     * @param iTypeNo               颜色类型编码
     * @param strColorHex           颜色16进制编码
     * @param iAuthority            颜色类型读写权限
     */
    bool addColorType(const int &iTypeNo, const QString &strColorHex, const int iAuthority);
    /**
     * @brief updateColorType       更新颜色类型
     * @param iTypeNo               颜色类型编码
     * @param strColorHex           颜色16进制编码
     */
    bool updateColorType(const int &iTypeNo, const QString &strColorHex);
    /**
     * @brief deleteColorType       删除颜色类型
     * @param iTypeNo               颜色类型编码
     */
    bool deleteColorType(const int &iTypeNo);
    QString getDbPath() const;
    void setDbPath(const QString &dbPath);

private:
    void CreateTables();
    /**
     * @brief initJobTypeTables      初始化日程类型，添加默认日程类型、颜色类型
     * @return                       无
     */
    void initJobTypeTables();
    void OpenSchedulerDatabase();
    /**
     * @brief dateTimeToString      将时间转换为string格式
     * @param dateTime
     * @return
     */
    QString dateTimeToString(const QDateTime &dateTime);

signals:

public slots:

private:
    QSqlDatabase m_database;
    QString m_dbPath;
};

#endif // SCHEDULERDATABASE_H
