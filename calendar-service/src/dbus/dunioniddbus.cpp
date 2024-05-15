// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dunioniddbus.h"
#include "ddlogService.h"

using namespace DDLOG_SERVICE;

DUnionIDDbus::DUnionIDDbus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    if (!this->isValid()) {
        qCWarning(calendar_service) << "Error connecting remote object, service:" << this->service() << ",path:" << this->path() << ",interface" << this->interface();
        qCWarning(calendar_service) << this->lastError();
        
    }
}

DUnionIDDbus::~DUnionIDDbus()
{

}
