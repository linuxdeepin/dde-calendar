// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncoperation.h"
#include "commondef.h"
#include <QLoggingCategory>


Syncoperation::Syncoperation(QObject *parent)
    : QObject(parent)
    , m_syncInter(new SyncInter(SYNC_DBUS_PATH, SYNC_DBUS_INTERFACE, QDBusConnection::sessionBus(), this))
{

    if (!QDBusConnection::sessionBus().connect(SYNC_DBUS_PATH,
                                               SYNC_DBUS_INTERFACE,
                                               "org.freedesktop.DBus.Properties",
                                               QLatin1String("PropertiesChanged"), this,
                                               SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)))) {
        qCWarning(ServiceLogger) << "Failed to connect to PropertiesChanged signal on DBus!";
    } else {
        qCDebug(ServiceLogger) << "Connected to PropertiesChanged signal on DBus.";
    }

    if (!QDBusConnection::sessionBus().connect(m_syncInter->service(), m_syncInter->path(), m_syncInter->interface(),
                                               "", this, SLOT(slotDbusCall(QDBusMessage)))) {
        qCWarning(ServiceLogger) << "Failed to connect to DBus call! Path:" << m_syncInter->path() << "Interface:" << m_syncInter->interface();
    } else {
        qCDebug(ServiceLogger) << "Connected to DBus call. Path:" << m_syncInter->path() << "Interface:" << m_syncInter->interface();
    }
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
    qCDebug(ServiceLogger) << "Starting upload operation. Key:" << key;
    SyncoptResult result;
    QDBusPendingReply<QByteArray> reply = m_syncInter->Upload(key);
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        qCInfo(ServiceLogger) << "Upload succeeded. Key:" << key;
        result.data = reply.value();
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qCWarning(ServiceLogger) << "Upload failed. Key:" << key << "Error:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
            qCWarning(ServiceLogger) << "Upload error code:" << result.error_code;
        }
    }
    return result;
}

SyncoptResult Syncoperation::optDownload(const QString &key, const QString &path)
{
    qCDebug(ServiceLogger) << "Starting download operation. Key:" << key << "Path:" << path;
    SyncoptResult result;
    QDBusPendingReply<QString> reply = m_syncInter->Download(key, path);
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        qCInfo(ServiceLogger) << "Download succeeded. Key:" << key << "Path:" << path;
        result.data = reply.value();
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qCWarning(ServiceLogger) << "Download failed. Key:" << key << "Path:" << path << "Error:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
            qCWarning(ServiceLogger) << "Download error code:" << result.error_code;
        }
    }
    return result;
}

SyncoptResult Syncoperation::optDelete(const QString &key)
{
    qCDebug(ServiceLogger) << "Starting delete operation. Key:" << key;
    SyncoptResult result;
    QDBusPendingReply<QString> reply = m_syncInter->Delete(key);
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        qCInfo(ServiceLogger) << "Delete succeeded. Key:" << key;
        result.data = reply.value();
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qCWarning(ServiceLogger) << "Delete failed. Key:" << key << "Error:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
            qCWarning(ServiceLogger) << "Delete error code:" << result.error_code;
        }
    }
    return result;
}

SyncoptResult Syncoperation::optMetadata(const QString &key)
{
    qCDebug(ServiceLogger) << "Starting metadata operation. Key:" << key;
    SyncoptResult result;
    QDBusPendingReply<QString> reply = m_syncInter->Metadata(key);
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        qCInfo(ServiceLogger) << "Metadata succeeded. Key:" << key;
        result.data = reply.value();
        result.ret = true;
        result.error_code = SYNC_No_Error;
    } else {
        qCWarning(ServiceLogger) << "Metadata failed. Key:" << key << "Error:" << reply.error().message();
        result.ret = false;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply.error().message().toLocal8Bit().data());
        QJsonObject obj = jsonDocument.object();
        if (obj.contains(QString("code"))) {
            result.error_code = obj.value(QString("code")).toInt();
            qCWarning(ServiceLogger) << "Metadata error code:" << result.error_code;
        }
    }
    return result;
}

bool Syncoperation::optUserData(QVariantMap &userInfoMap)
{
    qCDebug(ServiceLogger) << "Requesting user data via DBus";
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
        qCInfo(ServiceLogger) << "Successfully retrieved user data via DBus.";
        return true;
    } else {
        qCWarning(ServiceLogger) << "Failed to retrieve user data via DBus. Message type:" << reply.type();
        return false;
    }
}

bool Syncoperation::hasAvailable()
{
    qCDebug(ServiceLogger) << "Checking if sync service is available via DBus";
    QDBusMessage msg = QDBusMessage::createMethodCall(SYNC_DBUS_PATH,
                                                      SYNC_DBUS_INTERFACE,
                                                      "org.freedesktop.DBus.Introspectable",
                                                      QStringLiteral("Introspect"));

    QDBusMessage reply =  QDBusConnection::sessionBus().call(msg);
    qCInfo(ServiceLogger) << "Syncoperation hasAvailable reply type:" << reply.type();
    if (reply.type() == QDBusMessage::ReplyMessage) {
        qCDebug(ServiceLogger) << "Sync service is available.";
        return true;
    } else {
        qCWarning(ServiceLogger) << "Sync service is not available. Message type:" << reply.type();
        return false;
    }
}

SyncoptResult Syncoperation::optGetMainSwitcher()
{
    qCDebug(ServiceLogger) << "Getting main switcher state via DBus";
    SyncoptResult result;
    QDBusPendingReply<QString> reply = m_syncInter->SwitcherDump();
    reply.waitForFinished();
    if (reply.error().message().isEmpty()) {
        QJsonObject json;
        json = QJsonDocument::fromJson(reply.value().toUtf8()).object();
        if (json.contains(QString("enabled"))) {
            bool state = json.value(QString("enabled")).toBool();
            result.switch_state = state;
            qCInfo(ServiceLogger) << "Main switcher state:" << state;
        }
        result.ret = true;
    } else {
        qCWarning(ServiceLogger) << "Failed to get main switcher state via DBus. Error:" << reply.error().message();
        result.ret = false;
    }
    return  result;
}

SyncoptResult Syncoperation::optGetCalendarSwitcher()
{
    qCDebug(ServiceLogger) << "Getting calendar switcher state via DBus";
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
                qCInfo(ServiceLogger) << "Calendar switcher state:" << result.switch_state;
            } else {
                qCWarning(ServiceLogger) << "Failed to get calendar switcher state via DBus. Error:" << reply.error().message();
                result.ret = false;
            }
        } else {
            qCWarning(ServiceLogger) << "Main switcher is off, calendar switcher state not available.";
            result.ret = false;
        }
    } else {
        qCWarning(ServiceLogger) << "Failed to get main switcher, cannot get calendar switcher state.";
        result.ret = false;
    }
    return  result;
}

void Syncoperation::slotDbusCall(const QDBusMessage &msg)
{
    qCDebug(ServiceLogger) << "Received DBus call. Member:" << msg.member();
    if (msg.member() == "SwitcherChange") {
        SyncoptResult result;
        //获取总开关状态
        result = optGetCalendarSwitcher();
        if (result.ret) {
            qCDebug(ServiceLogger) << "SwitcherChange: Calendar switcher state:" << result.switch_state;
            Q_EMIT SwitcherChange(result.switch_state);
        } else {
            qCWarning(ServiceLogger) << "SwitcherChange: Failed to get calendar switcher state.";
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
            qCDebug(ServiceLogger) << "LoginStatus changed. New status:" << loginStatus.first();
            emit signalLoginStatusChange(loginStatus.first());
        } else {
            qCWarning(ServiceLogger) << "Failed to get loginStatus from DBus message.";
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
