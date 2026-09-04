// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcalendareventlog.h"
#include "commondef.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QVariant>
#include <QDebug>

DCalendarEventLog &DCalendarEventLog::instance()
{
    static DCalendarEventLog eventLog;
    return eventLog;
}

DCalendarEventLog::DCalendarEventLog()
    : m_library(QStringLiteral("libdeepin-event-log.so"))
{
    m_library.setLoadHints(QLibrary::PreventUnloadHint);
    if (!m_library.load()) {
        qCWarning(CommonLogger) << "Failed to load calendar event log library:" << m_library.errorString();
        return;
    }

    m_initialize = reinterpret_cast<InitializeFunction>(m_library.resolve("Initialize"));
    m_writeEventLog = reinterpret_cast<WriteEventLogFunction>(m_library.resolve("WriteEventLog"));
    if (m_initialize == nullptr || m_writeEventLog == nullptr) {
        qCWarning(CommonLogger) << "Calendar event log symbols are unavailable"
                                << "library:" << m_library.fileName()
                                << "error:" << m_library.errorString()
                                << "initializeResolved:" << (m_initialize != nullptr)
                                << "writeResolved:" << (m_writeEventLog != nullptr);
        return;
    }

    m_available = m_initialize(std::string("dde-calendar"), true);
    if (!m_available) {
        qCWarning(CommonLogger) << "Calendar event log initialization failed";
    }
}

void DCalendarEventLog::reportAddAccountClicked()
{
    writeEvent(AddAccountClicked);
}

void DCalendarEventLog::reportLoginClicked(
    DCalDavProviderProfile::ProviderType providerType)
{
    writeEvent(LoginClicked, {{QStringLiteral("account_type"), accountType(providerType)}});
}

void DCalendarEventLog::reportLoginValidationFinished(
    bool success, DCalDavValidationError::Type validationError)
{
    QJsonObject fields;
    fields.insert(QStringLiteral("verify_result"), success
                      ? QStringLiteral("success")
                      : QStringLiteral("fail"));
    if (!success) {
        fields.insert(QStringLiteral("fail_reason"), validationFailureReason(validationError));
    }
    writeEvent(LoginValidationFinished, fields);
}

void DCalendarEventLog::reportDeleteAccount(bool deleteData)
{
    writeEvent(DeleteAccount, {{QStringLiteral("delete_data"), deleteData}});
}

void DCalendarEventLog::writeEvent(EventType eventType, const QJsonObject &fields)
{
    if (!m_available || m_writeEventLog == nullptr) {
        qCDebug(CommonLogger) << "Calendar event log is unavailable; event skipped"
                             << "initialized:" << m_available
                             << "writeResolved:" << (m_writeEventLog != nullptr);
        return;
    }

    QJsonObject event(fields);
    event.insert(QStringLiteral("tid"), static_cast<int>(eventType));
    event.insert(QStringLiteral("event_time"),
                 QJsonValue::fromVariant(QVariant::fromValue(
                     QDateTime::currentMSecsSinceEpoch())));
    m_writeEventLog(QJsonDocument(event).toJson(QJsonDocument::Compact).toStdString());
}

QString DCalendarEventLog::accountType(
    DCalDavProviderProfile::ProviderType providerType)
{
    switch (providerType) {
    case DCalDavProviderProfile::Provider_DingTalk:
        return QStringLiteral("dingtalk");
    case DCalDavProviderProfile::Provider_WeCom:
        return QStringLiteral("wecom");
    case DCalDavProviderProfile::Provider_TencentMeeting:
        return QStringLiteral("tencent_meeting");
    case DCalDavProviderProfile::Provider_QQMail:
        return QStringLiteral("qqmail");
    case DCalDavProviderProfile::Provider_Feishu:
        return QStringLiteral("feishu");
    case DCalDavProviderProfile::Provider_Other:
        return QStringLiteral("caldav_other");
    default:
        return QStringLiteral("unknown");
    }
}

QString DCalendarEventLog::validationFailureReason(
    DCalDavValidationError::Type validationError)
{
    switch (validationError) {
    case DCalDavValidationError::AuthenticationFailed:
        return QStringLiteral("credential_error");
    case DCalDavValidationError::UnsupportedCalDav:
    case DCalDavValidationError::ParseError:
        return QStringLiteral("protocol_error");
    case DCalDavValidationError::NetworkUnavailable:
        return QStringLiteral("network_timeout");
    case DCalDavValidationError::ServerRejected:
        return QStringLiteral("server_reject");
    case DCalDavValidationError::CertificateInvalid:
    case DCalDavValidationError::Other:
    case DCalDavValidationError::NoError:
    default:
        return QStringLiteral("other");
    }
}
