#include "syncoperation.h"


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
        qInfo() << "Upload success!";
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
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qWarning() << "Upload failed:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
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
        qInfo() << "Download success!";
        result.data = reply.value();
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qWarning() << "Download failed:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
            qWarning() << result.error_code;
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
        qInfo() << "Delete success!";
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
            qWarning() << result.error_code;
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
        qInfo() << "Metadata success!";
        //元数据获取接口，暂时好像用不到
        result.data = reply.value();
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qWarning() << "Metadata failed:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
            qWarning() << result.error_code;
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

bool Syncoperation::hasAvailable()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(SYNC_DBUS_PATH,
                                                      SYNC_DBUS_INTERFACE,
                                                      "org.freedesktop.DBus.Introspectable",
                                                      QStringLiteral("Introspect"));

    QDBusMessage reply =  QDBusConnection::sessionBus().call(msg);

    if (reply.type() == QDBusMessage::ReplyMessage) {
        QVariant variant = reply.arguments().first();
        return variant.toString().contains("\"Available\"");
    } else {
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

SyncoptResult Syncoperation::optGetCalendarSwitcher()
{
    //登录后立即获取开关状态有问题，需要延迟获取
    QThread::msleep(500);
    SyncoptResult mainswitcher = optGetMainSwitcher();
    SyncoptResult result;
    result.switch_state = false; //默认关闭
    if (mainswitcher.ret) {
        if (mainswitcher.switch_state) {
            //总开关开启,获取日历按钮状态
            QDBusPendingReply<bool> reply = m_syncInter->SwitcherGet(utcloudcalendatpath);
            reply.waitForFinished();
            if (reply.error().message().isEmpty()) {
                result.switch_state = reply.value();
                result.ret = true;
            } else {
                qDebug() << "get calendar switcher failed";
                result.ret = false;
            }
        } else {
            result.ret = false;
        }
    } else {
        qDebug() << "get main switcher failed";
        result.ret = false;
    }

    return  result;
}

void Syncoperation::slotDbusCall(const QDBusMessage &msg)
{
    if (msg.member() == "SwitcherChange") {
        SyncoptResult result;
        //获取总开关状态
        result = optGetCalendarSwitcher();
        if (result.ret) {
            Q_EMIT SwitcherChange(result.switch_state);
        } else {
            Q_EMIT SwitcherChange(false);
        }
    } else if (msg.member() == "LoginStatus") {
        //帐户登陆登出信号监测
        QVariant variant = msg.arguments().first();
        const QDBusArgument &tmp = variant.value<QDBusArgument>();

        QVector<int> loginStatus;
        tmp.beginArray();
        while (!tmp.atEnd()) {
            tmp >> loginStatus;
        }
        tmp.endArray();
        if (loginStatus.size() > 0) {
            emit signalLoginStatusChange(loginStatus.first());
        } else {
            qWarning() << "get loginStatus error";
        }
    }
}


void Syncoperation::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(interfaceName);
    Q_UNUSED(invalidatedProperties);
    if (!changedProperties.contains("UserData"))
        return;
}
