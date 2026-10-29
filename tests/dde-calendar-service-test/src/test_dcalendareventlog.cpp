// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcalendareventlog.h"

#include <gtest/gtest.h>

#include <array>

namespace {

struct AccountTypeCase {
    DCalDavProviderProfile::ProviderType providerType;
    const char *expected;
};

struct ValidationFailureReasonCase {
    DCalDavValidationError::Type validationError;
    const char *expected;
};

TEST(DCalendarEventLog, MapsAccountTypesAccordingToProtocol)
{
    constexpr std::array<AccountTypeCase, 6> cases = {{
        {DCalDavProviderProfile::Provider_DingTalk, "dingtalk"},
        {DCalDavProviderProfile::Provider_WeCom, "wecom"},
        {DCalDavProviderProfile::Provider_TencentMeeting, "tencent_meeting"},
        {DCalDavProviderProfile::Provider_QQMail, "qqmail"},
        {DCalDavProviderProfile::Provider_Feishu, "feishu"},
        {DCalDavProviderProfile::Provider_Other, "caldav_other"},
    }};

    for (const auto &testCase : cases) {
        EXPECT_EQ(QString::fromLatin1(testCase.expected),
                  DCalendarEventLog::accountType(testCase.providerType))
            << "provider type: " << static_cast<int>(testCase.providerType);
    }
}

TEST(DCalendarEventLog, MapsUnknownAccountTypeToUnknown)
{
    constexpr auto invalidProviderType =
        static_cast<DCalDavProviderProfile::ProviderType>(-1);

    EXPECT_EQ(QStringLiteral("unknown"),
              DCalendarEventLog::accountType(invalidProviderType));
}

TEST(DCalendarEventLog, MapsValidationFailureReasonsAccordingToProtocol)
{
    constexpr std::array<ValidationFailureReasonCase, 5> cases = {{
        {DCalDavValidationError::AuthenticationFailed, "credential_error"},
        {DCalDavValidationError::NetworkUnavailable, "network_timeout"},
        {DCalDavValidationError::ServerRejected, "server_reject"},
        {DCalDavValidationError::UnsupportedCalDav, "protocol_error"},
        {DCalDavValidationError::Other, "other"},
    }};

    for (const auto &testCase : cases) {
        EXPECT_EQ(QString::fromLatin1(testCase.expected),
                  DCalendarEventLog::validationFailureReason(testCase.validationError))
            << "validation error: " << static_cast<int>(testCase.validationError);
    }
}

TEST(DCalendarEventLog, MapsUnsupportedValidationErrorToOther)
{
    constexpr std::array<DCalDavValidationError::Type, 2> invalidErrors = {{
        DCalDavValidationError::NoError,
        static_cast<DCalDavValidationError::Type>(-1),
    }};

    for (const auto validationError : invalidErrors) {
        EXPECT_EQ(QStringLiteral("other"),
                  DCalendarEventLog::validationFailureReason(validationError))
            << "validation error: " << static_cast<int>(validationError);
    }
}

// --- Extended tests for instance / report methods ---

TEST(DCalendarEventLog, InstanceReturnsSameSingleton)
{
    DCalendarEventLog &first = DCalendarEventLog::instance();
    DCalendarEventLog &second = DCalendarEventLog::instance();
    EXPECT_EQ(&first, &second);
}

TEST(DCalendarEventLog, ReportAddAccountClickedDoesNotCrash)
{
    // libdeepin-event-log.so is not present in the test environment, so
    // m_available == false and writeEvent early-returns.  The call must
    // not crash.
    DCalendarEventLog::instance().reportAddAccountClicked();
    SUCCEED();
}

TEST(DCalendarEventLog, ReportDeleteAccountDoesNotCrash)
{
    DCalendarEventLog::instance().reportDeleteAccount(true);
    DCalendarEventLog::instance().reportDeleteAccount(false);
    SUCCEED();
}

TEST(DCalendarEventLog, ReportLoginClickedDoesNotCrash)
{
    const std::array<DCalDavProviderProfile::ProviderType, 6> providers = {{
        DCalDavProviderProfile::Provider_DingTalk,
        DCalDavProviderProfile::Provider_WeCom,
        DCalDavProviderProfile::Provider_TencentMeeting,
        DCalDavProviderProfile::Provider_QQMail,
        DCalDavProviderProfile::Provider_Feishu,
        DCalDavProviderProfile::Provider_Other,
    }};
    for (const auto provider : providers) {
        DCalendarEventLog::instance().reportLoginClicked(provider);
    }
    SUCCEED();
}

TEST(DCalendarEventLog, ReportLoginValidationFinishedDoesNotCrash)
{
    const std::array<DCalDavValidationError::Type, 7> errors = {{
        DCalDavValidationError::NoError,
        DCalDavValidationError::NetworkUnavailable,
        DCalDavValidationError::AuthenticationFailed,
        DCalDavValidationError::UnsupportedCalDav,
        DCalDavValidationError::CertificateInvalid,
        DCalDavValidationError::Other,
        DCalDavValidationError::ServerRejected,
    }};
    for (const auto error : errors) {
        DCalendarEventLog::instance().reportLoginValidationFinished(true, error);
        DCalendarEventLog::instance().reportLoginValidationFinished(false, error);
    }
    SUCCEED();
}

TEST(DCalendarEventLog, WriteEventEarlyReturnsWhenUnavailable)
{
    // Access private writeEvent via -fno-access-control.
    // Force m_available to false so that writeEvent takes the early-return
    // path without invoking the function pointer.
    DCalendarEventLog &log = DCalendarEventLog::instance();
    const bool savedAvailable = log.m_available;
    log.m_available = false;

    QJsonObject fields;
    fields.insert("test_key", "test_value");
    log.writeEvent(DCalendarEventLog::AddAccountClicked, fields);
    log.writeEvent(DCalendarEventLog::LoginClicked, fields);
    log.writeEvent(DCalendarEventLog::LoginValidationFinished, fields);
    log.writeEvent(DCalendarEventLog::DeleteAccount, fields);

    log.m_available = savedAvailable;
    SUCCEED();
}

} // namespace
