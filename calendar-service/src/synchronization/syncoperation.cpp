#include "syncoperation.h"

static const QString utcloudcalendatpath = "/home/uos/build-dde-calendar-unknown-Debug/calendar-service/dde-calendar-service";

Syncoperation::Syncoperation(QObject *parent)
    : QObject(parent)
    , m_syncInter(new SyncInter(SYNC_DBUS_PATH, SYNC_DBUS_INTERFACE, QDBusConnection::sessionBus(), this))
{

    if (!QDBusConnection::sessionBus().connect(SYNC_DBUS_PATH,
                                               SYNC_DBUS_INTERFACE,
                                               "org.freedesktop.DBus.Properties",
                                               QLatin1String("PropertiesChanged"), this,
                                               SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)))) {
        qWarning() << "the PropertiesChanged was fail!";
    }

    if (!QDBusConnection::sessionBus().connect(m_syncInter->service(), m_syncInter->path(), m_syncInter->interface(),
                                               "", this, SLOT(slotDbusCall(QDBusMessage)))) {
        qWarning() << "the connection was fail!" << "path: " << m_syncInter->path() << "interface: " << m_syncInter->interface();
    };
//    //监听org.freedesktop.DBus.Properties接口的信号
//    if (!QDBusConnection::sessionBus().connect(m_syncInter->service(), m_syncInter->path(),
//                                               "org.freedesktop.DBus.Properties", "", this, SLOT(slotDbusCall(QDBusMessage)))) {
//        qWarning() << "the connection was fail!" << "path: " << m_syncInter->path() << "interface: " << m_syncInter->interface();
//    };
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

SyncoptResult Syncoperation::optGetMainSwitcher()
{
    SyncoptResult result;
    QDBusPendingReply<QString> reply = m_syncInter->SwitcherDump();
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        QJsonObject json;
        json = QJsonDocument::fromJson(reply.value().toUtf8()).object();
        if (json.contains(QString("enabled"))) {
            bool state = json.value(QString("enabled")).toBool();
            result.switch_state = state;
        }
        result.ret = true;
    } else {
        result.ret = false;
    }
    return  result;
}

SyncoptResult Syncoperation::optGetcalendarSwitcher(const QString &path)
{
    SyncoptResult result;
    QDBusPendingReply<bool> reply = m_syncInter->SwitcherGet(path);
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        result.switch_state = reply.value();
        result.ret = true;
    } else {
        result.ret = false;
    }
    return  result;
}

void Syncoperation::slotDbusCall(const QDBusMessage &msg)
{
    qInfo() << msg;
    if (msg.member() == "SwitcherChange") {
        SyncoptResult result;
        //获取总开关状态
        result = optGetMainSwitcher();
        if (result.ret) {
            if (result.switch_state) {
                //总开关开启，获取日历开关状态
                result = optGetcalendarSwitcher(utcloudcalendatpath);
                if (result.ret) {
                    Q_EMIT SwitcherChange(result.switch_state);
                } else {
                    qDebug() << "calendar switcher get state failed";
                    return;
                }
            } else {
                //总开关关闭
                Q_EMIT SwitcherChange(false);
            }
        } else {
            qDebug() << "main switcher get state failed";
        }
    }
}


void Syncoperation::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(interfaceName);
    Q_UNUSED(invalidatedProperties);
    if (!changedProperties.contains("UserData"))
        return;

//    const QDBusArgument &argument = changedProperties.value("UserData").value<QDBusArgument>();
//    QVariantMap userInfoMap;
//    argument >> userInfoMap;
    //发送登录信号
    emit signalLoginStatusChange(true);

}
