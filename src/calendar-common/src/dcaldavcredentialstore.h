// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVCREDENTIALSTORE_H
#define DCALDAVCREDENTIALSTORE_H

#include <QString>

class DCalDavCredentialStore
{
public:
    // credentialRef stores only a Secret Service item reference. Secrets are
    // never persisted or logged by this class.
    static bool storePassword(const QString &label, const QString &password,
                              QString &credentialRef, QString *errorMessage = nullptr);
    static bool readPassword(const QString &credentialRef, QString &password,
                             QString *errorMessage = nullptr);
    static bool deletePassword(const QString &credentialRef, QString *errorMessage = nullptr);

private:
    static bool parseItemPath(const QString &credentialRef, QString &itemPath);
};

#endif // DCALDAVCREDENTIALSTORE_H
