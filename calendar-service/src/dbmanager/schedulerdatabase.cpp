// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulerdatabase.h"
#include "src/commondef.h"
#include "pinyin/pinyinsearch.h"
#include "src/utils.h"

#include <QDebug>
#include <QSqlError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSqlQuery>
#include <QFile>
#include <QDir>
#include <QTimeZone>

SchedulerDatabase::SchedulerDatabase(QObject *parent)
    : QObject(parent)
    , m_dbPath("")
{
    //旧文件路径
    QString oldDbPatch = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/.config/deepin/dde-daemon/calendar");
    QDir dir;
    //如果该路径不存在，则创建该文件夹
    if (!dir.exists(oldDbPatch)) {
        dir.mkpath(oldDbPatch);
    }
    m_dbPath = oldDbPatch.append("/scheduler.db");
    OpenSchedulerDatabase();
}

//通过id获取日程信息
QString SchedulerDatabase::GetJob(qint64 id)
{
    QString strjson;
    QSqlQuery query(m_database);
    QString strsql = QString("SELECT id, type, title, description, "
                             "all_day, start, end, r_rule, remind, ignore , is_Lunar"
                             " FROM jobs WHERE id = '%1' ")
                         .arg(id);
    //id唯一因此此处最多只有一条数据
    if (query.exec(strsql) && query.next()) {
        QJsonDocument doc;
        QJsonObject obj;
        obj.insert("ID", query.value("id").toInt());
        obj.insert("Type", query.value("type").toInt());
        obj.insert("Title", query.value("title").toString());
        obj.insert("Description", query.value("description").toString());
        obj.insert("AllDay", query.value("all_day").toBool());
        //调整时间格式，方便前端解析
        obj.insert("Start", Utils::toconvertData(query.value("start").toDateTime()));
        obj.insert("End", Utils::toconvertData(query.value("end").toDateTime()));
        obj.insert("RRule", query.value("r_rule").toString());
        obj.insert("Remind", query.value("remind").toString());
        //将QString类型转换为QJsonArray类型，方便前端解析
        obj.insert("Ignore", QJsonDocument::fromJson(query.value("ignore").toString().toUtf8()).array());
        //数据库包含的都是原始数据所以RecurID默认为0
        obj.insert("RecurID", 0);
        obj.insert("IsLunar", query.value("is_Lunar").toBool());

        doc.setObject(obj);
        strjson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }
    return strjson;
}

/**
 * @brief  GetAllOriginJobs 获取所有原始日程
 * @return 返回所有原始日程集合
 */
QList<Job> SchedulerDatabase::GetAllOriginJobs()
{
    QList<Job> jobs;
    QSqlQuery query(m_database);

    QString strsql = QString("select id,type,title,description,all_day,start,end,r_rule,remind,ignore,title_pinyin,is_Lunar"
                             " from jobs ");
    if (query.exec(strsql)) {
        while (query.next()) {
            Job jb;
            jb.ID = query.value("id").toInt();
            jb.Type = query.value("type").toInt();
            jb.Title = query.value("title").toString();
            jb.Description = query.value("description").toString();
            jb.AllDay = query.value("all_day").toBool();
            jb.Start = query.value("start").toDateTime();
            jb.End = query.value("end").toDateTime();
            jb.RRule = query.value("r_rule").toString();
            jb.Remind = query.value("remind").toString();
            jb.Ignore = query.value("ignore").toString();
            jb.Title_pinyin = query.value("title_pinyin").toString();
            jb.IsLunar = query.value("is_Lunar").toBool();
            jobs.append(jb);
        }
    }
    if (query.isActive()) {
        query.finish();
    }

    return jobs;
}

/**
 * @brief  GetAllOriginJobs 获取所有与key相关的job原始数据
 * @param  key 搜索词
 * @param  strsort 查询排序条件
 * @return 返回所有原始日程集合
 */
QList<Job> SchedulerDatabase::GetAllOriginJobs(const QString &key, const QString &strsort)
{
    QList<Job> jobs;
    QSqlQuery query(m_database);
    QString strKey = key.trimmed();
    pinyinsearch *psearch = pinyinsearch::getPinPinSearch();
    QString strsql("select id,type,title,description,all_day,start,end,r_rule,remind,ignore,title_pinyin,is_Lunar from jobs");
    QMap<QString, QString> sqlBindValue;
    if (psearch->CanQueryByPinyin(strKey)) {
        //可以按照拼音查询
        QString pinyin = psearch->CreatePinyinQuery(strKey.toLower());
        strsql += QString(" where instr(UPPER(title), UPPER(:key)) OR title_pinyin LIKE :pinyin");
        sqlBindValue[":key"] = key;
        sqlBindValue[":pinyin"] = pinyin;
    } else if (!key.isEmpty()) {
        //按照key查询
        strsql += QString(" where instr(UPPER(title), UPPER(:key))");
        sqlBindValue[":key"] = key;
    }

    //排序条件不为空
    if (!strsort.isEmpty()) {
        strsql.append(QString("order by :strsort "));
        sqlBindValue[":strsort"] = strsort;
    }
    query.prepare(strsql);
    for (auto iter = sqlBindValue.constBegin(); iter != sqlBindValue.constEnd(); iter++) {
        query.bindValue(iter.key(), iter.value());
    }
    if (query.exec()) {
        while (query.next()) {
            Job jb;
            jb.ID = query.value("id").toInt();
            jb.Type = query.value("type").toInt();
            jb.Title = query.value("title").toString();
            jb.Description = query.value("description").toString();
            jb.AllDay = query.value("all_day").toBool();
            jb.Start = query.value("start").toDateTime();
            jb.End = query.value("end").toDateTime();
            jb.RRule = query.value("r_rule").toString();
            jb.Remind = query.value("remind").toString();
            jb.Ignore = query.value("ignore").toString();
            jb.Title_pinyin = query.value("title_pinyin").toString();
            jb.IsLunar = query.value("is_Lunar").toBool();
            jobs.append(jb);
        }
    }
    if (query.isActive()) {
        query.finish();
    }

    return jobs;
}

QList<Job> SchedulerDatabase::GetAllOriginJobsWithRule(const QString &key, const QString &rules)
{
    QList<Job> jobs;
    QSqlQuery query(m_database);
    QString strKey = key.trimmed();
    QString strrule;
    if (!rules.contains("BYDAY") && rules.contains("DAILY")) {
        // 每日
        strrule = QString("r_rule LIKE '%%1%' AND r_rule NOT LIKE '%BYDAY%' ").arg(rules);
    } else {
        // 工作日 每周 每月 每年
        strrule = QString("r_rule LIKE '%%1%'").arg(rules);
    }

    pinyinsearch *psearch = pinyinsearch::getPinPinSearch();
    QString strsql;
    if (psearch->CanQueryByPinyin(strKey)) {
        //可以按照拼音查询
        QString pinyin = psearch->CreatePinyinQuery(strKey.toLower());
        strsql = QString("select id from jobs where title like '%%1%' or title_pinyin like '%%2%' ").arg(key).arg(pinyin);
    } else if (!key.isEmpty()) {
        //按照key查询
        strsql = QString("select id from jobs where title like '%%1%' ").arg(key);
    } else {
        //如果没有key，则搜索所有
        strsql = QString("select id from jobs ");
    }

    strsql = QString("select id,type,title,description,all_day,start,end,r_rule,remind,ignore,title_pinyin,is_Lunar "
                     " from jobs where id in(%1) and %2")
                 .arg(strsql)
                 .arg(strrule);

    if (query.exec(strsql)) {
        while (query.next()) {
            Job jb;
            jb.ID = query.value("id").toInt();
            jb.Type = query.value("type").toInt();
            jb.Title = query.value("title").toString();
            jb.Description = query.value("description").toString();
            jb.AllDay = query.value("all_day").toBool();
            jb.Start = query.value("start").toDateTime();
            jb.End = query.value("end").toDateTime();
            jb.RRule = query.value("r_rule").toString();
            jb.Remind = query.value("remind").toString();
            jb.Ignore = query.value("ignore").toString();
            jb.Title_pinyin = query.value("title_pinyin").toString();
            jb.IsLunar = query.value("is_Lunar").toBool();
            jobs.append(jb);
        }
    }
    if (query.isActive()) {
        query.finish();
    }

    return jobs;
}

/**
 * @brief  GetJobsContainRemind 获取包含提醒规则的jos集合
 */
QList<Job> SchedulerDatabase::GetJobsContainRemind()
{
    QList<Job> jobs;
    QSqlQuery query(m_database);
    QString strSql("select id,type,title,description,all_day,start,end,r_rule,remind,ignore,title_pinyin,is_Lunar "
                   "from jobs where remind is not null and remind !=' ' ");
    if (query.exec(strSql)) {
        while (query.next()) {
            Job jb;
            jb.ID = query.value("id").toInt();
            jb.Type = query.value("type").toInt();
            jb.Title = query.value("title").toString();
            jb.Description = query.value("description").toString();
            jb.AllDay = query.value("all_day").toBool();
            jb.Start = query.value("start").toDateTime();
            jb.End = query.value("end").toDateTime();
            jb.RRule = query.value("r_rule").toString();
            jb.Remind = query.value("remind").toString();
            jb.Ignore = query.value("ignore").toString();
            jb.Title_pinyin = query.value("title_pinyin").toString();
            jb.IsLunar = query.value("is_Lunar").toBool();
            jobs.append(jb);
        }
    }
    if (query.isActive()) {
        query.finish();
    }
    return jobs;
}

//存储提醒日程的相关信息
void SchedulerDatabase::saveRemindJob(const Job &job)
{
    QSqlQuery query(m_database);
    QString strsql = "INSERT INTO jobsReminder (jobid,recurid,remindCount,notifyid, remindTime,jobStartTime,jobEndTime)"
                     "values (:jobid,:recurid,:remindCount,:notifyid,:remindTime,:jobStartTime,:jobEndTime)";
    query.prepare(strsql);
    int i = 0;
    query.bindValue(i, job.ID);
    query.bindValue(++i, job.RecurID);
    query.bindValue(++i, job.RemindLaterCount);
    //通知提醒id默认为-1,表示未提醒
    query.bindValue(++i, -1);
    query.bindValue(++i, job.RemidTime);
    query.bindValue(++i, job.Start);
    query.bindValue(++i, job.End);
    if (query.exec()) {
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
    }
}

void SchedulerDatabase::updateRemindJob(const Job &job)
{
    //点击稍后提醒后，更新信息并设置通知提醒为-1
    QString strsql = QString("UPDATE jobsReminder SET remindCount = '%1' , remindTime = '%2', notifyid = -1 WHERE jobid = %3 and recurid = %4 ")
                         .arg(job.RemindLaterCount)
                         .arg(dateTimeToString(job.RemidTime))
                         .arg(job.ID)
                         .arg(job.RecurID);
    QSqlQuery query(m_database);
    if (query.exec(strsql)) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qWarning() << __FUNCTION__ << query.lastError();
    }
}

void SchedulerDatabase::deleteRemindJobs(const QList<qlonglong> &Ids)
{
    if (Ids.size() == 0)
        return;
    QStringList idList;
    for (int i = 0; i < Ids.size(); ++i) {
        idList.append(QString::number(Ids.at(i)));
    }
    QSqlQuery query(m_database);
    QString sql = QString("delete from jobsReminder where  jobsReminder.jobid in ( %1)").arg(idList.join(","));
    if (query.exec(sql)) {
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qWarning() << __FUNCTION__ << query.lastError();
    }
}

void SchedulerDatabase::deleteRemindJobs(const qlonglong &jobID, const qint64 recurid)
{
    QSqlQuery query(m_database);
    QString sql = QString("delete from jobsReminder where  jobsReminder.jobid = %1 and jobsReminder.recurid = %2")
                      .arg(jobID)
                      .arg(recurid);
    if (query.exec(sql)) {
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qWarning() << __FUNCTION__ << query.lastError();
    }
}

QList<Job> SchedulerDatabase::getValidRemindJob()
{
    QList<Job> jobs{};
    QSqlQuery query(m_database);
    QString sql("select jobs.id, jobs.all_day,jobs.type,jobs.title,jobs.description,jobs.is_Lunar,jobsReminder.jobStartTime as start,"
                "jobsReminder.jobEndTime as end,jobs.r_rule,jobs.remind,jobs.ignore,jobs.title_pinyin,jobsReminder.remindCount,"
                "jobsReminder.remindTime , jobsReminder.recurid from jobs left join jobsReminder on jobs.id = jobsReminder.jobid "
                "where jobsReminder.remindTime > ");
    sql += QString(" '%1'").arg(dateTimeToString(QDateTime::currentDateTime()));
    if (query.exec(sql)) {
        while (query.next()) {
            Job jb;
            jb.ID = query.value("id").toInt();
            jb.Type = query.value("type").toInt();
            jb.Title = query.value("title").toString();
            jb.Description = query.value("description").toString();
            jb.AllDay = query.value("all_day").toBool();
            jb.Start = query.value("start").toDateTime();
            jb.End = query.value("end").toDateTime();
            jb.RRule = query.value("r_rule").toString();
            jb.Remind = query.value("remind").toString();
            jb.Ignore = query.value("ignore").toString();
            jb.Title_pinyin = query.value("title_pinyin").toString();
            jb.RemindLaterCount = query.value("remindCount").toInt();
            jb.RemidTime = query.value("remindTime").toDateTime();
            jb.RecurID = query.value("recurid").toInt();
            jb.IsLunar = query.value("is_Lunar").toBool();
            jobs.append(jb);
        }
    }
    if (query.isActive()) {
        query.finish();
    }
    return jobs;
}

void SchedulerDatabase::clearRemindJobDatabase()
{
    QSqlQuery query(m_database);
    QString sql("delete from jobsReminder");
    if (query.exec(sql)) {
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qWarning() << __FUNCTION__ << query.lastError();
    }

}

Job SchedulerDatabase::getRemindJob(const qint64 id, const qint64 recurid)
{
    QSqlQuery query(m_database);
    QString sql = QString("select jobs.id, jobs.all_day,jobs.type,jobs.title,jobs.description,jobs.is_Lunar,"
                          "jobsReminder.jobStartTime as start,jobsReminder.jobEndTime as end,jobs.r_rule,jobs.remind,jobs.ignore,jobs.title_pinyin,"
                          "jobsReminder.remindCount,jobsReminder.remindTime , jobsReminder.recurid from jobs inner join jobsReminder "
                          "on jobs.id = jobsReminder.jobid   where jobsReminder.jobid = %1 and jobsReminder.recurid = %2")
                      .arg(id)
                      .arg(recurid);

    //id唯一因此此处最多只有一条数据
    Job jb;
    if (query.exec(sql) && query.next()) {
        jb.ID = query.value("id").toInt();
        jb.Type = query.value("type").toInt();
        jb.Title = query.value("title").toString();
        jb.Description = query.value("description").toString();
        jb.AllDay = query.value("all_day").toBool();
        jb.Start = query.value("start").toDateTime();
        jb.End = query.value("end").toDateTime();
        jb.RRule = query.value("r_rule").toString();
        jb.Remind = query.value("remind").toString();
        jb.Ignore = query.value("ignore").toString();
        jb.Title_pinyin = query.value("title_pinyin").toString();
        jb.RemindLaterCount = query.value("remindCount").toInt();
        jb.RemidTime = query.value("remindTime").toDateTime();
        jb.RecurID = query.value("recurid").toInt();
        jb.IsLunar = query.value("is_Lunar").toBool();
    } else {
        qWarning() << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }
    return jb;
}

QList<Job> SchedulerDatabase::getRemindJob(const qint64 id)
{
    QSqlQuery query(m_database);
    QString sql = QString("select jobs.id, jobs.all_day,jobs.type,jobs.title,jobs.description,jobs.is_Lunar,"
                          "jobsReminder.jobStartTime as start,jobsReminder.jobEndTime as end,jobs.r_rule,jobs.remind,jobs.ignore,jobs.title_pinyin,"
                          "jobsReminder.remindCount,jobsReminder.remindTime , jobsReminder.recurid from jobs inner join jobsReminder "
                          "on jobs.id = jobsReminder.jobid   where jobsReminder.jobid = %1")
                      .arg(id);

    //id唯一因此此处最多只有一条数据
    QList<Job> jbList;
    if (query.exec(sql) && query.next()) {
        Job jb;
        jb.ID = query.value("id").toInt();
        jb.Type = query.value("type").toInt();
        jb.Title = query.value("title").toString();
        jb.Description = query.value("description").toString();
        jb.AllDay = query.value("all_day").toBool();
        jb.Start = query.value("start").toDateTime();
        jb.End = query.value("end").toDateTime();
        jb.RRule = query.value("r_rule").toString();
        jb.Remind = query.value("remind").toString();
        jb.Ignore = query.value("ignore").toString();
        jb.Title_pinyin = query.value("title_pinyin").toString();
        jb.RemindLaterCount = query.value("remindCount").toInt();
        jb.RemidTime = query.value("remindTime").toDateTime();
        jb.RecurID = query.value("recurid").toInt();
        jb.IsLunar = query.value("is_Lunar").toBool();
        jbList.append(jb);
    } else {
        qWarning() << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }
    return jbList;
}

//获取桌面顶部通知ID
QVector<int> SchedulerDatabase::getNotifyID(const qint64 id)
{
    QVector<int> notifyid;
    QSqlQuery query(m_database);
    QString sql("select distinct jobsReminder.notifyid from jobsReminder where jobsReminder.jobid = ");
    sql += QString::number(id);
    if (query.exec(sql) && query.next()) {
        notifyid.append(query.value("notifyid").toInt());
    }
    if (query.isActive()) {
        query.finish();
    }
    return notifyid;
}

int SchedulerDatabase::getNotifyID(const qint64 jobID, const qint64 recurid)
{
    int notifyid = -1;
    QSqlQuery query(m_database);
    QString sql = QString("select distinct jobsReminder.notifyid from jobsReminder where jobsReminder.jobid = %1 and jobsReminder.recurid = %2")
                      .arg(jobID)
                      .arg(recurid);
    if (query.exec(sql) && query.next()) {
        notifyid = query.value("notifyid").toInt();
    }
    if (query.isActive()) {
        query.finish();
    }
    return notifyid;
}

//更新桌面顶部通知ID
void SchedulerDatabase::updateNotifyID(const Job &job, int notifyid)
{
    QString strsql =
        QString("UPDATE jobsReminder SET notifyid = '%1'  WHERE jobid = %2 and recurid = %3")
        .arg(notifyid)
        .arg(job.ID)
        .arg(job.RecurID);
    QSqlQuery query(m_database);
    if (query.exec(strsql)) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qWarning() << __FUNCTION__ << query.lastError();
    }
}

/**
 * @brief  CreateTables 创建日程相关数据表（新用户创建）
 */
void SchedulerDatabase::CreateTables()
{
    QSqlQuery query(m_database);
    bool ret;
    //table job_types
    ret = query.exec("CREATE TABLE IF NOT EXISTS \"job_types\" (\"id\" integer primary key autoincrement,\"created_at\""
                     " datetime,\"updated_at\" datetime,\"deleted_at\" datetime,\"name\" varchar(255),\"color\" varchar(255) )");
    if (!ret) {
        qDebug() << query.lastError();
    }

    ret = query.exec("CREATE INDEX IF NOT EXISTS idx_job_types_deleted_at ON \"job_types\"(deleted_at)");
    if (!ret) {
        qDebug() << query.lastError();
    }
    //table jobs
    ret = query.exec("CREATE TABLE IF NOT EXISTS \"jobs\" (\"id\" integer primary key autoincrement,"
                     "\"created_at\" datetime,\"updated_at\" datetime,\"deleted_at\" datetime,"
                     "\"type\" integer,\"title\" varchar(255),\"description\" varchar(255),"
                     "\"all_day\" bool,\"start\" datetime,\"end\" datetime,\"r_rule\" varchar(255),"
                     "\"remind\" varchar(255),\"ignore\" varchar(255) , \"title_pinyin\" varchar(255))");
    if (!ret) {
        qDebug() << query.lastError();
    }
    ret = query.exec("CREATE INDEX  IF NOT EXISTS idx_jobs_deleted_at ON \"jobs\"(deleted_at)");
    if (!ret) {
        qDebug() << query.lastError();
    }

    ret = query.exec("CREATE TABLE IF NOT EXISTS \"jobsReminder\" (\"id\" integer primary key autoincrement,"
                     "\"jobid\" integer,\"recurid\" integer,\"remindCount\" integer ,\"notifyid\" integer ,"
                     "\"remindTime\" datetime ,\"jobStartTime\" datetime ,\"jobEndTime\" datetime) ");
    if (!ret) {
        qDebug() << query.lastError();
    }

    ret = query.exec("CREATE TABLE IF NOT EXISTS JobType (                                                  \
                                         ID          INTEGER     PRIMARY KEY   AUTOINCREMENT        \
                                        ,TypeNo      INTEGER     NOT NULL      UNIQUE               \
                                        ,TypeName    VARCHAR(20) NOT NULL                           \
                                        ,ColorTypeNo INTEGER     NOT NULL                           \
                                        ,CreateTime  DATETIME    NOT NULL                           \
                                        ,Authority   INTEGER     NOT NULL                           \
                                        )");//Authority：用来标识权限，0：读 1：展示 2：改 4：删

    if (!ret) {
        qDebug() << query.lastError();
    }
    ret = query.exec("CREATE TABLE IF NOT EXISTS  ColorType (                                                \
                                         ID          INTEGER     PRIMARY KEY   AUTOINCREMENT        \
                                        ,TypeNo      INTEGER     NOT NULL      UNIQUE               \
                                        ,ColorHex    CHAR(10)    NOT NULL                           \
                                        ,Authority   INTEGER     NOT NULL                           \
                                        )");//Authority：用来标识权限，0：读 1：展示 2：改 4：删
    if (!ret) {
        qDebug() << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }

    m_database.commit();
}
/**
 * @brief initJobTypeTables      初始化日程类型，添加默认日程类型、颜色类型
 * @return                       无
 */
void SchedulerDatabase::initJobTypeTables()
{
    //getJobTypeByTypeNo(int iTypeNo, JobTypeInfo jobType);
    JobTypeInfo jobType;
    if (!getJobTypeByTypeNo(1, jobType) || jobType.getJobTypeNo() > 0) {
        return;
    }
//    addJobType(1, "Work", 1, 1);
//    addJobType(2, "Life", 7, 1);
//    addJobType(3, "Other", 4, 1);
    addJobType(4, "Festival", 2, 0);

    addColorType(1, "#ff5e97", 1);
    addColorType(2, "#ff9436", 1);
    addColorType(3, "#ffdc00", 1);
    addColorType(4, "#5bdd80", 1);
    addColorType(5, "#00b99b", 1);
    addColorType(6, "#4293ff", 1);
    addColorType(7, "#5d51ff", 1);
    addColorType(8, "#a950ff", 1);
    addColorType(9, "#717171", 1);

    return;
}

void SchedulerDatabase::OpenSchedulerDatabase()
{
    // 重复调用QSQLITE会导致数据库连接覆盖导致失败，需指定每部分的连接名称
    m_database = QSqlDatabase::addDatabase("QSQLITE", "SchedulerDatabase");
    const QString &dbpath = getDbPath();
    m_database.setDatabaseName(dbpath);
    //这里用QFile来修改日历数据库文件的权限
    QFile file;
    file.setFileName(dbpath);
    //如果不存在该文件则创建
    if (!file.exists()) {
        m_database.open();
        m_database.close();
    }
    //将权限修改为600（对文件的所有者可以读写，其他用户不可读不可写）
    if (!file.setPermissions(QFile::WriteOwner | QFile::ReadOwner)) {
        qWarning() << "权限设置失败，错误:" << file.errorString();
    }
    if (m_database.open()) {
        const QStringList tables = m_database.tables();
        QSqlQuery query(m_database);
        CreateTables();
        initJobTypeTables();

        //jobs需要添加一个是否为农历日程的字段
        //判断jobs表中是否有该字段，如果有则不处理
        QString getHasIsLunarField = "select count(1) from sqlite_master where type='table' and "
                                     "tbl_name = 'jobs' and sql like '%is_Lunar%'";
        if (query.exec(getHasIsLunarField) && query.next()) {
            //获取是否存在为农历标识字段，若存在则返回1,不存在则返回0
            int fieldNum = query.value(0).toInt();
            if (fieldNum == 0) {
                //添加字段
                QString alterField = "alter table jobs add is_Lunar bool default false ";
                if (!query.exec(alterField)) {
                    qWarning() << "Failed to add field," << query.lastError();
                };
            }
        } else {
            qWarning() << "select field failed," << query.lastError();
        }
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();

    } else {
        qDebug() << __FUNCTION__ << m_database.lastError();
    }
}

QString SchedulerDatabase::dateTimeToString(const QDateTime &dateTime)
{
    QTime _offsetTime = QTime(0, 0).addSecs(dateTime.timeZone().offsetFromUtc(dateTime));
    return QString("%1.000+%2").arg(dateTime.toString("yyyy-MM-ddThh:mm:ss")).arg(_offsetTime.toString("hh:mm"));
}

QString SchedulerDatabase::getDbPath() const
{
    return m_dbPath;
}

void SchedulerDatabase::setDbPath(const QString &dbPath)
{
    m_dbPath = dbPath;
}

// 执行删除日程的数据库SQL命令，以ID为依据
void SchedulerDatabase::DeleteJob(qint64 id)
{
    QString strsql = QString("DELETE FROM jobs WHERE id = %1").arg(id);
    QSqlQuery query(m_database);
    if (query.exec(strsql)) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
    }
}

// 执行添加日程的数据库SQL命令，并返回其ID值
qint64 SchedulerDatabase::CreateJob(const Job &job)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    currentDateTime.setOffsetFromUtc(currentDateTime.offsetFromUtc());
    QSqlQuery query(m_database);
    QString strsql = "INSERT INTO jobs (created_at, updated_at, type, title,"
                     "description, all_day, start, end, r_rule, remind, ignore, title_pinyin,is_Lunar)"
                     "values (:created_at, :updated_at, :type, :title, :description,"
                     ":all_day, :start, :end, :r_rule, :remind, :ignore, :title_pinyin,:is_Lunar)";
    query.prepare(strsql);
    int i = 0;
    query.bindValue(i, currentDateTime);
    query.bindValue(++i, currentDateTime);
    query.bindValue(++i, job.Type);
    query.bindValue(++i, job.Title);
    query.bindValue(++i, job.Description);
    query.bindValue(++i, job.AllDay);
    //修改开始结束时间格式，与前端保持一致
    query.bindValue(++i, Utils::toconvertData(job.Start));
    query.bindValue(++i, Utils::toconvertData(job.End));
    query.bindValue(++i, job.RRule);
    query.bindValue(++i, job.Remind);
    query.bindValue(++i, job.Ignore);
    query.bindValue(++i, job.Title_pinyin);
    query.bindValue(++i, job.IsLunar);
    if (query.exec()) {
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
    }

    // 获取最新刚插入日程的ID。由于id为数据库自增，因此插入的日程id一直为最大值。
    qint64 jobID;
    QString returnIdsql = "SELECT MAX(id) FROM jobs";
    if (query.exec(returnIdsql) && query.next()) {
        jobID = query.value(0).toInt();
        if (query.isActive()) {
            query.finish();
        }
        // 共有两次sql语句执行，commit操作需要置于最后
        m_database.commit();
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
        return -1;
    }
    return jobID;
}

// 根据传入的jobInfo中的Id来更新数据库中相应的数据
qint64 SchedulerDatabase::UpdateJob(const QString &jobInfo)
{
    // TODO: 对job数据进行合法性检测
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jobInfo.toLocal8Bit(), &json_error));
    if (json_error.error != QJsonParseError::NoError) {
        return -1;
    }
    QJsonObject rootObj = jsonDoc.object();
    // 此处Ignore参数需要单独解析，后续pinyin参数也可能会单独解析
    QJsonArray subArray = rootObj.value("Ignore").toArray();
    QJsonDocument doc;
    doc.setArray(subArray);

    QDateTime currentDateTime = QDateTime::currentDateTime();
    currentDateTime.setOffsetFromUtc(currentDateTime.offsetFromUtc());
    QSqlQuery query(m_database);
    QString strsql = "UPDATE jobs SET updated_at = ?, type = ?, title = ?, "
                     "description = ?, all_day = ?, start = ?, end = ?, r_rule = ?, "
                     "remind = ?, ignore = ?, title_pinyin = ? ,is_Lunar = ? WHERE id = ?";
    query.prepare(strsql);
    qint64 id = rootObj.value("ID").toInt();
    int i = 0;
    //修改updatetime时间格式
    query.bindValue(i, currentDateTime);
    query.bindValue(++i, rootObj.value("Type").toInt());
    query.bindValue(++i, rootObj.value("Title").toString());
    query.bindValue(++i, rootObj.value("Description").toString());
    query.bindValue(++i, rootObj.value("AllDay").toBool());
    query.bindValue(++i, rootObj.value("Start").toString());
    query.bindValue(++i, rootObj.value("End").toString());
    query.bindValue(++i, rootObj.value("RRule").toString());
    query.bindValue(++i, rootObj.value("Remind").toString());
    query.bindValue(++i, QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    query.bindValue(++i, pinyinsearch::getPinPinSearch()->CreatePinyin(rootObj.value("Title").toString()));
    query.bindValue(++i, rootObj.value("IsLunar").toBool());
    query.bindValue(++i, id);
    if (query.exec()) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
    }

    return id;
}

bool SchedulerDatabase::UpdateJobIgnore(const QString &strignore, qint64 id)
{
    QSqlQuery query(m_database);
    QString strsql = QString("UPDATE jobs SET ignore='%1' where id=%2;").arg(strignore).arg(id);
    bool bsuccess = false;
    if (query.exec(strsql)) {
        if (query.isActive()) {
            query.finish();
        }
        bsuccess = m_database.commit();
    }

    return bsuccess;
}

/**
 * @brief getJobTypeByTypeNo    根据类型编号获取日程类型信息
 * @return
 */
bool SchedulerDatabase::getJobTypeByTypeNo(int iTypeNo, JobTypeInfo &jobType)
{
    bool bRet = false;
    QSqlQuery query(m_database);

    QString strsql = QString("   SELECT JobType.TypeNo, JobType.TypeName, JobType.ColorTypeNo, ColorType.ColorHex, JobType.Authority            "
                             "     FROM JobType LEFT JOIN ColorType               "
                             "       ON JobType.ColorTypeNo = ColorType.TypeNo    "
                             "    WHERE JobType.TypeNo = %1                       ").arg(iTypeNo);//单个查询时，不过滤Authority为0的结果
    bRet = query.exec(strsql);
    if (bRet) {
        while (query.next()) {
            jobType.setJobTypeNo(query.value("TypeNo").toInt());
            jobType.setJobTypeName(query.value("TypeName").toString());
            jobType.getColorInfo().setTypeNo(query.value("ColorTypeNo").toInt());
            jobType.getColorInfo().setColorHex(query.value("ColorHex").toString());
            jobType.setAuthority(query.value("Authority").toInt());
        }
    }
    if (query.isActive()) {
        query.finish();
    }
    return bRet;
}
/**
 * @brief getJobTypeList        获取日程类型json串
 * @param 无
 */
bool SchedulerDatabase::getJobTypeList(QList<JobTypeInfo> &lstJobType)
{
    bool bRet = false;
    QSqlQuery query(m_database);

    QString strsql = QString("   SELECT JobType.TypeNo, JobType.TypeName, JobType.ColorTypeNo, ColorType.ColorHex, JobType.Authority            "
                             "     FROM JobType LEFT JOIN ColorType             "
                             "       ON JobType.ColorTypeNo = ColorType.TypeNo  "
                             "    WHERE JobType.Authority > 0                   "
                             " ORDER BY JobType.CreateTime                      ");
    bRet = query.exec(strsql);
    if (bRet) {
        QList<QList<QVariant> > lstDefault;
        lstDefault.append({1, tr("Work"),  1, "#ff5e97", 1});
        lstDefault.append({2, tr("Life"),  7, "#5d51ff", 1});
        lstDefault.append({3, tr("Other"), 4, "#5bdd80", 1});

        for (QList<QVariant> lst : lstDefault) {
            JobTypeInfo jobType;
            jobType.setJobTypeNo(lst[0].toInt());
            jobType.setJobTypeName(lst[1].toString());
            jobType.setColorTypeNo(lst[2].toInt());
            jobType.setColorHex(lst[3].toString());
            jobType.setAuthority(lst[4].toInt());
            lstJobType.append(jobType);
        }

        while (query.next()) {
            JobTypeInfo jobType;
            jobType.setJobTypeNo(query.value("TypeNo").toInt());
            jobType.setJobTypeName(query.value("TypeName").toString());
            jobType.setColorTypeNo(query.value("ColorTypeNo").toInt());
            jobType.setColorHex(query.value("ColorHex").toString());
            jobType.setAuthority(query.value("Authority").toInt());
            lstJobType.append(jobType);
        }
    }
    if (query.isActive()) {
        query.finish();
    }

    return bRet;
}

/**
 * @brief isJobTypeUsed        查询日程类型是否被使用
 * @return
 */
bool SchedulerDatabase::isJobTypeUsed(int iTypeNo)
{
    bool bRet = false;
    QSqlQuery query(m_database);

    QString strsql = QString("   SELECT count(1) FROM jobs WHERE type = %1").arg(iTypeNo);
    if (query.exec(strsql) && query.next()) {
        //获取是否存在为农历标识字段，若存在则返回1,不存在则返回0
        int fieldNum = query.value(0).toInt();
        if (fieldNum > 0) {
            //被使用
            bRet = true;
        }
    }
    if (query.isActive()) {
        query.finish();
    }

    return bRet;
}

bool SchedulerDatabase::DeleteJobsByJobType(int iTypeNo)
{
    bool bRet = false;
    QSqlQuery query(m_database);

    QString strsql = QString("DELETE FROM jobs  WHERE type = %1").arg(iTypeNo);
    query.prepare(strsql);
    bRet = query.exec();
    if (bRet) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qWarning() << __FUNCTION__ << query.lastError();
        qWarning() << strsql;
    }
    return bRet;
}

QVector<qint64> SchedulerDatabase::getJobIDByJobType(int iTypeNo)
{
    QVector<qint64> jobsID;
    QSqlQuery query(m_database);

    QString strsql = QString(" SELECT       \
                             id             \
                         FROM               \
                             jobs           \
                         WHERE              \
                             jobs.\"type\" = %1;")
                         .arg(iTypeNo);
    if (query.exec(strsql)) {
        //获取所有的有效数据
        while (query.next()) {
            jobsID.append(query.value("id").toInt());
        }
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qWarning() << query.lastError();
    }
    return jobsID;
}
/**
 * @brief addJobType            新增日程类型
 * @param iTypeNo               日程类型编码
 * @param strTypeName           日程类型名称
 * @param iColorTypeNo          日程类型对应颜色编码
 * @param iAuthority            日程类型读写权限
 */
bool SchedulerDatabase::addJobType(const int &iTypeNo, const QString &strTypeName, const int &iColorTypeNo, int iAuthority)
{
    bool bRet = false;
    QDateTime currentDateTime = QDateTime::currentDateTime();

    QSqlQuery query(m_database);
    QString strsql = "INSERT INTO JobType  (TypeNo,   TypeName,  ColorTypeNo,  CreateTime,  Authority)"
                     "            VALUES   (:TypeNo, :TypeName, :ColorTypeNo, :CreateTime, :Authority)";
    query.prepare(strsql);
    int i = 0;
    query.bindValue(i, iTypeNo);
    query.bindValue(++i, strTypeName);
    query.bindValue(++i, iColorTypeNo);
    currentDateTime.setOffsetFromUtc(currentDateTime.offsetFromUtc());
    query.bindValue(++i, currentDateTime);
    query.bindValue(++i, iAuthority);
    bRet = query.exec();
    if (bRet) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
    }
    return bRet;
}
/**
 * @brief updateJobType         更新日程类型
 * @param iTypeNo               日程类型编码
 * @param strTypeName           日程类型名称
 * @param iColorTypeNo          日程类型对应颜色编码
 */
bool SchedulerDatabase::updateJobType(const int &iTypeNo, const QString &strTypeName, const int &iColorTypeNo)
{
    bool bRet = false;

    QSqlQuery query(m_database);
    //使用占位符的方式更新数据库
    QString strsql = "UPDATE JobType SET TypeName = ?,ColorTypeNo = ? WHERE TypeNo = ?";
    query.prepare(strsql);
    int i = 0;
    query.bindValue(i, strTypeName);
    query.bindValue(++i, iColorTypeNo);
    query.bindValue(++i, iTypeNo);
    bRet = query.exec();
    if (bRet) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qWarning() << Q_FUNC_INFO << query.lastError();
        qWarning() << strsql;
    }
    return bRet;
}
/**
 * @brief deleteJobType         删除日程类型
 * @param strTypeNo             日程类型编码
 */
bool SchedulerDatabase::deleteJobType(const int &iTypeNo)
{
    bool bRet = false;
    QSqlQuery query(m_database);
    QString strsql = QString("DELETE FROM JobType  WHERE TypeNo = %1").arg(iTypeNo);

    query.prepare(strsql);
    bRet = query.exec();
    if (bRet) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
    }
    return bRet;
}

/**
 * @brief getColorTypeList      获取颜色类型json串
 * @param 无
 */
bool SchedulerDatabase::getColorTypeList(QList<JobTypeColorInfo> &lstColorType)
{
    bool bRet = false;
    QSqlQuery query(m_database);

    QString strsql = QString("   SELECT TypeNo, ColorHex, Authority      "
                             "     FROM ColorType             "
                             " ORDER BY TypeNo");
    bRet = query.exec(strsql);
    if (bRet) {
        while (query.next()) {
            JobTypeColorInfo colorType;
            colorType.setTypeNo(query.value("TypeNo").toInt());
            colorType.setColorHex(query.value("ColorHex").toString());
            colorType.setAuthority(query.value("Authority").toInt());
            lstColorType.append(colorType);
        }
    }
    if (query.isActive()) {
        query.finish();
    }

    return bRet;
}
/**
 * @brief addColorType          新增颜色类型
 * @param iTypeNo               颜色类型编码
 * @param strColorHex           颜色16进制编码
 * @param iAuthority            颜色类型读写权限
 */
bool SchedulerDatabase::addColorType(const int &iTypeNo, const QString &strColorHex, const int iAuthority)
{
    bool bRet = false;
    QSqlQuery query(m_database);
    QString strsql = "INSERT INTO ColorType  (TypeNo, ColorHex, Authority) VALUES(:TypeNo, :ColorHex, :Authority)";
    query.prepare(strsql);
    int i = 0;
    query.bindValue(i, iTypeNo);
    query.bindValue(++i, strColorHex);
    query.bindValue(++i, iAuthority);
    bRet = query.exec();
    if (bRet) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
    }
    return bRet;
}

/**
 * @brief updateColorType       更新颜色类型
 * @param iTypeNo               颜色类型编码
 * @param strColorHex           颜色16进制编码
 */
bool SchedulerDatabase::updateColorType(const int &iTypeNo, const QString &strColorHex)
{
    bool bRet = false;
    QSqlQuery query(m_database);
    QString strsql = "UPDATE ColorType  SET strColorHex = ? WHERE TypeNo = ?";

    query.prepare(strsql);
    int i = 0;
    query.bindValue(i, strColorHex);
    query.bindValue(++i, iTypeNo);
    bRet = query.exec();
    if (bRet) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
    }
    return bRet;
}
/**
 * @brief deleteColorType       删除颜色类型
 * @param iTypeNo               颜色类型编码
 */
bool SchedulerDatabase::deleteColorType(const int &iTypeNo)
{
    bool bRet = false;
    QSqlQuery query(m_database);
    QString strsql = QString("DELETE FROM ColorType  WHERE TypeNo = %1").arg(iTypeNo);

    query.prepare(strsql);
    bRet = query.exec();
    if (bRet) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qDebug() << __FUNCTION__ << query.lastError();
    }
    return bRet;
}
