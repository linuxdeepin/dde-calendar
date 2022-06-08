#include "syncoperation.h"

Syncoperation::Syncoperation(QObject *parent)
    : QObject(parent)
    , m_syncInter(new SyncInter(SYNC_DBUS_PATH, SYNC_DBUS_INTERFACE, QDBusConnection::sessionBus(), this))
{
    QStringList argumentMatch;
    argumentMatch << SYNC_DBUS_INTERFACE;
    //用户登录状态改变
    connect(m_syncInter, &SyncInter::LoginStatus, this, &Syncoperation::OnLoginStatus, Qt::QueuedConnection);
    //关联dbus propertiesChanged 属性改变信号
    QDBusConnection::sessionBus().connect(SYNC_DBUS_PATH,
                                          SYNC_DBUS_INTERFACE,
                                          "org.freedesktop.DBus.Properties",
                                          "PropertiesChanged",
                                          argumentMatch, QString(),
                                          this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
}

Syncoperation::~Syncoperation()
{

}

void Syncoperation::optlogin()
{
    //异步调用无需等待结果,由后续LoginStatus触发处理
    m_syncInter->login();
}

void Syncoperation::optlogout()
{
    //异步调用无需等待结果,由后续LoginStatus触发处理
    m_syncInter->logout();
}

SyncoptResult Syncoperation::optUpload(const QString &key)
{
    SyncoptResult result;
    QDBusPendingReply<QByteArray> reply = m_syncInter->Upload(key);
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        qInfo() << "Upload success!" << reply.value();
        //解析上传成功数据的元信息ID值 ,这个ID值暂时没用，不解析出来
//        QJsonObject json;
//        json = QJsonDocument::fromJson(reply).object();
//        if (json.contains(QString("data"))) {
//            QJsonObject subJsonObject = json.value(QString("data")).toObject();
//            if (subJsonObject.contains(QString("id"))) {
//                result.data = subJsonObject.value(QString("id")).toString();
//                qInfo() << result.data;
//            }
//        }
        result.data = reply.value();
        qInfo() << result.data;
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qWarning() << "Upload failed:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
            qInfo() << result.error_code;
        }
    }

    return result;
}

SyncoptResult Syncoperation::optDownload(const QString &key, const QString &path)
{
    SyncoptResult result;
    QDBusPendingReply<QString> reply = m_syncInter->Download(key, path);
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        qDebug() << "Download success!";
        result.data = reply.value();
        qInfo() << result.data;
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qWarning() << "Download failed:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
            qDebug() << result.error_code;
        }
    }

    return result;
}

SyncoptResult Syncoperation::optDelete(const QString &key)
{
    SyncoptResult result;
    QDBusPendingReply<QString> reply = m_syncInter->Delete(key);
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        qDebug() << "Delete success!";
        result.data = reply.value();
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qWarning() << "Delete failed:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
            qDebug() << result.error_code;
        }
    }

    return result;
}

SyncoptResult Syncoperation::optMetadata(const QString &key)
{
    SyncoptResult result;
    QDBusPendingReply<QString> reply = m_syncInter->Metadata(key);
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        qDebug() << "Metadata success!";
        //元数据获取接口，暂时好像用不到
        result.data = reply.value();
        qInfo() << result.data;
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qWarning() << "Metadata failed:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
            qDebug() << result.error_code;
        }
    }

    return result;
}

bool Syncoperation::optUserData(QVariantMap &userInfoMap)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(SYNC_DBUS_PATH,
                                                      SYNC_DBUS_INTERFACE,
                                                      "org.freedesktop.DBus.Properties",
                                                      QStringLiteral("Get"));
    msg << QString("com.deepin.utcloud.Daemon") << QString("UserData");
    QDBusMessage reply =  QDBusConnection::sessionBus().call(msg);

    if (reply.type() == QDBusMessage::ReplyMessage) {
        QVariant variant = reply.arguments().first();
        QDBusArgument argument = variant.value<QDBusVariant>().variant().value<QDBusArgument>();
        argument >> userInfoMap;
        return true;
    } else {
        qWarning() << "Download failed:";
        return false;
    }
}

void Syncoperation::OnLoginStatus(const int32_t value)
{
    qInfo() << "login status" << value;
    Q_EMIT LoginStatuschanged(value);
}

void Syncoperation::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    if (interfaceName == SYNC_DBUS_INTERFACE) {
        //获取到属性改变信号，提取出传输的用户信息数据
        for (QVariantMap::const_iterator it = changedProperties.cbegin(), end = changedProperties.cend(); it != end; ++it) {
            if (it.key() == "UserData") {
                //用户信心更新的信号，后续处理 TO：
                QDBusArgument argument = it.value().value<QDBusVariant>().variant().value<QDBusArgument>();
                QVariantMap userInfoMap;
                argument >> userInfoMap;
                Q_EMIT UserDatachanged(userInfoMap);
            }
        }
    }
}