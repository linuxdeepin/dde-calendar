// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavaccountdeletioncleanup.h"
#include "daccountmanagerdatabase.h"
#include "commondef.h"
#include <QFile>
#include <QDir>
void DCalDavAccountDeletionCleanup::resume(DAccountManagerDataBase *database, const QString &basePath)
{
    if (!database) return;
    const QMap<QString, QString> cleanups = database->getCalDavAccountDeletionCleanups();
    for (auto it = cleanups.constBegin(); it != cleanups.constEnd(); ++it) {
        if (database->getAccountByID(it.key())) { database->deleteCalDavAccountDeletionCleanup(it.key()); continue; }
        const QString dbName = it.value();
        if (dbName.isEmpty() || dbName.contains(QChar::Null) || dbName.contains('/')
            || dbName.contains(QStringLiteral(".."))) {
            qCWarning(ServiceLogger) << "Ignoring unsafe CalDAV cleanup database name.";
            continue;
        }
        const QString databasePath = QDir(basePath).filePath(dbName);
        if (!QFile::exists(databasePath) || QFile::remove(databasePath))
            database->deleteCalDavAccountDeletionCleanup(it.key());
        else
            qCWarning(ServiceLogger) << "Failed to remove pending CalDAV account database:" << databasePath;
    }
}
