// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavretrypolicy.h"
#include "dcaldavtransport.h"

#include <gtest/gtest.h>

namespace {

DCalDavTransport::Response makeResponse(DCalDavTransport::Error error)
{
    DCalDavTransport::Response response;
    response.error = error;
    return response;
}

} // namespace

TEST(CalDavRetryPolicySvc, NegativeRetryCountReturnsNoRetry)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::NetworkUnavailable);
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, -1);
    EXPECT_FALSE(decision.retry);
    EXPECT_EQ(0, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, ZeroRetryCountUsesFallbackDelay)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::NetworkUnavailable);
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(60, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, FirstRetryUsesFiveMinuteDelay)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::NetworkUnavailable);
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 1);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(300, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, SubsequentRetriesUseFifteenMinuteDelay)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::NetworkUnavailable);
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 2);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(900, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, RequestTimedOutIsRetryable)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::RequestTimedOut);
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(60, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, ServerUnavailableIsRetryable)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::ServerUnavailable);
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(60, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, NonRetryableErrorsReturnNoRetry)
{
    const DCalDavTransport::Error nonRetryableErrors[] = {
        DCalDavTransport::NoError,
        DCalDavTransport::InvalidUrl,
        DCalDavTransport::InsecureUrl,
        DCalDavTransport::CertificateInvalid,
        DCalDavTransport::AuthenticationFailed,
        DCalDavTransport::PermissionDenied,
        DCalDavTransport::ResponseTooLarge,
    };
    for (DCalDavTransport::Error error : nonRetryableErrors) {
        DCalDavTransport::Response response = makeResponse(error);
        const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
        EXPECT_FALSE(decision.retry) << "error=" << static_cast<int>(error);
        EXPECT_EQ(0, decision.delaySeconds) << "error=" << static_cast<int>(error);
    }
}

TEST(CalDavRetryPolicySvc, RetryAfterAsZeroSeconds)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::NetworkUnavailable);
    response.retryAfter = QByteArray("0");
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(0, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, RetryAfterAsNegativeNumberUsesFallback)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::NetworkUnavailable);
    response.retryAfter = QByteArray("-5");
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(60, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, EmptyRetryAfterUsesFallback)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::NetworkError);
    response.retryAfter = QByteArray();
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(60, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, InvalidRetryAfterStringUsesFallback)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::RateLimited);
    response.retryAfter = QByteArray("not-a-number");
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 1);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(300, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, RetryAfterWithWhitespaceIsParsed)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::NetworkUnavailable);
    response.retryAfter = QByteArray("  120  ");
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(120, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, RetryAfterAsRfc2822FutureDate)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::ServerUnavailable);
    response.retryAfter = QByteArray("Wed, 02 Sep 2099 14:00:00 +0000");
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
    EXPECT_TRUE(decision.retry);
    EXPECT_GT(decision.delaySeconds, 0);
}

TEST(CalDavRetryPolicySvc, RetryAfterAsRfc2822PastDateClampsToZero)
{
    DCalDavTransport::Response response = makeResponse(DCalDavTransport::ServerUnavailable);
    response.retryAfter = QByteArray("Sat, 02 Sep 2000 14:00:00 +0000");
    const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
    EXPECT_TRUE(decision.retry);
    EXPECT_EQ(0, decision.delaySeconds);
}

TEST(CalDavRetryPolicySvc, AllRetryableErrorTypes)
{
    const DCalDavTransport::Error retryableErrors[] = {
        DCalDavTransport::NetworkUnavailable,
        DCalDavTransport::RequestTimedOut,
        DCalDavTransport::RateLimited,
        DCalDavTransport::ServerUnavailable,
        DCalDavTransport::NetworkError,
    };
    for (DCalDavTransport::Error error : retryableErrors) {
        DCalDavTransport::Response response = makeResponse(error);
        const DCalDavRetryPolicy::Decision decision = DCalDavRetryPolicy::decide(response, 0);
        EXPECT_TRUE(decision.retry) << "error=" << static_cast<int>(error);
        EXPECT_EQ(60, decision.delaySeconds) << "error=" << static_cast<int>(error);
    }
}
