// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbuscloudsync.h"
#include <QJsonObject>
#include <QJsonDocument>
#include "units.h"
#include "ddlogService.h"

using namespace DDLOG_SERVICE;

Dbuscloudsync::Dbuscloudsync(QObject *parent)
    : DServiceBase(serviceBasePath + "/CloudSync", serviceBaseName + ".CloudSync", parent)
{

}

void Dbuscloudsync::MsgCallBack(const QByteArray &msg)
{
    //msg信息处理
    QJsonObject json;
    json = QJsonDocument::fromJson(msg).object();

    //TODO:解析获取到的数据，依据需要做后续操作

    qCWarning(calendar_service) << "Get " << " error.";
}

