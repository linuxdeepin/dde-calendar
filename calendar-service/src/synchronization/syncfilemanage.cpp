// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncfilemanage.h"
#include "commondef.h"

#include <DSysInfo>
#include "units.h"

#include <QSqlError>

SyncFileManage::SyncFileManage(QObject *parent)
    : QObject(parent)
    , m_syncoperation(new Syncoperation)
    , m_account(new DAccount(DAccount::Account_UnionID))
{

}

SyncFileManage::~SyncFileManage()
{

}

bool SyncFileManage::SyncDataDownload(const QString &uid, QString &filepath, int &errorcode)
{
    qCDebug(ServiceLogger) << "Starting sync data download for user:" << uid;
    
    //文件下载目录检查
    QString usersyncdir(getDBPath() + QString("/%1_calendar").arg(uid));
    qCDebug(ServiceLogger) << "Checking user sync directory:" << usersyncdir;
    UserSyncDirectory(usersyncdir);
    
    QString syncDB = usersyncdir + "/" + syncDBname;
    QFile syncDBfile(syncDB);
    if (syncDBfile.exists()) {
        qCDebug(ServiceLogger) << "Removing existing sync DB file:" << syncDB;
        //存在文件即删除
        if (!syncDBfile.remove()) {
            qCWarning(ServiceLogger) << "Failed to remove existing sync DB file:" << syncDB;
        }
    }

    qCDebug(ServiceLogger) << "Downloading sync data to:" << syncDB;
    SyncoptResult result;
    result = m_syncoperation->optDownload(syncDB, syncDB);
    
    if (result.error_code == SYNC_No_Error) {
        qCInfo(ServiceLogger) << "Successfully downloaded sync data";
        //下载成功
        if (result.data != syncDB) {
            qCDebug(ServiceLogger) << "Downloaded file path differs from target path, moving file"
                                 << "\n  From:" << result.data
                                 << "\n  To:" << syncDB;
            //文件下载路径不正确
            //将文件移动到正确路径
            if (!QFile::rename(result.data, syncDB)) {
                qCWarning(ServiceLogger) << "Failed to move downloaded file to correct path"
                                       << "\n  From:" << result.data
                                       << "\n  To:" << syncDB;
                errorcode = -1;
                return false;
            }
        }
        filepath = syncDB;
        return true;
    } else if (result.error_code == SYNC_Data_Not_Exist) {
        qCInfo(ServiceLogger) << "Sync data does not exist, creating new database";
        //云同步数据库文件不存在
        if (SyncDbCreate(syncDB)) {
            qCInfo(ServiceLogger) << "Successfully created new sync database:" << syncDB;
            filepath = syncDB;
            return true;
        } else {
            qCWarning(ServiceLogger) << "Failed to create new sync database:" << syncDB;
            errorcode = -1;
            return false;
        }
    }
    
    qCWarning(ServiceLogger) << "Sync download failed with error code:" << result.error_code;
    errorcode = result.error_code;
    return false;
}

bool SyncFileManage::SyncDbCreate(const QString &DBpath)
{
    qCDebug(ServiceLogger) << "Creating sync database at:" << DBpath;
    
    QFile file(DBpath);
    if (!file.exists()) {
        qCDebug(ServiceLogger) << "Creating new database file";
        bool bRet = file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append);
        if (!bRet) {
            qCWarning(ServiceLogger) << "Failed to create database file:" << DBpath
                                   << "\n  Error:" << file.errorString();
            return false;
        }
        file.close();
    } else {
        qCDebug(ServiceLogger) << "Database file already exists:" << DBpath;
    }

    QSqlDatabase m_db;
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setPassword(syncDBpassword);
    m_db.setDatabaseName(DBpath);
    
    qCDebug(ServiceLogger) << "Opening database connection";
    if (!m_db.open()) {
        qCWarning(ServiceLogger) << "Failed to open database:" << DBpath
                               << "\n  Error:" << m_db.lastError().text();
        return false;
    }
    qCInfo(ServiceLogger) << "Successfully opened database:" << DBpath;
    m_db.close();
    return true;
}

bool SyncFileManage::SyncDbDelete(const QString &DBpath)
{
    qCDebug(ServiceLogger) << "Deleting sync database:" << DBpath;
    
    if (DBpath.isEmpty()) {
        qCWarning(ServiceLogger) << "Cannot delete database: path is empty";
        return false;
    }
    
    QFileInfo fileinfo(DBpath);
    QDir dir = fileinfo.dir();

    if (dir.exists()) {
        qCDebug(ServiceLogger) << "Removing directory recursively:" << dir.path();
        if (!dir.removeRecursively()) {
            qCWarning(ServiceLogger) << "Failed to remove directory:" << dir.path();
            return false;
        }
        qCInfo(ServiceLogger) << "Successfully removed directory:" << dir.path();
    } else {
        qCDebug(ServiceLogger) << "Directory does not exist:" << dir.path();
    }

    return true;
}

bool SyncFileManage::SyncDataUpload(const QString &filepath, int &errorcode)
{
    qCDebug(ServiceLogger) << "Uploading sync data from:" << filepath;
    
    SyncoptResult result;
    result = m_syncoperation->optUpload(filepath);
    errorcode = result.error_code;
    
    if (result.error_code != SYNC_No_Error) {
        qCWarning(ServiceLogger) << "Failed to upload sync data"
                               << "\n  File:" << filepath
                               << "\n  Error code:" << errorcode;
        return false;
    }
    
    qCInfo(ServiceLogger) << "Successfully uploaded sync data:" << filepath;
    return true;
}

bool SyncFileManage::syncDataDelete(const QString &filepath)
{
    qCDebug(ServiceLogger) << "Deleting sync data file:" << filepath;
    
    if (!SyncDbDelete(filepath)) {
        qCWarning(ServiceLogger) << "Failed to delete sync data file:" << filepath;
        return false;
    }
    
    qCInfo(ServiceLogger) << "Successfully deleted sync data file:" << filepath;
    return true;
}

DAccount::Ptr SyncFileManage::getuserInfo()
{
    QVariantMap userInfoMap;

    if (!m_syncoperation->optUserData(userInfoMap)) {
        qCWarning(ServiceLogger) << "Failed to get user information";
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
