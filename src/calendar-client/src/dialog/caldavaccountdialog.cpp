// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "caldavaccountdialog.h"

#include "accountmanager.h"
#include "commondef.h"
#include "dcaldavprofile.h"
#include "caldavprovidericon.h"
#include "dcaldavcredentialstore.h"
#include "dcalendareventlog.h"

#include <DComboBox>
#include <DFloatingMessage>
#include <DMessageManager>
#include <DLabel>
#include <DLineEdit>
#include <DPalette>

#include <QCoreApplication>
#include <QFormLayout>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QSize>
#include <QSizePolicy>
#include <QTimer>
#include <QUrl>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace {


DLabel *formLabel(const QString &text, QWidget *parent)
{
    DLabel *label = new DLabel(parent);
    label->setText(text + QCoreApplication::translate("CalDavAccountDialog", ":"));
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    return label;
}

} // namespace

CalDavAccountDialog::CalDavAccountDialog(QWidget *parent)
    : DDialog(parent)
{
    setWindowTitle(tr("Add Calendar Account"));
    setFixedSize(500, 300);
    setContentLayoutContentsMargins(QMargins(20, 0, 20, 0));

    QWidget *content = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(10);

    QFormLayout *form = new QFormLayout;
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);
    m_providerComboBox = new DComboBox(content);
    const DCalDavProviderProfile::ProviderType providers[] = {
        DCalDavProviderProfile::Provider_DingTalk,
        DCalDavProviderProfile::Provider_WeCom,
        DCalDavProviderProfile::Provider_TencentMeeting,
        DCalDavProviderProfile::Provider_QQMail,
        DCalDavProviderProfile::Provider_Feishu,
        DCalDavProviderProfile::Provider_Other,
    };
    m_providerComboBox->addItem(tr("Select account type"), -1);
    for (const DCalDavProviderProfile::ProviderType provider : providers) {
        m_providerComboBox->addItem(CalDavAccountUi::providerIcon(provider),
                                     DCalDavProviderProfile::providerName(provider), provider);
    }

    m_providerComboBox->setFixedHeight(36);
    m_providerComboBox->setIconSize(QSize(24, 24));
    m_serverUrlEdit = new DLineEdit(content);
    m_usernameEdit = new DLineEdit(content);
    m_passwordEdit = new DPasswordEdit(content);
    m_serverUrlEdit->setPlaceholderText(tr("Automatically filled after selecting account type"));
    m_usernameEdit->setPlaceholderText(tr("Enter username or email"));
    m_passwordEdit->setPlaceholderText(tr("Enter password"));
    m_serverUrlEdit->setFixedHeight(36);
    m_usernameEdit->setFixedHeight(36);
    m_passwordEdit->setFixedHeight(36);
    m_passwordEdit->setEchoButtonIsVisible(true);
    m_passwordEdit->lineEdit()->setInputMethodHints(Qt::ImhHiddenText | Qt::ImhNoPredictiveText);

    form->addRow(formLabel(tr("Account Type"), content), m_providerComboBox);
    form->addRow(formLabel(tr("Server Address"), content), m_serverUrlEdit);
    form->addRow(formLabel(tr("Username"), content), m_usernameEdit);
    form->addRow(formLabel(tr("Password"), content), m_passwordEdit);
    contentLayout->addLayout(form);

    m_errorLabel = new DLabel(content);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setForegroundRole(DPalette::TextWarning);
    m_errorLabel->hide();
    contentLayout->addWidget(m_errorLabel);
    addContent(content, Qt::AlignVCenter);

    setOnButtonClickedClose(false);
    addButton(tr("Cancel", "button"));
    addButton(tr("Sign In", "button"), false, DDialog::ButtonRecommend);
    getButton(1)->setEnabled(false);
    for (int i = 0; i < buttonCount(); ++i) {
        getButton(i)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        getButton(i)->setFixedHeight(36);
    }
    connect(getButton(0), &QPushButton::clicked, this, &QDialog::reject);
    connect(getButton(1), &QPushButton::clicked, this, &CalDavAccountDialog::slotLogin);
    connect(m_providerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CalDavAccountDialog::slotProviderChanged);
    connect(m_serverUrlEdit->lineEdit(), &QLineEdit::textChanged, this, [this](const QString &) {
        setInputError(m_serverUrlEdit, false);
        m_errorLabel->hide();
        updateLoginButtonState();
    });
    connect(m_usernameEdit->lineEdit(), &QLineEdit::textChanged, this, [this](const QString &) {
        setInputError(m_usernameEdit, false);
        m_errorLabel->hide();
        updateLoginButtonState();
    });
    connect(m_passwordEdit->lineEdit(), &QLineEdit::textChanged, this, [this](const QString &) {
        setInputError(m_passwordEdit, false);
        m_errorLabel->hide();
        updateLoginButtonState();
    });
    const QMetaObject::Connection validationStartedConnection = connect(
        gAccountManager, &AccountManager::signalCalDavAccountValidationStarted, this,
        [this](const QString &requestID) {
            qCDebug(ClientLogger) << "CalDAV dialog received validation start";
            slotValidationStarted(requestID);
        });
    const QMetaObject::Connection validationUpdateStartedConnection = connect(
        gAccountManager, &AccountManager::signalCalDavAccountValidationForUpdateStarted, this,
        [this](const QString &requestID) {
            qCDebug(ClientLogger) << "CalDAV dialog received update validation start";
            slotValidationStarted(requestID);
        });
    const QMetaObject::Connection validationFinishedConnection = connect(
        gAccountManager, &AccountManager::signalCalDavAccountValidationFinished, this,
        [this](const QString &requestID, bool success, int validationError,
               const QString &errorMessage, const QString &principalDisplayName) {
            qCDebug(ClientLogger) << "CalDAV dialog received validation result"
                                    << "validationError:" << validationError
                                    << "principalDisplayNamePresent:" << !principalDisplayName.isEmpty();
            slotValidationFinished(requestID, success, validationError, errorMessage,
                                   principalDisplayName);
        });
    qCDebug(ClientLogger) << "CalDAV dialog validation handlers connected"
                            << "accountManager:" << gAccountManager
                            << "startConnected:" << static_cast<bool>(validationStartedConnection)
                            << "updateStartConnected:" << static_cast<bool>(validationUpdateStartedConnection)
                            << "finishedConnected:" << static_cast<bool>(validationFinishedConnection);
    connect(gAccountManager, &AccountManager::signalCreateCalDavAccountFinish,
            this, &CalDavAccountDialog::slotCreateFinished);
    connect(gAccountManager, &AccountManager::signalUpdateCalDavAccountFinish,
            this, &CalDavAccountDialog::slotUpdateFinished);
    connect(gAccountManager, &AccountManager::signalCalDavAccountRequestFailed,
            this, &CalDavAccountDialog::slotCalDavRequestFailed);

    m_validationTimeout = new QTimer(this);
    m_validationTimeout->setSingleShot(true);
    m_validationTimeout->setInterval(60000);
    connect(m_validationTimeout, &QTimer::timeout,
            this, &CalDavAccountDialog::slotValidationTimedOut);

    updateProviderFields();
}

CalDavAccountDialog::~CalDavAccountDialog()
{
    if (!m_pendingCredentialRef.isEmpty()) {
        DCalDavCredentialStore::deletePassword(m_pendingCredentialRef);
    }
}

void CalDavAccountDialog::setEditAccount(const QString &config)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(config.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return;
    }
    const QJsonObject object = document.object();
    const int providerType = object.value(QStringLiteral("providerType")).toInt();
    const int providerIndex = m_providerComboBox->findData(providerType);
    if (providerIndex < 0) {
        return;
    }

    m_editMode = true;
    m_accountID = object.value(QStringLiteral("accountID")).toString();
    setWindowTitle(tr("Edit Account"));
    getButton(1)->setText(tr("Save", "button"));
    m_providerComboBox->blockSignals(true);
    m_providerComboBox->setCurrentIndex(providerIndex);
    m_providerComboBox->blockSignals(false);
    m_serverUrlEdit->setText(object.value(QStringLiteral("serverUrl")).toString());
    m_usernameEdit->setText(object.value(QStringLiteral("username")).toString());
    m_passwordEdit->clear();
    m_passwordEdit->setPlaceholderText(tr("Leave empty to keep the current password"));
    m_errorLabel->hide();
    updateLoginButtonState();
}

void CalDavAccountDialog::slotProviderChanged(int)
{
    updateProviderFields();
}

void CalDavAccountDialog::slotLogin()
{
    if (!validateInput()) {
        qCDebug(ClientLogger) << "CalDAV login input validation failed";
        return;
    }
    if (!m_editMode) {
        DCalendarEventLog::instance().reportLoginClicked(
            static_cast<DCalDavProviderProfile::ProviderType>(
                m_providerComboBox->currentData().toInt()));
    }

    qCDebug(ClientLogger) << "CalDAV login started"
                            << "provider:" << m_providerComboBox->currentData().toInt()
                            << "editMode:" << m_editMode;

    clearPendingCredential(true);
    if (!storePendingCredential()) {
        showError(QString(), true, false);
        setLoginEnabled(true);
        return;
    }

    setLoginEnabled(false);
    const int providerType = m_providerComboBox->currentData().toInt();
    if (m_editMode) {
        gAccountManager->validateCalDavAccountForUpdate(
            m_accountID, providerType, m_serverUrlEdit->text().trimmed(),
            m_usernameEdit->text().trimmed(), m_pendingCredentialRef);
    } else {
        gAccountManager->validateCalDavAccount(providerType, m_serverUrlEdit->text().trimmed(),
                                               m_usernameEdit->text().trimmed(),
                                               m_pendingCredentialRef);
    }
}

void CalDavAccountDialog::slotValidationStarted(const QString &requestID)
{
    qCDebug(ClientLogger) << "CalDAV validation request started"
                            << "requestIdPresent:" << !requestID.isEmpty();
    if (requestID.isEmpty()) {
        clearPendingCredential(true);
        showError(QString(), true, false);
        setLoginEnabled(true);
        return;
    }
    m_validationRequestID = requestID;
    // The validation-finished signal can arrive before the asynchronous DBus
    // method reply that provides its request ID. Replay that early result once
    // the reply identifies the request, rather than dropping a valid result.
    if (m_earlyValidationRequestID == requestID) {
        const bool success = m_earlyValidationSuccess;
        const int validationError = m_earlyValidationError;
        const QString errorMessage = m_earlyValidationMessage;
        const QString principalDisplayName = m_earlyValidationPrincipalDisplayName;
        m_earlyValidationRequestID.clear();
        m_earlyValidationMessage.clear();
        m_earlyValidationPrincipalDisplayName.clear();
        slotValidationFinished(requestID, success, validationError, errorMessage,
                               principalDisplayName);
        return;
    }
    m_earlyValidationRequestID.clear();
    m_earlyValidationMessage.clear();
    m_earlyValidationPrincipalDisplayName.clear();
    m_validationTimeout->start();
}

void CalDavAccountDialog::slotValidationFinished(const QString &requestID, bool success,
                                                  int validationError, const QString &errorMessage,
                                                  const QString &principalDisplayName)
{
    qCDebug(ClientLogger) << "CalDAV validation finished"
                            << "requestIdPresent:" << !requestID.isEmpty()
                            << "requestMatches:" << (requestID == m_validationRequestID)
                            << "success:" << success
                            << "validationError:" << validationError
                            << "errorPresent:" << !errorMessage.isEmpty();
    if (requestID.isEmpty()) {
        qCWarning(ClientLogger) << "Ignoring CalDAV validation result without request ID";
        return;
    }
    if (m_validationRequestID.isEmpty()) {
        // The service signal won the race against the DBus method reply. Keep
        // its result until slotValidationStarted() receives the request ID and
        // can verify that both messages belong to the same validation request.
        m_earlyValidationRequestID = requestID;
        m_earlyValidationSuccess = success;
        m_earlyValidationError = validationError;
        m_earlyValidationMessage = errorMessage;
        m_earlyValidationPrincipalDisplayName = principalDisplayName;
        qCDebug(ClientLogger) << "Stored early CalDAV validation result";
        return;
    }
    if (requestID != m_validationRequestID) {
        qCWarning(ClientLogger) << "Ignoring unmatched CalDAV validation result";
        return;
    }
    m_validationTimeout->stop();
    m_validationRequestID.clear();
    if (!success) {
        const DCalDavValidationError::Type errorType =
            static_cast<DCalDavValidationError::Type>(validationError);
        const bool credentialError = errorType == DCalDavValidationError::AuthenticationFailed;
        clearPendingCredential(true);
        showValidationToast(errorType);
        showError(validationErrorText(errorType), !credentialError, credentialError);
        setLoginEnabled(true);
        return;
    }

    const int providerType = m_providerComboBox->currentData().toInt();
    qCDebug(ClientLogger) << "CalDAV validation succeeded; creating account"
                            << "provider:" << providerType
                            << "editMode:" << m_editMode;
    if (m_editMode) {
        gAccountManager->updateCalDavAccount(
            m_accountID, providerType, m_serverUrlEdit->text().trimmed(),
            m_usernameEdit->text().trimmed(), m_pendingCredentialRef, principalDisplayName);
        return;
    }

    gAccountManager->createCalDavAccount(providerType, m_serverUrlEdit->text().trimmed(),
                                         m_usernameEdit->text().trimmed(), m_pendingCredentialRef,
                                         principalDisplayName);
}

void CalDavAccountDialog::slotCreateFinished(const QString &accountID)
{
    qCDebug(ClientLogger) << "CalDAV account creation finished"
                            << "accountIdPresent:" << !accountID.isEmpty();
    if (accountID.isEmpty()) {
        clearPendingCredential(true);
        showError(QString(), true, false);
        setLoginEnabled(true);
        return;
    }
    clearPendingCredential(false);
    accept();
}

void CalDavAccountDialog::slotUpdateFinished(bool success)
{
    qCDebug(ClientLogger) << "CalDAV account update finished" << "success:" << success;
    if (!success) {
        clearPendingCredential(true);
        showError(QString(), true, false);
        setLoginEnabled(true);
        return;
    }
    clearPendingCredential(false);
    accept();
}

void CalDavAccountDialog::slotCalDavRequestFailed(const QString &method, const QString &errorMessage)
{
    qCWarning(ClientLogger) << "CalDAV DBus request failed"
                            << "method:" << method
                            << "errorPresent:" << !errorMessage.isEmpty();
    const bool validationRequest = method == QStringLiteral("validateCalDavAccount")
        || method == QStringLiteral("validateCalDavAccountForUpdate");
    if (validationRequest) {
        m_validationTimeout->stop();
        m_validationRequestID.clear();
    }

    if (validationRequest) {
        const DCalDavValidationError::Type errorType = DCalDavValidationError::NetworkUnavailable;
        DCalendarEventLog::instance().reportLoginValidationFinished(false, errorType);
        showValidationToast(errorType);
        showError(validationErrorText(errorType), true, false);
    }
    clearPendingCredential(true);
    setLoginEnabled(true);
}

void CalDavAccountDialog::slotValidationTimedOut()
{
    qCWarning(ClientLogger) << "CalDAV validation timed out"
                            << "requestIdPresent:" << !m_validationRequestID.isEmpty();
    if (m_validationRequestID.isEmpty()) {
        clearPendingCredential(true);
        return;
    }
    m_validationRequestID.clear();
    clearPendingCredential(true);
    const DCalDavValidationError::Type errorType = DCalDavValidationError::NetworkUnavailable;
    DCalendarEventLog::instance().reportLoginValidationFinished(false, errorType);
    showValidationToast(errorType);
    showError(validationErrorText(errorType), true, false);
    setLoginEnabled(true);
}

bool CalDavAccountDialog::storePendingCredential()
{
    if (m_passwordEdit->text().isEmpty()) {
        return true;
    }

    const int providerType = m_providerComboBox->currentData().toInt();
    const DCalDavProviderProfile profile = DCalDavProviderProfile::forProvider(
        static_cast<DCalDavProviderProfile::ProviderType>(providerType));
    QString credentialRef;
    QString errorMessage;
    if (!DCalDavCredentialStore::storePassword(profile.displayName, m_passwordEdit->text(),
                                                credentialRef, &errorMessage)) {
        qCWarning(ClientLogger) << "Failed to store CalDAV credential"
                                << "errorPresent:" << !errorMessage.isEmpty();
        return false;
    }
    m_pendingCredentialRef = credentialRef;
    return true;
}

void CalDavAccountDialog::clearPendingCredential(bool deleteSecret)
{
    if (m_pendingCredentialRef.isEmpty()) {
        return;
    }
    if (deleteSecret) {
        DCalDavCredentialStore::deletePassword(m_pendingCredentialRef);
    }
    m_pendingCredentialRef.clear();
}

void CalDavAccountDialog::updateProviderFields()
{
    m_validationTimeout->stop();
    m_validationRequestID.clear();
    setInputError(m_serverUrlEdit, false);
    setInputError(m_usernameEdit, false);
    setInputError(m_passwordEdit, false);
    const int providerType = m_providerComboBox->currentData().toInt();
    if (providerType < DCalDavProviderProfile::Provider_DingTalk) {
        m_serverUrlEdit->clear();
        m_serverUrlEdit->lineEdit()->setReadOnly(false);
        updateLoginButtonState();
        return;
    }
    const DCalDavProviderProfile profile = DCalDavProviderProfile::forProvider(
        static_cast<DCalDavProviderProfile::ProviderType>(providerType));
    m_serverUrlEdit->setText(profile.serverUrl);
    m_serverUrlEdit->lineEdit()->setReadOnly(!profile.serverUrlEditable);
    setInputError(m_serverUrlEdit, false);
    m_errorLabel->hide();
    updateLoginButtonState();
}

void CalDavAccountDialog::updateLoginButtonState()
{
    const int providerType = m_providerComboBox->currentData().toInt();
    const bool providerSelected = providerType >= DCalDavProviderProfile::Provider_DingTalk
        && providerType <= DCalDavProviderProfile::Provider_Other;
    const bool serverAddressValid = !DCalDavProviderProfile::normalizeServerUrl(
        m_serverUrlEdit->text()).isEmpty();
    const bool usernamePresent = !m_usernameEdit->text().trimmed().isEmpty();
    const bool passwordPresent = m_editMode || !m_passwordEdit->text().isEmpty();
    getButton(1)->setEnabled(providerSelected && serverAddressValid
                              && usernamePresent && passwordPresent);
}

bool CalDavAccountDialog::validateInput()
{
    setInputError(m_serverUrlEdit, false);
    setInputError(m_usernameEdit, false);
    setInputError(m_passwordEdit, false);
    if (m_providerComboBox->currentData().toInt() < DCalDavProviderProfile::Provider_DingTalk) {
        m_errorLabel->hide();
        return false;
    }

    const QUrl serverUrl = DCalDavProviderProfile::normalizeServerUrl(
        m_serverUrlEdit->text());
    if (!serverUrl.isValid()) {
        const QString message = tr("Please enter a valid server address.");
        setInputError(m_serverUrlEdit, true, message);
        m_errorLabel->hide();
        return false;
    }
    // Username and secret are required by the button enable rule. They do not
    // produce a separate dialog prompt when the button is disabled.
    if (m_usernameEdit->text().trimmed().isEmpty()
        || (!m_editMode && m_passwordEdit->text().isEmpty())) {
        m_errorLabel->hide();
        return false;
    }
    m_errorLabel->hide();
    return true;
}

void CalDavAccountDialog::setLoginEnabled(bool enabled)
{
    m_providerComboBox->setEnabled(enabled);
    m_serverUrlEdit->setEnabled(enabled);
    m_usernameEdit->setEnabled(enabled);
    m_passwordEdit->setEnabled(enabled);
    getButton(0)->setEnabled(enabled);
    if (enabled) {
        updateLoginButtonState();
    } else {
        getButton(1)->setEnabled(false);
    }
}

void CalDavAccountDialog::setInputError(DLineEdit *edit, bool error, const QString &message)
{
    if (edit == nullptr) {
        return;
    }
    edit->setAlert(error);
    if (error && !message.isEmpty()) {
        edit->showAlertMessage(message);
    } else if (!error) {
        edit->hideAlertMessage();
    }
}

void CalDavAccountDialog::showError(const QString &message, bool serverError, bool credentialError)
{
    m_errorLabel->hide();
    if (serverError) {
        setInputError(m_serverUrlEdit, true, message.isEmpty()
            ? tr("Please enter a valid server address.") : message);
    }
    if (credentialError) {
        setInputError(m_usernameEdit, true, tr("Please enter the correct username."));
        setInputError(m_passwordEdit, true, tr("Please enter the correct password."));
    }
}

QString CalDavAccountDialog::validationErrorText(
    DCalDavValidationError::Type validationError) const
{
    switch (validationError) {
    case DCalDavValidationError::AuthenticationFailed:
        return tr("Incorrect username or password. Please try again.");
    case DCalDavValidationError::UnsupportedCalDav:
        return tr("This server does not support CalDAV.");
    case DCalDavValidationError::ServerRejected:
        return tr("The server rejected your login request. Please check your account permissions.");
    case DCalDavValidationError::ParseError:
        return tr("Unable to parse the data returned by the server. Please verify the server address or try again later.");
    case DCalDavValidationError::CertificateInvalid:
        return tr("The server certificate is invalid.");
    case DCalDavValidationError::NetworkUnavailable:
    case DCalDavValidationError::NoError:
    default:
        return tr("Unable to connect to the server. Please check your network connection and server address.");
    }
}

void CalDavAccountDialog::showValidationToast(DCalDavValidationError::Type validationError)
{
    const QString toastText = validationErrorText(validationError);

    DFloatingMessage *message = new DFloatingMessage(DFloatingMessage::TransientType);
    message->setIcon(QIcon::fromTheme(QStringLiteral("dialog-error")));
    message->setMessage(toastText);
    message->setDuration(2000);
    DMessageManager::instance()->sendMessage(window(), message);
}
