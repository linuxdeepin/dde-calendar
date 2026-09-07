// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVRECOVERYHANDLER_H
#define DCALDAVRECOVERYHANDLER_H

#include <QString>

class DAccountDataBase;
class DAccountManagerDataBase;

class DCalDavRecoveryHandler
{
public:
    static void recover(DAccountDataBase *localDatabase,
                        DAccountManagerDataBase *accountManagerDatabase,
                        const QString &accountID);
};

#endif // DCALDAVRECOVERYHANDLER_H
