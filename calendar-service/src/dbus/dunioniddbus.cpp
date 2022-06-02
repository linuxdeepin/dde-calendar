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
#include "dunioniddbus.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonObject>

DUnionIDDbus::DUnionIDDbus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    if (!this->isValid()) {
        qWarning() << "Error connecting remote object, service:" << this->service() << ",path:" << this->path() << ",interface" << this->interface();
    }
    //关联后端dbus触发信号
    // if (!QDBusConnection::sessionBus().connect(this->service(), this->path(), this->interface(), "", this, SLOT(propertyChanged(QDBusMessage)))) {
    //     qWarning() << "the connection was fail!";
    // }
}

DUnionIDDbus::~DUnionIDDbus()
{

}

#if 0
DAccount::Ptr DUnionIDDbus::getUserData()
{
    return accountChangeHandle(getPropertyByName("UserData").toString());
}


DAccount::Ptr DUnionIDDbus::accountChangeHandle(const QString &accountInfo)
{
    //TODO:数据解析
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(accountInfo.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "error:" << jsonError.errorString();
        return nullptr;
    }
    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("profile_image")) {
        m_account->setAvatar(rootObj.value("profile_image").toString());
    }
    if (rootObj.contains("uid")) {
        m_account->setAccountID(rootObj.value("uid").toString());
    }
    if (rootObj.contains("username")) {
        m_account->setDisplayName(rootObj.value("username").toString());
    }
    if (rootObj.contains("nickname")) {
        m_account->setAccountName(rootObj.value("nickname").toString());
    }
    return m_account;
}

QVariant DUnionIDDbus::getPropertyByName(const QString &porpertyName)
{
    //TODO:未能获取到正确数据
    QDBusInterface dbusinterface(this->service(), this->path(), "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus(), this);
    QList<QVariant> argumentList;
    argumentList << "com.deepin.utcloud.Daemon";
    argumentList << porpertyName;
    QDBusMessage reply = dbusinterface.callWithArgumentList(QDBus::Block, QStringLiteral("Get"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "Get " << porpertyName << " error.";
        return QVariant();
    }
    QDBusReply<QVariant> data = reply;
    return data;
}

void DUnionIDDbus::propertyChanged(const QDBusMessage &msg)
{
    if (msg.type() == QDBusMessage::SignalMessage && msg.path() == this->path() && msg.interface() == this->interface()) {
        if (msg.member() == "UserData") {
            //如果是帐户信息改变信号
            if (msg.arguments().size() > 0) {
                accountChangeHandle(msg.arguments().first().toString());
            }
            return;
        }
    }
}

void DUnionIDDbus::slotDBusError(const QDBusError &error)
{
    qWarning() << Q_FUNC_INFO << error;
}
#endif
