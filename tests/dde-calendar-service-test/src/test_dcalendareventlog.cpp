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

} // namespace
