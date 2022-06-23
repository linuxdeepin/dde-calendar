#include "syncfilemanage.h"

#include <DSysInfo>

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
    //文件下载目录检查
    QString usersyncdir(QString("/tmp/%1_calendar").arg(uid));
    UserSyncDirectory(usersyncdir);
    QString syncDB = usersyncdir + "/" + syncDBname;
    QFile syncDBfile(syncDB);
    if (syncDBfile.exists()) {
        //存在文件即删除
        syncDBfile.remove();
    }

    SyncoptResult result;
    result = m_syncoperation->optDownload(syncDB, syncDB);
    if (result.error_code == SYNC_No_Error) {
        //下载成功
        if (result.data != syncDB) {
            //文件下载路径不正确
            //将文件移动到正确路径
            if (!QFile::rename(result.data, syncDB)) {
                qDebug() << "down path error!";
                errorcode = -1;
                return false;
            }
        }
        filepath = syncDB;
        return true;
    } else if (result.error_code == SYNC_Data_Not_Exist) {
        //云同步数据库文件不存在
        if (SyncDbCreate(syncDB)) {
            filepath = syncDB;
            return true;
        } else {
            errorcode = -1;
            return false;
        }
    }
    errorcode = result.error_code;
    return false;
}

bool SyncFileManage::SyncDbCreate(const QString &DBpath)
{
    QFile file(DBpath);
    if (!file.exists()) {
        bool bRet = file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append);
        if (!bRet) {
            qDebug() << "file creat failed";
            return false;
        }
        file.close();
    }

    QSqlDatabase m_db;
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setPassword(syncDBpassword);
    m_db.setDatabaseName(DBpath);
    if (!m_db.open()) {
        qInfo() << "db open failed";
        return false;
    }
    qInfo() << "db open successed";
    m_db.close();
    return true;
}

bool SyncFileManage::SyncDbDelete(const QString &DBpath)
{
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
    SyncoptResult result;
    result = m_syncoperation->optUpload(filepath);
    errorcode = result.error_code;
    if (result.error_code != SYNC_No_Error) {
        qWarning() << "upload failed";
        return false;
    }
    return true;
}

bool SyncFileManage::syncDataDelete(const QString &filepath)
{
    if (!SyncDbDelete(filepath)) {
        qWarning() << "delete file error:" << filepath;
        return false;
    }
    return true;
}

DAccount::Ptr SyncFileManage::getuserInfo()
{
    QVariantMap userInfoMap;
    //如果为社区版本则返回空
    if (Dtk::Core::DSysInfo::uosEditionType() == Dtk::Core::DSysInfo::UosCommunity) {
        return nullptr;
    }

    if (!m_syncoperation->optUserData(userInfoMap)) {
        qInfo() << "can't get userinfo";
        return nullptr;
    }

    qInfo() << userInfoMap.value("username").toString();
    qInfo() << userInfoMap.value("profile_image").toString();
    qInfo() << userInfoMap.value("nickname").toString();
    qInfo() << userInfoMap.value("uid").toString();

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
