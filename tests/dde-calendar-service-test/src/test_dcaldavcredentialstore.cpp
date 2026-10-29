// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"
#include "dcaldavcredentialstore.h"

#include <QString>
#include <QObject>
#include <QTimer>
#include <QEventLoop>

// parseItemPath is a private static method, but -fno-access-control
// makes it accessible from tests.

// ============ parseItemPath — valid cases ============

TEST(DCalDavCredentialStoreTest, ParseItemPath_ValidSimplePath)
{
    QString itemPath;
    // parseItemPath is private static; access via -fno-access-control
    bool result = DCalDavCredentialStore::parseItemPath(
        QStringLiteral("secret-service:/org/freedesktop/secrets/item1"), itemPath);
    EXPECT_TRUE(result);
    EXPECT_EQ(itemPath, "/org/freedesktop/secrets/item1");
}

TEST(DCalDavCredentialStoreTest, ParseItemPath_ValidSingleSegment)
{
    QString itemPath;
    bool result = DCalDavCredentialStore::parseItemPath(
        QStringLiteral("secret-service:/item"), itemPath);
    EXPECT_TRUE(result);
    EXPECT_EQ(itemPath, "/item");
}

TEST(DCalDavCredentialStoreTest, ParseItemPath_ValidDeepPath)
{
    QString itemPath;
    bool result = DCalDavCredentialStore::parseItemPath(
        QStringLiteral("secret-service:/a/b/c/d/e/f"), itemPath);
    EXPECT_TRUE(result);
    EXPECT_EQ(itemPath, "/a/b/c/d/e/f");
}

TEST(DCalDavCredentialStoreTest, ParseItemPath_ValidWithNumbersAndUnderscores)
{
    QString itemPath;
    bool result = DCalDavCredentialStore::parseItemPath(
        QStringLiteral("secret-service:/path_123/abc456"), itemPath);
    EXPECT_TRUE(result);
    EXPECT_EQ(itemPath, "/path_123/abc456");
}

// ============ parseItemPath — invalid cases ============

TEST(DCalDavCredentialStoreTest, ParseItemPath_NoPrefix_ReturnsFalse)
{
    QString itemPath;
    bool result = DCalDavCredentialStore::parseItemPath(
        QStringLiteral("/org/freedesktop/secrets/item1"), itemPath);
    EXPECT_FALSE(result);
    EXPECT_TRUE(itemPath.isEmpty());
}

TEST(DCalDavCredentialStoreTest, ParseItemPath_EmptyString_ReturnsFalse)
{
    QString itemPath;
    bool result = DCalDavCredentialStore::parseItemPath(QString(), itemPath);
    EXPECT_FALSE(result);
    EXPECT_TRUE(itemPath.isEmpty());
}

TEST(DCalDavCredentialStoreTest, ParseItemPath_OnlyPrefix_ReturnsFalse)
{
    QString itemPath;
    bool result = DCalDavCredentialStore::parseItemPath(
        QStringLiteral("secret-service:"), itemPath);
    // prefix matches but path is empty → regex won't match
    EXPECT_FALSE(result);
    EXPECT_TRUE(itemPath.isEmpty());
}

TEST(DCalDavCredentialStoreTest, ParseItemPath_PathWithoutLeadingSlash_ReturnsFalse)
{
    QString itemPath;
    bool result = DCalDavCredentialStore::parseItemPath(
        QStringLiteral("secret-service:org/freedesktop/secrets"), itemPath);
    EXPECT_FALSE(result);
    EXPECT_TRUE(itemPath.isEmpty());
}

TEST(DCalDavCredentialStoreTest, ParseItemPath_PathWithInvalidChars_ReturnsFalse)
{
    QString itemPath;
    bool result = DCalDavCredentialStore::parseItemPath(
        QStringLiteral("secret-service:/path with spaces"), itemPath);
    EXPECT_FALSE(result);
    EXPECT_TRUE(itemPath.isEmpty());
}

TEST(DCalDavCredentialStoreTest, ParseItemPath_PathWithSpecialChars_ReturnsFalse)
{
    QString itemPath;
    // Dots and hyphens are not in [A-Za-z0-9_]
    bool result = DCalDavCredentialStore::parseItemPath(
        QStringLiteral("secret-service:/path.with.dots"), itemPath);
    EXPECT_FALSE(result);
    EXPECT_TRUE(itemPath.isEmpty());
}

TEST(DCalDavCredentialStoreTest, ParseItemPath_PathWithHyphens_ReturnsFalse)
{
    QString itemPath;
    bool result = DCalDavCredentialStore::parseItemPath(
        QStringLiteral("secret-service:/a-b-c"), itemPath);
    EXPECT_FALSE(result);
    EXPECT_TRUE(itemPath.isEmpty());
}

TEST(DCalDavCredentialStoreTest, ParseItemPath_TooLong_ReturnsFalse)
{
    QString itemPath;
    // Build a path longer than 512 chars
    QString longSegment = QString("a").repeated(100);
    QString longPath = "secret-service:";
    for (int i = 0; i < 6; ++i) {
        longPath += "/" + longSegment;
    }
    bool result = DCalDavCredentialStore::parseItemPath(longPath, itemPath);
    EXPECT_FALSE(result);
    EXPECT_TRUE(itemPath.isEmpty());
}

// ============ readPassword — error path (invalid credentialRef) ============

TEST(DCalDavCredentialStoreTest, ReadPassword_InvalidCredentialRef_ReturnsFalse)
{
    QString password;
    QString errorMsg;
    bool result = DCalDavCredentialStore::readPassword("invalid-ref", password, &errorMsg);
    EXPECT_FALSE(result);
    EXPECT_TRUE(password.isEmpty());
    EXPECT_FALSE(errorMsg.isEmpty());
}

TEST(DCalDavCredentialStoreTest, ReadPassword_InvalidCredentialRef_NullErrorMsg)
{
    QString password;
    bool result = DCalDavCredentialStore::readPassword("invalid-ref", password, nullptr);
    EXPECT_FALSE(result);
    EXPECT_TRUE(password.isEmpty());
}

// ============ deletePassword — error path (invalid credentialRef) ============

TEST(DCalDavCredentialStoreTest, DeletePassword_InvalidCredentialRef_ReturnsFalse)
{
    QString errorMsg;
    bool result = DCalDavCredentialStore::deletePassword("invalid-ref", &errorMsg);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMsg.isEmpty());
}

TEST(DCalDavCredentialStoreTest, DeletePassword_InvalidCredentialRef_NullErrorMsg)
{
    bool result = DCalDavCredentialStore::deletePassword("invalid-ref", nullptr);
    EXPECT_FALSE(result);
}

// ============ readPasswordAsync — invalid credentialRef path ============

TEST(DCalDavCredentialStoreTest, ReadPasswordAsync_InvalidCredentialRef_CallsCallbackWithFalse)
{
    bool callbackCalled = false;
    bool callbackSuccess = true;
    QString callbackPassword;
    QString callbackError;

    DCalDavCredentialStore::readPasswordAsync("invalid-ref",
        [&](bool success, const QString &password, const QString &errorMessage) {
            callbackCalled = true;
            callbackSuccess = success;
            callbackPassword = password;
            callbackError = errorMessage;
        });

    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(callbackSuccess);
    EXPECT_TRUE(callbackPassword.isEmpty());
    EXPECT_FALSE(callbackError.isEmpty());
}

TEST(DCalDavCredentialStoreTest, ReadPasswordAsync_InvalidCredentialRef_NullCallback_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE({
        DCalDavCredentialStore::readPasswordAsync("invalid-ref", nullptr);
    });
}

// ============ deletePasswordAsync — invalid credentialRef path ============

TEST(DCalDavCredentialStoreTest, DeletePasswordAsync_InvalidCredentialRef_CallsCallbackWithFalse)
{
    bool callbackCalled = false;
    bool callbackSuccess = true;
    QString callbackError;

    DCalDavCredentialStore::deletePasswordAsync("invalid-ref",
        [&](bool success, const QString &errorMessage) {
            callbackCalled = true;
            callbackSuccess = success;
            callbackError = errorMessage;
        });

    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(callbackSuccess);
    EXPECT_FALSE(callbackError.isEmpty());
}

TEST(DCalDavCredentialStoreTest, DeletePasswordAsync_InvalidCredentialRef_NullCallback_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE({
        DCalDavCredentialStore::deletePasswordAsync("invalid-ref", nullptr);
    });
}

// ============ storePassword — DBus dependent (will fail without Secret Service) ============

TEST(DCalDavCredentialStoreTest, StorePassword_NoSecretService_ReturnsFalse)
{
    QString credentialRef;
    QString errorMsg;
    // Without a Secret Service running, storePassword should fail gracefully
    bool result = DCalDavCredentialStore::storePassword(
        "test-label", "test-password", credentialRef, &errorMsg);
    EXPECT_FALSE(result);
    EXPECT_TRUE(credentialRef.isEmpty());
}
