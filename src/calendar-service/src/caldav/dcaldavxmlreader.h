// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVXMLREADER_H
#define DCALDAVXMLREADER_H

#include <QString>
#include <QVector>

class DCalDavXmlReader
{
public:
    enum Privilege {
        NoPrivilege = 0,
        ReadPrivilege = 0x1,
        WritePrivilege = 0x2,
    };

    struct CalendarCollection {
        QString href;
        QString displayName;
        QString color;
        int privileges = NoPrivilege;
        bool privilegesKnown = false;
    };

    struct DiscoveryResult {
        QString currentUserPrincipalHref;
        QString calendarHomeSetHref;
        QString principalDisplayName;
        QVector<CalendarCollection> calendarCollections;
    };

    static bool parseDiscovery(const QByteArray &xml, DiscoveryResult &result, QString *errorMessage = nullptr);
};

#endif // DCALDAVXMLREADER_H
