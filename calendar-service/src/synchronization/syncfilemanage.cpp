// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncfilemanage.h"
#include "commondef.h"

#include <DSysInfo>
#include "units.h"

Q_LOGGING_CATEGORY(SyncFileLog, "calendar.sync.file")

SyncFileManage::SyncFileManage(QObject *parent)
    : QObject(parent)
    , m_syncoperation(new Syncoperation)
    , m_account(new DAccount(DAccount::Account_UnionID))
{
    qCDebug(SyncFileLog) << "Initializing sync file manager";
}

SyncFileManage::~SyncFileManage()
{
    qCDebug(SyncFileLog) << "Destroying sync file manager";
}

bool SyncFileManage::SyncDataDownload(const QString &uid, QString &filepath, int &errorcode)
{
    qCDebug(SyncFileLog) << "Starting sync data download for user:" << uid;
    //文件下载目录检查
    QString usersyncdir(getDBPath() +QString("/%1_calendar").arg(uid));
    UserSyncDirectory(usersyncdir);
    QString syncDB = usersyncdir + "/" + syncDBname;
    QFile syncDBfile(syncDB);
    if (syncDBfile.exists()) {
        //存在文件即删除
        qCDebug(SyncFileLog) << "Removing existing sync database file:" << syncDB;
        syncDBfile.remove();
    }

    SyncoptResult result;
    result = m_syncoperation->optDownload(syncDB, syncDB);
    if (result.error_code == SYNC_No_Error) {
        //下载成功
        qCInfo(SyncFileLog) << "Successfully downloaded sync database file";
        if (result.data != syncDB) {
            //文件下载路径不正确
            //将文件移动到正确路径
            qCWarning(SyncFileLog) << "Download path mismatch, moving file from" << result.data << "to" << syncDB;
            if (!QFile::rename(result.data, syncDB)) {
                qCWarning(ServiceLogger) << "Failed to move downloaded file to correct path";
                errorcode = -1;
                return false;
            }
        }
        filepath = syncDB;
        return true;
    } else if (result.error_code == SYNC_Data_Not_Exist) {
        //云同步数据库文件不存在
        qCInfo(SyncFileLog) << "Sync database does not exist, creating new one";
        if (SyncDbCreate(syncDB)) {
            filepath = syncDB;
            return true;
        } else {
            qCWarning(ServiceLogger) << "Failed to create new sync database";
            errorcode = -1;
            return false;
        }
    }
    qCWarning(SyncFileLog) << "Sync data download failed with error code:" << result.error_code;
    errorcode = result.error_code;
    return false;
}

bool SyncFileManage::SyncDbCreate(const QString &DBpath)
{
    qCDebug(SyncFileLog) << "Creating new sync database at:" << DBpath;
    QFile file(DBpath);
    if (!file.exists()) {
        bool bRet = file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append);
        if (!bRet) {
            qCWarning(ServiceLogger) << "Failed to create sync database file";
            return false;
        }
        file.close();
    }

    QSqlDatabase m_db;
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setPassword(syncDBpassword);
    m_db.setDatabaseName(DBpath);
    if (!m_db.open()) {
        qCWarning(ServiceLogger) << "Failed to open sync database";
        return false;
    }
    qCInfo(ServiceLogger) << "Successfully created and opened sync database";
    m_db.close();
    return true;
}

bool SyncFileManage::SyncDbDelete(const QString &DBpath)
{
    if (DBpath.isEmpty()) {
        qCWarning(ServiceLogger) << "DBpath isEmpty";
        return false;
    }
    QFileInfo fileinfo(DBpath);
    QDir dir = fileinfo.dir();

    if (dir.exists()) {
        if (!dir.removeRecursively()) {
            return false;
        }
    }

    return true;
}

bool SyncFileManage::SyncDataUpload(const QString &filepath, int &errorcode)
{
    qCDebug(SyncFileLog) << "Starting sync data upload for file:" << filepath;
    SyncoptResult result;
    result = m_syncoperation->optUpload(filepath);
    errorcode = result.error_code;
    if (result.error_code != SYNC_No_Error) {
        qCWarning(ServiceLogger) << "Sync data upload failed with error code:" << result.error_code;
        return false;
    }
    qCInfo(SyncFileLog) << "Successfully uploaded sync data";
    return true;
}

bool SyncFileManage::syncDataDelete(const QString &filepath)
{
    qCDebug(SyncFileLog) << "Attempting to delete sync data file:" << filepath;
    if (!SyncDbDelete(filepath)) {
        qCWarning(ServiceLogger) << "Failed to delete sync data file:" << filepath;
        return false;
    }
    qCInfo(SyncFileLog) << "Successfully deleted sync data file";
    return true;
}

DAccount::Ptr SyncFileManage::getuserInfo()
{
    QVariantMap userInfoMap;

    if (!m_syncoperation->optUserData(userInfoMap)) {
        qCInfo(ServiceLogger) << "can't get userinfo";
        return nullptr;
    }

    m_account->setDisplayName(userInfoMap.value("username").toString());
    m_account->setAccountID(userInfoMap.value("uid").toString());
    m_account->setAvatar(userInfoMap.value("profile_image").toString());
    m_account->setAccountName(userInfoMap.value("nickname").toString());
    return m_account;
}


Syncoperation *SyncFileManage::getSyncoperation()
{
    return m_syncoperation;
}

void SyncFileManage::UserSyncDirectory(const QString &dir)
{
    QDir udir(dir);
    if (!udir.exists()) {
        udir.mkdir(dir);
    }
}
