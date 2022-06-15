/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ddatabasemanagement.h"

#include "ddatabase.h"
#include "vcalformat.h"
#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "units.h"
#include "recurrence.h"
#include "alarm.h"
#include "icalformat.h"
#include "memorycalendar.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QJsonArray>

const QString localDBName = "localAccount.db";

DDataBaseManagement::DDataBaseManagement()
    : m_newDatabaseName("accountmanager.db")
    , m_oldDatabaseName("scheduler.db")
{
    //旧文件路径
    QString oldDbPatch = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/.config/deepin/dde-daemon/calendar");
    setOldDatabasePath(oldDbPatch);
    //新文件路径
    QString newDbPatch = getDBPath();
    setNewDatabasePath(newDbPatch);
    QString newDB(newDatabasePath() + "/" + m_newDatabaseName);
    //如果新数据库不存在
    if (!databaseExists(newDatabasePath())) {
        QString localAccountDB(newDatabasePath() + "/" + localDBName);
        DAccountManagerDataBase accountManager;
        accountManager.setDBPath(newDB);
        accountManager.setLoaclDB(localDBName);
        accountManager.initDBData();

        DAccount::List accountList = accountManager.getAccountList();
        DAccount::Ptr localAccount;
        for (auto account : accountList) {
            if (account->accountType() == DAccount::Account_Local) {
                localAccount = account;
                break;
            }
        }
        DAccountDataBase localDB(localAccount);
        localDB.setDBPath(localAccountDB);
        localDB.initDBData();

        //判断是否存在旧的数据库
        QString oldDBFile(oldDatabasePath() + "/" + m_oldDatabaseName);
        if (databaseExists(oldDBFile)) {
            //对数据进行迁移
            QSqlDatabase oldDB = QSqlDatabase::addDatabase("QSQLITE", "oldDB");
            oldDB.setDatabaseName(oldDBFile);
            oldDB.open();

            m_typeMap.insert(1, "107c369e-b13a-4d45-9ff3-de4eb3c0475b");
            m_typeMap.insert(2, "24cf3ae3-541d-487f-83df-f068416b56b6");
            m_typeMap.insert(3, "403bf009-2005-4679-9c76-e73d9f83a8b4");

            //获取类型
            if (hasTypeDB(oldDB)) {
                DScheduleType::List typeList = queryOldJobTypeData(oldDB);
                foreach (auto &type, typeList) {
                    m_typeMap.insert(type->typeID().toInt(), localDB.createScheduleType(type));
                }

                //获取颜色
                QVector<DTypeColor> colorList = queryOldTypeColorData(oldDB);
                foreach (auto &color, colorList) {
                    localDB.addTypeColor(color.colorID(), color.colorCode(), color.privilege());
                }
            }

            DSchedule::List scheduleList = queryOldJobData(oldDB, hasLunnarField(oldDB));
            //将原数据库中日程编号与当前日程编号匹配上
            foreach (auto schedule, scheduleList) {
                int scheduleID = schedule->uid().toInt();
                m_schedule.insert(scheduleID, localDB.createSchedule(schedule));
            }

            //提醒任务
            if (hasRemindDB(oldDB)) {
                DRemindData::List remindList = querOldRemindData(oldDB);
                foreach (auto remind, remindList) {
                    remind->setAccountID(localAccount->accountID());
                    localDB.createRemindInfo(remind);
                }
            }
        }
    }
}

QString DDataBaseManagement::newDatabasePath() const
{
    return m_newDatabasePath;
}

void DDataBaseManagement::setNewDatabasePath(const QString &newDatabasePath)
{
    m_newDatabasePath = newDatabasePath;
}

QString DDataBaseManagement::oldDatabasePath() const
{
    return m_oldDatabasePath;
}

void DDataBaseManagement::setOldDatabasePath(const QString &oldDatabasePath)
{
    m_oldDatabasePath = oldDatabasePath;
}

bool DDataBaseManagement::databaseExists(const QString &databasePath, bool create)
{
    QDir dir;
    bool exist = false;
    if (dir.exists(databasePath)) {
        exist = true;
    } else {
        if (create) {
            dir.mkpath(databasePath);
        }
    }
    return exist;
}

bool DDataBaseManagement::hasLunnarField(QSqlDatabase &db)
{
   SqliteQuery query(db);
    bool haslunnar = false;
    QString hasIsLunarField = "select count(1) from sqlite_master where type='table' and "
                              "tbl_name = 'jobs' and sql like '%is_Lunar%'";
    if (query.exec(hasIsLunarField) && query.next()) {
        //获取是否存在为农历标识字段，若存在则返回1,不存在则返回0
        haslunnar = query.value(0).toInt();
    }
    if (query.isActive()) {
        query.finish();
    }
    return haslunnar;
}

bool DDataBaseManagement::hasTypeDB(QSqlDatabase &db)
{
   SqliteQuery query(db);
    bool hasType = false;
    QString strSql = "select count(1) from sqlite_master where type='table' and "
                     "tbl_name = 'JobType'";
    if (query.exec(strSql) && query.next()) {
        //获取是否存在为农历标识字段，若存在则返回1,不存在则返回0
        hasType = query.value(0).toInt();
    }
    if (query.isActive()) {
        query.finish();
    }
    return hasType;
}

bool DDataBaseManagement::hasRemindDB(QSqlDatabase &db)
{
   SqliteQuery query(db);
    bool hasRemind = false;
    QString strSql = "select count(1) from sqlite_master where type='table' and "
                     "tbl_name = 'jobsReminder'";
    if (query.exec(strSql) && query.next()) {
        //获取是否存在为农历标识字段，若存在则返回1,不存在则返回0
        hasRemind = query.value(0).toInt();
    }
    if (query.isActive()) {
        query.finish();
    }
    return hasRemind;
}

DScheduleType::List DDataBaseManagement::queryOldJobTypeData(QSqlDatabase &db)
{
   SqliteQuery query(db);
    QString strSql("SELECT TypeNo, TypeName, ColorTypeNo, CreateTime, Authority                 \
                   FROM JobType where Authority >0;");
    DScheduleType::List typeList;
    query.prepare(strSql);
    if (query.exec()) {
        while (query.next()) {
            DScheduleType::Ptr type = DScheduleType::Ptr(new DScheduleType);
            type->setTypeID(QString::number(query.value("TypeNo").toInt()));
            type->setTypeName(query.value("TypeName").toString());
            type->setDisplayName(query.value("TypeName").toString());
            type->setDtCreate(query.value("CreateTime").toDateTime());
            type->setPrivilege(static_cast<DScheduleType::Privilege>(query.value("Authority").toInt()));
            type->setColorID(query.value("ColorTypeNo").toInt());
            typeList.append(type);
        }
    }
    return typeList;
}

DSchedule::List DDataBaseManagement::queryOldJobData(QSqlDatabase &db, const bool haslunar)
{
   SqliteQuery query(db);
    QString strSql;
    if (haslunar) {
        strSql = "SELECT id, created_at, \"type\", title, description, all_day,                     \
                \"start\", \"end\", r_rule, remind, \"ignore\", title_pinyin, is_Lunar          \
               FROM jobs;";
    } else {
        strSql = "SELECT id, created_at, \"type\", title, description, all_day,                     \
                \"start\", \"end\", r_rule, remind, \"ignore\", title_pinyin                    \
               FROM jobs;";
    }
    DSchedule::List scheduleList;
    query.prepare(strSql);
    if (query.exec()) {
        while (query.next()) {
            DSchedule::Ptr schedule = DSchedule::Ptr(new DSchedule);
            schedule->setUid(QString::number(query.value("id").toInt()));
            schedule->setDtStart(query.value("start").toDateTime());
            schedule->setCreated(query.value("created_at").toDateTime());
            schedule->setSummary(query.value("title").toString());
            schedule->setAllDay(query.value("all_day").toBool());
            schedule->setDtEnd(query.value("end").toDateTime());
            QString rrule = query.value("r_rule").toString();
            if (!rrule.isEmpty()) {
                //重复规则
                KCalendarCore::Recurrence *recurrence = schedule->recurrence();
                KCalendarCore::ICalFormat ical;
                KCalendarCore::RecurrenceRule *rule = new KCalendarCore::RecurrenceRule;

                if (ical.fromString(rule, query.value("r_rule").toString())) {
                    recurrence->addRRule(rule);
                }

                //添加忽略列表
                QJsonArray ignore = query.value("Ignore").toJsonArray();
                foreach (auto ignoreTime, ignore) {
                    if (schedule->allDay()) {
                        recurrence->addExDate(dtFromString(ignoreTime.toString()).date());
                    } else {
                        recurrence->addExDateTime(dtFromString(ignoreTime.toString()));
                    }
                }
            }
            QString remind = query.value("remind").toString();
            if (!remind.isEmpty()) {
                //提醒规则
                QStringList strList = remind.split(";", QString::SkipEmptyParts);

                int remindNum = strList.at(0).toInt();
                //小于0表示不提醒
                if (remindNum >= 0) {
                    KCalendarCore::Alarm::Ptr alarm = KCalendarCore::Alarm::Ptr(new KCalendarCore::Alarm(schedule.data()));
                    alarm->setEnabled(true);
                    alarm->setType(KCalendarCore::Alarm::Display);
                    alarm->setDisplayAlarm(schedule->summary());

                    if (schedule->allDay()) {
                        //提前多少秒
                        int offset = 0;
                        if (strList.size() > 1) {
                            QTime time = QTime::fromString(strList.at(1), "hh:mm");
                            offset = time.hour() * 60 * 60 + time.second() * 60;
                        }
                        KCalendarCore::Duration duration(-(24 * 60 * 60 * remindNum - offset));
                        alarm->setStartOffset(duration);
                    } else {
                        KCalendarCore::Duration duration(-(60 * remindNum));
                        alarm->setStartOffset(duration);
                    }
                    schedule->addAlarm(alarm);
                }
            }
            int type = query.value("type").toInt();
            if (m_typeMap.contains(type)) {
                schedule->setScheduleTypeID(m_typeMap[type]);
            } else {
                qWarning() << " can not find type:" << type;
            }
            scheduleList.append(schedule);
        }
    }

    if (query.isActive()) {
        query.finish();
    }
    return scheduleList;
}

QVector<DTypeColor> DDataBaseManagement::queryOldTypeColorData(QSqlDatabase &db)
{
   SqliteQuery query(db);
    QVector<DTypeColor> colorVector;
    //获取用户创建颜色
    QString strSql("SELECT TypeNo, ColorHex, Authority              \
                   FROM ColorType WHERE ID  >9;");
    query.prepare(strSql);
    if (query.exec()) {
        while (query.next()) {
            DTypeColor color;
            color.setColorID(query.value("TypeNo").toInt());
            color.setColorCode(query.value("ColorHex").toString());
            color.setPrivilege(DTypeColor::PriUser);
            colorVector.append(color);
        }
    }
    if (query.isActive()) {
        query.finish();
    }
    return colorVector;
}

DRemindData::List DDataBaseManagement::querOldRemindData(QSqlDatabase &db)
{
   SqliteQuery query(db);
    DRemindData::List remindList;
    QString strSql("SELECT jobid, recurid, remindCount, notifyid, remindTime, jobStartTime, jobEndTime FROM jobsReminder;");
    query.prepare(strSql);
    if (query.exec()) {
        while (query.next()) {
            int jobid = query.value("jobid").toInt();
            if (m_schedule.contains(jobid)) {
                DRemindData::Ptr remind = DRemindData::Ptr(new DRemindData);
                remind->setScheduleID(m_schedule[jobid]);
                remind->setRemindCount(query.value("remindCount").toInt());
                remind->setNotifyid(query.value("notifyid").toInt());
                remind->setDtRemind(query.value("remindTime").toDateTime());
                remind->setDtStart(query.value("jobStartTime").toDateTime());
                remind->setDtEnd(query.value("jobEndTime").toDateTime());

                int recurid = query.value("recurid").toInt();
                //如果重复id大于0，则表示为重复日程的提醒，设置提醒id为日程开始时间
                if (recurid > 0) {
                    remind->setRecurrenceId(remind->dtStart());
                }
                remindList.append(remind);
            }
        }
    }
    if (query.isActive()) {
        query.finish();
    }
    return remindList;
}
