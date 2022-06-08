#include "syncfilemanage.h"


SyncFileManage::SyncFileManage(QObject *parent)
    : QObject(parent)
    , m_syncoperation(new Syncoperation)
    , m_account(new DAccount(DAccount::Account_UnionID))
{
    connect(m_syncoperation, &Syncoperation::UserDatachanged, this, &SyncFileManage::onUserDatachanged);
    connect(m_syncoperation, &Syncoperation::LoginStatuschanged, this, &SyncFileManage::onLoginStatuschanged);
    //需要先调用一次用户登入
    m_syncoperation->optlogin();
}

SyncFileManage::~SyncFileManage()
{

}

int SyncFileManage::SyncDataDownload(QString &UID, QString &filename)
{
    //文件下载目录检查
    QString usersyncdir(QString("/tmp/%1_calendar").arg(UID));
    UserSyncDirectory(usersyncdir);
    QString syncDB = usersyncdir + "/" + syncDBname;

    SyncoptResult result;
    result = m_syncoperation->optDownload(syncDB, syncDB);
    if (result.error_code == SYNC_No_Error) {
        //下载成功
        if (result.data != syncDB) {
            //文件下载路径不正确
            qDebug() << "down path error!";
        }
        filename = result.data;
    } else if (result.error_code == SYNC_Data_Not_Exist) {
        //创建原始云同步数据库
        if (SyncDbCreate(syncDB)) {
            filename = syncDB;
        } else {
            return -1;
        }
    }

    return result.error_code;
}

bool SyncFileManage::SyncDbCreate(QString &DBpath)
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
    if (!m_db.open()) {
        qInfo() << "db open failed";
        return false;
    }
    qInfo() << "db open successed";
    m_db.close();
    return true;
}

int SyncFileManage::SyncDataUpload(const QString &filenanme)
{
    SyncoptResult result;
    result = m_syncoperation->optUpload(filenanme);

    return result.error_code;
}

DAccount::Ptr SyncFileManage::getuserInfo()
{
    QVariantMap userInfoMap;

    if (!m_syncoperation->optUserData(userInfoMap)) {
        qDebug() << "can't get userinfo";
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

void SyncFileManage::onUserDatachanged(const QVariantMap  &value)
{
    qInfo() << value.value("username").toString();
    qInfo() << value.value("profile_image").toString();
    qInfo() << value.value("nickname").toString();
    qInfo() << value.value("uid").toString();
    // m_userinfo = value; 这边是否直接更新？？？
}

void SyncFileManage::onLoginStatuschanged(const int32_t value)
{
    qInfo() << value;
}

void SyncFileManage::UserSyncDirectory(const QString &dir)
{
    QDir udir(dir);
    if (!udir.exists()) {
        udir.mkdir(dir);
    }
}
