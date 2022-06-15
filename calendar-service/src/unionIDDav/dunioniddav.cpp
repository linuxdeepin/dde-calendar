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
#include "dunioniddav.h"

#include "dcalendargeneralsettings.h"
#include "syncfilemanage.h"
#include "units.h"
#include "ddatabase.h"
#include "dunioniddav_p.h"

#include <QDBusConnection>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QThread>
#include <QMutex>

#include <unistd.h>

/**
 * @brief The SqlTransactionLocker class 用于数据库事务相关
 */

bool SyncAccount::defaultScheduleType()
{
    //工作类型
    DScheduleType::Ptr workType(new DScheduleType(_accountID));
    workType->setTypeID(DDataBase::createUuid());
    workType->setDtCreate(QDateTime::currentDateTime());
    workType->setPrivilege(DScheduleType::Read);
    workType->setTypeName("Work");
    workType->setDisplayName("Work");
    DTypeColor workColor;
    workColor.setColorID(1);
    workColor.setColorCode("#ff5e97");
    workColor.setPrivilege(DTypeColor::PriSystem);
    workType->setTypeColor(workColor);
    workType->setShowState(DScheduleType::Show);
    if(!insertToScheduleType(workType))
        return false;

    //生活
    DScheduleType::Ptr lifeType(new DScheduleType(_accountID));
    lifeType->setTypeID(DDataBase::createUuid());
    lifeType->setDtCreate(QDateTime::currentDateTime());
    lifeType->setPrivilege(DScheduleType::Read);
    lifeType->setTypeName("Life");
    lifeType->setDisplayName("Life");
    DTypeColor lifeColor;
    lifeColor.setColorID(7);
    lifeColor.setColorCode("#5d51ff");
    lifeColor.setPrivilege(DTypeColor::PriSystem);
    lifeType->setTypeColor(lifeColor);
    lifeType->setShowState(DScheduleType::Show);
    if(!insertToScheduleType(lifeType))
        return false;

    //其他
    DScheduleType::Ptr otherType(new DScheduleType(_accountID));
    otherType->setTypeID(DDataBase::createUuid());
    otherType->setDtCreate(QDateTime::currentDateTime());
    otherType->setPrivilege(DScheduleType::Read);
    otherType->setTypeName("Other");
    otherType->setDisplayName("Other");
    DTypeColor otherColor;
    otherColor.setColorID(4);
    otherColor.setColorCode("#5bdd80");
    otherColor.setPrivilege(DTypeColor::PriSystem);
    otherType->setTypeColor(otherColor);
    otherType->setShowState(DScheduleType::Show);
    if(!insertToScheduleType(otherType))
        return false;

    return true;
}

bool SyncAccount::defaultTypeColor()
{
    if(!insertToTypeColor(1, "#ff5e97", 1)) return false;
    if(!insertToTypeColor(2, "#ff9436", 1)) return false;
    if(!insertToTypeColor(3, "#ffdc00", 1)) return false;
    if(!insertToTypeColor(4, "#5bdd80", 1)) return false;
    if(!insertToTypeColor(5, "#00b99b", 1)) return false;
    if(!insertToTypeColor(6, "#4293ff", 1)) return false;
    if(!insertToTypeColor(7, "#5d51ff", 1)) return false;
    if(!insertToTypeColor(8, "#a950ff", 1)) return false;
    if(!insertToTypeColor(9, "#717171", 1)) return false;
    return true;
}

bool SyncAccount::insertToScheduleType(const DScheduleType::Ptr &scheduleType)
{
    QString strSql("INSERT INTO scheduleType (                      \
                   typeID, typeName, typeDisplayName, typePath,     \
                   typeColorID, description, privilege, showState,  \
                   syncTag,dtCreate,isDeleted)                      \
            VALUES(?,?,?,?,?,?,?,?,?,?,?)");
    QSqlQuery query(QSqlDatabase::database(_connectionName));
    query.prepare(strSql);
    if (scheduleType->typeID().size() < 30) {
        scheduleType->setTypeID(DDataBase::createUuid());
    }

    query.addBindValue(scheduleType->typeID());
    query.addBindValue(scheduleType->typeName());
    query.addBindValue(scheduleType->displayName());
    query.addBindValue(scheduleType->typePath());
    query.addBindValue(scheduleType->typeColor().colorID());
    query.addBindValue(scheduleType->description());
    query.addBindValue(int(scheduleType->privilege()));
    query.addBindValue(scheduleType->showState());
    query.addBindValue(scheduleType->syncTag());
    query.addBindValue(dtToString(scheduleType->dtCreate()));
    query.addBindValue(scheduleType->deleted());

    return query.exec();
}

bool SyncAccount::insertToTypeColor(int typeColorNo, QString strColorHex, int privilege)
{
    QSqlQuery query(QSqlDatabase::database(_connectionName));
    query.prepare("replace into TypeColor(ColorID, ColorHex, privilege) VALUES(?, ?, ?)");
    query.addBindValue(typeColorNo);
    query.addBindValue(strColorHex);
    query.addBindValue(privilege);
    return query.exec();
}

void DUIDSynDataWorker::syncData(SyncStack syncType)
{
    if(nullptr == mSyncTimer) {
        mSyncTimer = new QTimer(this);
        mSyncTimer->setSingleShot(true);
        connect(mSyncTimer, &QTimer::timeout, this, &DUIDSynDataWorker::slotSync);
    }
    mSyncTimer->start(100);

    mSyncMutex.lock();
    mSyncList.append(syncType);
    mSyncMutex.unlock();
}

void DUIDSynDataWorker::slotSync()
{
    bool next = false;
    mSyncMutex.lock();
    for(int k = mSyncList.count() - 1; k >= 0; k --) {
        if(k == mSyncList.count() - 1) {
            mSync = mSyncList[k];
            next = true;
        } else {
            mSync.syncType |= mSyncList[k].syncType;
        }
        if(mSync.syncType.testFlag(DDataSyncBase::Sync_Upload)
                && mSync.syncType.testFlag(DDataSyncBase::Sync_Download))
            break;
    }

    mSyncList.clear();
    mSyncMutex.unlock();

    if(next)
        startUpdate();

}

void DUIDSynDataWorker::startUpdate()
{
    /*
     * A为本地数据库
     * B为云端下载的临时数据库
     * 1.下载B
     * 2.将A的uploadTask同步到B
     * 3.将B的数据库完全复制到A
     * 4.上传B
     */
    QMutexLocker locker(&mSyncMutex);
    SyncFileManage fileManger;
    int errCode = 0;
    DUnionIDDav::UpdateTypes updateType = DUnionIDDav::Update_None;

    QSqlDatabase::cloneDatabase(mSync.db_account, mSync.dbname_account_thread);
    QSqlDatabase::cloneDatabase(mSync.db_manager, mSync.dbname_manager_thread);
    //获取云端数据
    bool isInitSyncData = false;
    errCode = mSync.downloadUidData(isInitSyncData, fileManger);

    SqlTransactionLocker transactionLocker({mSync.dbname_account_thread, mSync.dbname_manager_thread, mSync.dbname_sync_thread});
    //本地数据迁移到临时文件中
    if (errCode == 0) {
        errCode =  mSync.loadToTmp();
    }

    //将临时文件数据迁移到本地中
    if (errCode == 0 && mSync.syncType.testFlag(DDataSyncBase::SyncType::Sync_Download)) {
        errCode =  mSync.tmpToLoad(updateType);
    }
    if(errCode == 0) {
        transactionLocker.commit();
    }
    QSqlDatabase::removeDatabase(mSync.dbname_account_thread);
    QSqlDatabase::removeDatabase(mSync.dbname_manager_thread);
    QSqlDatabase::removeDatabase(mSync.dbname_sync_thread);

    //如果为上传需要上传数据
    if (errCode == 0 && (isInitSyncData || mSync.syncType.testFlag(DDataSyncBase::SyncType::Sync_Upload))) {
        errCode = mSync.uploadTmpData(fileManger);
    }
    //删除临时文件
     mSync.deleteTmpData(fileManger);

    qInfo() << "同步完成";
    if(errCode == 0) {
        //发送数据更新消息
        emit signalUpdate(updateType);
    }
    //发送同步状态
    emit signalSyncState(errCode);
}

DUnionIDDav::DUnionIDDav()
    : DDataSyncBase()
    , d(new DUIDSynDataPrivate())
{
    connect(d, &DUIDSynDataPrivate::signalUpdate, this, &DUnionIDDav::signalUpdate);
    connect(d, &DUIDSynDataPrivate::signalSyncState, this, &DUnionIDDav::signalSyncState);
}

DUnionIDDav::~DUnionIDDav()
{
    delete d;
}

void DUnionIDDav::syncData(QString accountId, QString accountName, int accountState, DDataSyncBase::SyncTypes syncType)
{
    qInfo() << "DUnionIDDav 同步数据";

    d->syncData(accountId, accountName, accountState, syncType);
}


int SyncStack::downloadUidData(bool &isInitSyncData, SyncFileManage &fileManger)
{
    int errCode = 0;
    isInitSyncData = false;
    //下载数据库sync
    if (!fileManger.SyncDataDownload(accountId, dbpath_sync, errCode)) {
        return errCode;
    }

    QSqlDatabase db_sync = QSqlDatabase::addDatabase("QSQLITE", dbname_sync_thread);
    db_sync.setDatabaseName(dbpath_sync);
    if (!db_sync.open()) {
        return -1;
    }
    qInfo() << "初始化表结构";
    QSqlQuery query(QSqlDatabase::database(dbname_sync_thread));

    if(!query.exec(DAccountDataBase::sql_create_schedules))
        return -1;
    if(!query.exec(DAccountDataBase::sql_create_scheduleType))
        return -1;
    if(!query.exec(DAccountDataBase::sql_create_typeColor))
        return -1;
    if(!query.exec(DAccountDataBase::sql_create_calendargeneralsettings))
        return -1;

    query.exec("select count(0) from scheduleType");
    if (query.next() && query.value(0).toInt() == 0) {
        qInfo() << "没有数据则设置：默认日程类型、颜色、默认通用设置";
        SyncAccount accountDb(dbname_sync_thread, accountId);

        qInfo() << "默认日程类型";
        if(!accountDb.defaultScheduleType())
            return -1;

        qInfo() << "默认颜色";
        if(!accountDb.defaultTypeColor())
            return -1;

        if (accountState & DAccount::Account_Setting) {
            qInfo() << "默认通用设置";
            if(!syncIntoTable("calendargeneralsettings", dbname_manager_thread, dbname_sync_thread))
                return -1;
        }
        //云端没有对应数据需要初始化，返回初始化状态
        isInitSyncData = true;
    }

    return 0;
}

int SyncStack::loadToTmp()
{
    qInfo() << "将本地A的uploadTask同步到刚刚下载的B里";
    QSqlQuery query(QSqlDatabase::database(dbname_account_thread));
    query.exec("select taskID,uploadType,uploadObject,objectID  from uploadTask");
    while (query.next()) {
        int type = query.value("uploadType").toInt();
        int obj = query.value("uploadObject").toInt();
        QString key_value = query.value("objectID").toString();
        QString key_name = DUploadTaskData::sql_table_primary_key(obj);
        QString table_name = DUploadTaskData::sql_table_name(obj);

        switch (type) {
        case DUploadTaskData::Create:
        case DUploadTaskData::Modify:
            if(!replaceIntoRecord(table_name, selectRecord(table_name, key_name, key_value, dbname_account_thread), dbname_sync_thread))
                return -1;
            break;
        case DUploadTaskData::Delete:
            if(!deleteTableLine(table_name, key_name, key_value, dbname_sync_thread))
                return -1;
            break;
        }
    }

    qInfo() << "清空uploadTask";
    if(!deleteTable("uploadTask", dbname_account_thread))
        return -1;

    if (accountState & DAccount::Account_Setting) {
        qInfo() << "同步通用设置";
        QDateTime managerDate = selectValue("vch_value", "calendargeneralsettings", "vch_key", "dt_update", dbname_manager_thread).toDateTime();
        QDateTime syncDate = selectValue("vch_value", "calendargeneralsettings", "vch_key", "dt_update", dbname_sync_thread).toDateTime();
        if (managerDate > syncDate)
            if(!syncIntoTable("calendargeneralsettings", dbname_manager_thread, dbname_sync_thread))
                return -1;
        if (syncDate > managerDate)
            if(!syncIntoTable("calendargeneralsettings", dbname_sync_thread, dbname_manager_thread))
                return -1;
    }

    return 0;
}

int SyncStack::tmpToLoad(DDataSyncBase::UpdateTypes updateType)
{
    qInfo() << "更新schedules、schedules、typeColor";
    if(!syncIntoTable("schedules", dbname_sync_thread, dbname_account_thread))
        return -1;
    if(!syncIntoTable("scheduleType", dbname_sync_thread, dbname_account_thread))
        return -1;
    if(!syncIntoTable("typeColor", dbname_sync_thread, dbname_account_thread))
        return -1;
    updateType = DUnionIDDav::Update_Schedule | DUnionIDDav::Update_ScheduleType;

    if (accountState & DAccount::Account_Setting) {
        qInfo() << "更新通用设置";
        if(!syncIntoTable("calendargeneralsettings", dbname_sync_thread, dbname_manager_thread))
            return -1;
        updateType = updateType | DUnionIDDav::Update_Setting;
    }

    return 0;
}

int SyncStack::uploadTmpData(SyncFileManage &fileManger)
{
    int errCode = 0;
    fileManger.SyncDataUpload(dbpath_sync, errCode);
    return errCode;
}

void SyncStack::deleteTmpData(SyncFileManage &fileManger)
{
    fileManger.syncDataDelete(dbpath_sync);
}

QString SyncStack::prepareQuest(int count)
{
    QString r;
    while (count--) {
        r += "?,";
    }
    r.chop(1);
    return r;
}

void SyncStack::prepareBinds(QSqlQuery &query, QSqlRecord source)
{
    for (int k = 0; k < source.count(); k++)
        query.addBindValue(source.value(k));
}

bool SyncStack::syncIntoTable(const QString &table_name, const QString &connection_name_source, const QString &connection_name_target)
{
    QSqlQuery source(QSqlDatabase::database(connection_name_source));
    QSqlQuery target(QSqlDatabase::database(connection_name_target));
    if(!target.exec(" delete from " + table_name))
        return false;
    source.exec(" select * from " + table_name);
    while (source.next()) {
        target.prepare("replace into " + table_name + " values(" + prepareQuest(source.record().count()) + ")");
        prepareBinds(target, source.record());
        if(!target.exec()) {
            qInfo() << target.lastError();
            return false;
        }
    }
    return true;
}

QSqlRecord SyncStack::selectRecord(const QString &table_name, const QString &key_name, const QVariant &key_value, const QString &connection_name)
{
    QSqlQuery query(QSqlDatabase::database(connection_name));
    query.prepare("select * from " + table_name + " where " + key_name + " = ?");
    query.addBindValue(key_value);
    query.exec();
    if(query.next())
        return query.record();
    return QSqlRecord();
}

bool SyncStack::replaceIntoRecord(const QString &table_name, QSqlRecord record, const QString &connection_name)
{
    if (record.isEmpty())
        return true;
    QSqlQuery query(QSqlDatabase::database(connection_name));
    query.prepare("replace into " + table_name + " values(" + prepareQuest(record.count()) + ")");
    prepareBinds(query, record);
    return query.exec();
}

QVariant SyncStack::selectValue(const QString &value_name, const QString &table_name, const QString &key_name, const QVariant &key_value, const QString &connection_name)
{
    QSqlQuery query(QSqlDatabase::database(connection_name));
    query.prepare("select " + value_name + " from " + table_name + " where " + key_name + " = ?");
    query.addBindValue(key_value);
    query.exec();
    query.next();
    return query.value(0);
}

bool SyncStack::deleteTableLine(const QString &table_name, const QString &key_name, const QVariant &key_value, const QString &connection_name)
{
    QSqlQuery query(QSqlDatabase::database(connection_name));
    query.prepare("delete from " + table_name + " where " + key_name + " = ?");
    query.addBindValue(key_value);
    return query.exec();
}

bool SyncStack::deleteTable(const QString &table_name, const QString &connection_name)
{
    QSqlQuery query(QSqlDatabase::database(connection_name));
    if(!query.exec("delete from " + table_name)) {
        qInfo() << query.lastError();
        return false;
    }
    return true;
}


DUnionIDDav::DUIDSynDataPrivate::DUIDSynDataPrivate()
{
    mThread = new QThread(this);
    DUIDSynDataWorker *mWorker = new DUIDSynDataWorker;
    qRegisterMetaType<SyncStack>("SyncStack");
    mWorker->moveToThread(mThread);
    connect(mThread, &QThread::finished, mWorker, &DUIDSynDataWorker::deleteLater);
    connect(mThread, &QThread::finished, mThread, &QThread::deleteLater);

    connect(mWorker, &DUIDSynDataWorker::signalUpdate, this, &DUIDSynDataPrivate::signalUpdate);
    connect(mWorker, &DUIDSynDataWorker::signalSyncState, this, &DUIDSynDataPrivate::signalSyncState);
    connect(this, &DUIDSynDataPrivate::signalSyncData, mWorker, &DUIDSynDataWorker::syncData);

    mThread->start();
}

DUnionIDDav::DUIDSynDataPrivate::~DUIDSynDataPrivate()
{
    if (mThread->isRunning()) {
        mThread->quit();
        mThread->wait();
    }
}

void DUnionIDDav::DUIDSynDataPrivate::syncData(QString accountId, QString accountName, int accountState, DDataSyncBase::SyncTypes syncType)    {
    emit signalSyncData(SyncStack{accountId, accountName, accountState, syncType});
}
