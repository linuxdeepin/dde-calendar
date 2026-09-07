// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVACCOUNTDELETIONCLEANUP_H
#define DCALDAVACCOUNTDELETIONCLEANUP_H
#include <QString>
class DAccountManagerDataBase;
class DCalDavAccountDeletionCleanup
{
public:
    static void resume(DAccountManagerDataBase *database, const QString &basePath);
};
#endif
