// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVCREDENTIALSTORE_H
#define DCALDAVCREDENTIALSTORE_H

#include <QString>

#include <functional>

class QObject;

class DCalDavCredentialStore
{
public:
    using ReadCallback = std::function<void(bool success, const QString &password,
                                             const QString &errorMessage)>;
    using StoreCallback = std::function<void(bool success, const QString &credentialRef,
                                              const QString &errorMessage)>;
    using DeleteCallback = std::function<void(bool success, const QString &errorMessage)>;

    // credentialRef stores only a Secret Service item reference. Secrets are never
    // persisted or logged by this class.
    static bool storePassword(const QString &label, const QString &password,
                              QString &credentialRef, QString *errorMessage = nullptr);
    static bool readPassword(const QString &credentialRef, QString &password,
                             QString *errorMessage = nullptr);
    static bool deletePassword(const QString &credentialRef, QString *errorMessage = nullptr);

    // Asynchronous APIs avoid nested event loops while waiting for Secret Service
    // prompts. The optional context owns the operation and cancels its callback
    // when the context is destroyed.
    static void storePasswordAsync(const QString &label, const QString &password,
                                   const StoreCallback &callback, QObject *context = nullptr);
    static void readPasswordAsync(const QString &credentialRef, const ReadCallback &callback,
                                  QObject *context = nullptr);
    static void deletePasswordAsync(const QString &credentialRef, const DeleteCallback &callback,
                                    QObject *context = nullptr);

private:
    static bool parseItemPath(const QString &credentialRef, QString &itemPath);
};

#endif // DCALDAVCREDENTIALSTORE_H
