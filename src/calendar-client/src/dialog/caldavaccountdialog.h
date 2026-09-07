// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALDAVACCOUNTDIALOG_H
#define CALDAVACCOUNTDIALOG_H

#include <DDialog>
#include <DLabel>
#include <DLineEdit>
#include <DPasswordEdit>

#include "dcaldavvalidationerror.h"

#include <QString>

class QComboBox;
class QTimer;

DWIDGET_USE_NAMESPACE

class CalDavAccountDialog : public DDialog
{
    Q_OBJECT
public:
    explicit CalDavAccountDialog(QWidget *parent = nullptr);
    ~CalDavAccountDialog() override;

    void setEditAccount(const QString &config);

private slots:
    void slotProviderChanged(int index);
    void slotLogin();
    void slotValidationStarted(const QString &requestID);
    void slotValidationFinished(const QString &requestID, bool success, int validationError,
                                const QString &errorMessage, const QString &principalDisplayName);
    void slotCreateFinished(const QString &accountID);
    void slotUpdateFinished(bool success);
    void slotCalDavRequestFailed(const QString &method, const QString &errorMessage);
    void slotValidationTimedOut();

private:
    void updateProviderFields();
    bool validateInput();
    void updateLoginButtonState();
    void setLoginEnabled(bool enabled);
    void setInputError(DLineEdit *edit, bool error, const QString &message = QString());
    void showError(const QString &message, bool serverError = false, bool credentialError = false);
    QString validationErrorText(DCalDavValidationError::Type validationError) const;
    void showValidationToast(DCalDavValidationError::Type validationError);
    bool storePendingCredential();
    void clearPendingCredential(bool deleteSecret);

    QComboBox *m_providerComboBox = nullptr;
    DLineEdit *m_serverUrlEdit = nullptr;
    DLineEdit *m_usernameEdit = nullptr;
    DPasswordEdit *m_passwordEdit = nullptr;
    DLabel *m_errorLabel = nullptr;
    QTimer *m_validationTimeout = nullptr;
    QString m_validationRequestID;
    QString m_earlyValidationRequestID;
    bool m_earlyValidationSuccess = false;
    int m_earlyValidationError = 0;
    QString m_earlyValidationMessage;
    QString m_earlyValidationPrincipalDisplayName;
    QString m_pendingCredentialRef;
    QString m_accountID;
    bool m_editMode = false;
};

#endif // CALDAVACCOUNTDIALOG_H
