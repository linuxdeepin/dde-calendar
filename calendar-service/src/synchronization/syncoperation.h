#ifndef SYNCOPERATION_H
#define SYNCOPERATION_H

#include <QtDBus>
#include <QtDBus/qtdbusglobal.h>
#include <QtDBus/qdbusabstractinterface.h>
#include <QtDBus/qdbusconnection.h>
#include "dunioniddbus.h"

//云同步DBUS接口信息
#define SYNC_DBUS_PATH              "com.deepin.sync.Daemon"
#define SYNC_DBUS_INTERFACE         "/com/deepin/utcloud/Daemon"


//云同步错误码
#define SYNC_No_Error                   0               /**/
#define SYNC_Internal_Error             7500            /**/
#define SYNC_Parameter_Error            7501            /**/
#define SYNC_Login_Expired              7502            /**/
#define SYNC_No_Access                  7503
#define SYNC_Data_Not_Exist             7504
#define SYNC_File_Operation_Failed      7505
#define SYNC_Network_Request_Error      7506
#define SYNC_Oss_Operation_Error        7507
#define SYNC_Space_Not_Available        7508
#define SYNC_File_Or_Path_Error         7509
#define SYNC_Invalid_File_Size          7510
#define SYNC_Metadata_Check_Error       7511

using SyncInter = com::deepin::sync::cloudopt;

struct SyncoptResult {
    QString data = "";  //执行结果数据
    bool ret = false;   //执行云同步操作的结果
    int error_code = 0; //错误码
};

class Syncoperation : public QObject
{
    Q_OBJECT
public:
    explicit Syncoperation(QObject *parent = nullptr);
    ~Syncoperation();

    void optlogin();

    void optlogout();

    SyncoptResult optUpload(const QString &key);

    SyncoptResult optDownload(const QString &key, const QString &path);

    SyncoptResult optDelete(const QString &key);

    SyncoptResult optMetadata(const QString &key);

    bool optUserData(QVariantMap &userInfoMap);

Q_SIGNALS:
    void UserDatachanged(const QVariantMap  &value) const;
    void LoginStatuschanged(const int32_t value) const;
    void signalLoginStatusChange(const bool staus);

private slots:
    void slotDbusCall(const QDBusMessage &msg);

private:
    DAccount::Ptr accountChangeHandle(const QDBusArgument &accountInfo);

private Q_SLOTS:
    void OnLoginStatus(const int32_t value);
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties);
private:
    SyncInter *m_syncInter;
};

#endif // SYNCUPLOAD_H
