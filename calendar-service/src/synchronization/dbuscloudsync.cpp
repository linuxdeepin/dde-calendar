// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbuscloudsync.h"
#include <QJsonObject>
#include <QJsonDocument>
#include "units.h"
#include "commondef.h"

Q_LOGGING_CATEGORY(DBusCloudSyncLog, "calendar.sync.dbus")

Dbuscloudsync::Dbuscloudsync(QObject *parent)
    : DServiceBase(serviceBasePath + "/CloudSync", serviceBaseName + ".CloudSync", parent)
{
    qCDebug(DBusCloudSyncLog) << "Initializing DBus cloud sync service";
}

void Dbuscloudsync::MsgCallBack(const QByteArray &msg)
{
    //msg信息处理
    QJsonObject json;
    json = QJsonDocument::fromJson(msg).object();

    qCDebug(DBusCloudSyncLog) << "Received cloud sync message callback";

    //TODO:解析获取到的数据，依据需要做后续操作
    qCWarning(CommonLogger) << "Get " << " error.";
}

