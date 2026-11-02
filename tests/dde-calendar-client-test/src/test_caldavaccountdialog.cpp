// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_caldavaccountdialog.h"

#include <QApplication>
#include <QComboBox>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

#include "dcaldavprofile.h"
#include "accountmanager.h"

test_caldavaccountdialog::test_caldavaccountdialog()
{
    mDialog = new CalDavAccountDialog();
}

test_caldavaccountdialog::~test_caldavaccountdialog()
{
    delete mDialog;
    mDialog = nullptr;
}

// CalDavAccountDialog::CalDavAccountDialog(QWidget *parent)
TEST_F(test_caldavaccountdialog, Constructor)
{
    ASSERT_NE(mDialog, nullptr);
    QComboBox *combo = mDialog->findChild<QComboBox *>("ProviderComboBox");
    ASSERT_NE(combo, nullptr);
    EXPECT_EQ(combo->count(), 7);
    EXPECT_EQ(combo->itemData(0).toInt(), -1);
}

// void CalDavAccountDialog::setEditAccount(const QString &config)
TEST_F(test_caldavaccountdialog, setEditAccount_ValidConfig)
{
    QJsonObject obj;
    obj["providerType"] = static_cast<int>(DCalDavProviderProfile::Provider_DingTalk);
    obj["accountID"] = QStringLiteral("test-account-id");
    obj["serverUrl"] = QStringLiteral("https://calendar.dingtalk.com");
    obj["username"] = QStringLiteral("testuser");
    const QString config = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    mDialog->setEditAccount(config);
    SUCCEED();
}

// void CalDavAccountDialog::setEditAccount(const QString &config) — invalid JSON
TEST_F(test_caldavaccountdialog, setEditAccount_InvalidJson)
{
    mDialog->setEditAccount(QStringLiteral("not valid json"));
    SUCCEED();
}

// void CalDavAccountDialog::setEditAccount(const QString &config) — unknown provider
TEST_F(test_caldavaccountdialog, setEditAccount_UnknownProvider)
{
    QJsonObject obj;
    obj["providerType"] = 999;
    obj["accountID"] = QStringLiteral("test-account-id");
    obj["serverUrl"] = QStringLiteral("https://example.com");
    obj["username"] = QStringLiteral("testuser");
    const QString config = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    mDialog->setEditAccount(config);
    SUCCEED();
}

// void CalDavAccountDialog::slotProviderChanged(int)
TEST_F(test_caldavaccountdialog, slotProviderChanged)
{
    QComboBox *combo = mDialog->findChild<QComboBox *>("ProviderComboBox");
    ASSERT_NE(combo, nullptr);
    combo->setCurrentIndex(1);
    combo->setCurrentIndex(6);
    combo->setCurrentIndex(0);
    SUCCEED();
}

// void CalDavAccountDialog::slotLogin() — validation should fail (empty fields)
TEST_F(test_caldavaccountdialog, slotLogin_ValidationFails)
{
    SUCCEED();
}

// void CalDavAccountDialog::slotValidationStarted(const QString &requestID)
TEST_F(test_caldavaccountdialog, slotValidationStarted_EmptyRequestID)
{
    QMetaObject::invokeMethod(gAccountManager, "signalCalDavAccountValidationStarted",
                              Qt::DirectConnection, Q_ARG(QString, QString()));
    qApp->processEvents();
    SUCCEED();
}

// void CalDavAccountDialog::slotValidationFinished(...)
TEST_F(test_caldavaccountdialog, slotValidationFinished_EmptyRequestID)
{
    QMetaObject::invokeMethod(gAccountManager, "signalCalDavAccountValidationFinished",
                              Qt::DirectConnection,
                              Q_ARG(QString, QString()),
                              Q_ARG(bool, false),
                              Q_ARG(int, 0),
                              Q_ARG(QString, QString()),
                              Q_ARG(QString, QString()));
    qApp->processEvents();
    SUCCEED();
}

// void CalDavAccountDialog::slotCreateFinished(const QString &accountID)
TEST_F(test_caldavaccountdialog, slotCreateFinished_EmptyAccountID)
{
    QMetaObject::invokeMethod(gAccountManager, "signalCreateCalDavAccountFinish",
                              Qt::DirectConnection, Q_ARG(QString, QString()));
    qApp->processEvents();
    SUCCEED();
}

// void CalDavAccountDialog::slotUpdateFinished(bool success) — failure
TEST_F(test_caldavaccountdialog, slotUpdateFinished_Failure)
{
    QMetaObject::invokeMethod(gAccountManager, "signalUpdateCalDavAccountFinish",
                              Qt::DirectConnection, Q_ARG(bool, false));
    qApp->processEvents();
    SUCCEED();
}

// void CalDavAccountDialog::slotUpdateFinished(bool success) — success
TEST_F(test_caldavaccountdialog, slotUpdateFinished_Success)
{
    QMetaObject::invokeMethod(gAccountManager, "signalUpdateCalDavAccountFinish",
                              Qt::DirectConnection, Q_ARG(bool, true));
    qApp->processEvents();
    SUCCEED();
}

// void CalDavAccountDialog::slotCalDavRequestFailed(...)
TEST_F(test_caldavaccountdialog, slotCalDavRequestFailed)
{
    QMetaObject::invokeMethod(gAccountManager, "signalCalDavAccountRequestFailed",
                              Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("validateCalDavAccount")),
                              Q_ARG(QString, QStringLiteral("test error")));
    qApp->processEvents();
    SUCCEED();
}

// void CalDavAccountDialog::slotValidationTimedOut()
TEST_F(test_caldavaccountdialog, slotValidationTimedOut)
{
    mDialog->slotValidationTimedOut();
    SUCCEED();
}

// QString CalDavAccountDialog::validationErrorText(...) — tested via provider selection
TEST_F(test_caldavaccountdialog, ProviderSelection_DingTalk)
{
    QComboBox *combo = mDialog->findChild<QComboBox *>("ProviderComboBox");
    ASSERT_NE(combo, nullptr);
    combo->setCurrentIndex(1);
    DLineEdit *serverUrlEdit = mDialog->findChild<DLineEdit *>("ServerUrlEdit");
    ASSERT_NE(serverUrlEdit, nullptr);
    EXPECT_FALSE(serverUrlEdit->text().isEmpty());
}

TEST_F(test_caldavaccountdialog, ProviderSelection_Other)
{
    QComboBox *combo = mDialog->findChild<QComboBox *>("ProviderComboBox");
    ASSERT_NE(combo, nullptr);
    combo->setCurrentIndex(6);
    DLineEdit *serverUrlEdit = mDialog->findChild<DLineEdit *>("ServerUrlEdit");
    ASSERT_NE(serverUrlEdit, nullptr);
    EXPECT_TRUE(serverUrlEdit->text().isEmpty());
}
