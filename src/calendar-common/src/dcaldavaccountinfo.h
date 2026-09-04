// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVACCOUNTINFO_H
#define DCALDAVACCOUNTINFO_H

#include <QDateTime>
#include <QString>

class DCalDavAccountInfo
{
public:
    QString accountId;
    int providerType = 0;
    QString serverUrl;
    QString username;
    QString credentialRef;
    QString accountColor;
    int retryCount = 0;
    QDateTime nextRetryAt;
};

#endif // DCALDAVACCOUNTINFO_H
