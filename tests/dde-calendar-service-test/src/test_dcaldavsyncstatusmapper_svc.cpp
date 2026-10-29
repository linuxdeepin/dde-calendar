// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavsyncstatusmapper.h"
#include "dcaldavtransport.h"
#include "dcaldaverrorcode.h"
#include "dcaldavaccountstatus.h"

#include <gtest/gtest.h>

#include <vector>

namespace {

DCalDavTransport::Response makeResponse(DCalDavTransport::Error error,
                                        int httpStatus)
{
    DCalDavTransport::Response response;
    response.error = error;
    response.httpStatus = httpStatus;
    return response;
}

struct HttpStatusCase {
    int httpStatus;
    DCalDavErrorCode expectedCode;
};

struct TransportErrorCase {
    DCalDavTransport::Error error;
    DCalDavErrorCode expectedCode;
};

} // namespace

TEST(DCalDavSyncStatusMapper, ErrorCodeForFailureMapsHttpStatusCodes)
{
    const std::vector<HttpStatusCase> cases = {
        {401, DCalDavErrorCode::AuthenticationFailed},
        {403, DCalDavErrorCode::PermissionDenied},
        {409, DCalDavErrorCode::Conflict},
        {412, DCalDavErrorCode::Conflict},
        {429, DCalDavErrorCode::RateLimited},
        {503, DCalDavErrorCode::ServerUnavailable},
    };

    for (const auto &testCase : cases) {
        auto response = makeResponse(DCalDavTransport::NoError, testCase.httpStatus);
        EXPECT_EQ(testCase.expectedCode,
                  DCalDavSyncStatusMapper::errorCodeForFailure(response))
            << "httpStatus: " << testCase.httpStatus;
    }
}

TEST(DCalDavSyncStatusMapper, ErrorCodeForFailureFallsThroughToTransportError)
{
    const std::vector<TransportErrorCase> cases = {
        {DCalDavTransport::InvalidUrl, DCalDavErrorCode::InvalidRequest},
        {DCalDavTransport::InsecureUrl, DCalDavErrorCode::InvalidRequest},
        {DCalDavTransport::CertificateInvalid, DCalDavErrorCode::CertificateInvalid},
        {DCalDavTransport::NetworkUnavailable, DCalDavErrorCode::NetworkUnavailable},
        {DCalDavTransport::RequestTimedOut, DCalDavErrorCode::RequestTimedOut},
        {DCalDavTransport::AuthenticationFailed, DCalDavErrorCode::AuthenticationFailed},
        {DCalDavTransport::PermissionDenied, DCalDavErrorCode::PermissionDenied},
        {DCalDavTransport::RateLimited, DCalDavErrorCode::RateLimited},
        {DCalDavTransport::ServerUnavailable, DCalDavErrorCode::ServerUnavailable},
        {DCalDavTransport::ResponseTooLarge, DCalDavErrorCode::ResponseTooLarge},
        {DCalDavTransport::NetworkError, DCalDavErrorCode::NetworkError},
        {DCalDavTransport::NoError, DCalDavErrorCode::NoError},
    };

    for (const auto &testCase : cases) {
        auto response = makeResponse(testCase.error, 0);
        EXPECT_EQ(testCase.expectedCode,
                  DCalDavSyncStatusMapper::errorCodeForFailure(response))
            << "transport error: " << static_cast<int>(testCase.error);
    }
}

TEST(DCalDavSyncStatusMapper, ErrorCodeForFailureReturnsUnknownForUnmappedError)
{
    auto response = makeResponse(static_cast<DCalDavTransport::Error>(9999), 0);
    EXPECT_EQ(DCalDavErrorCode::Unknown,
              DCalDavSyncStatusMapper::errorCodeForFailure(response));
}

TEST(DCalDavSyncStatusMapper, HttpStatusTakesPrecedenceOverTransportError)
{
    auto response = makeResponse(DCalDavTransport::NetworkError, 401);
    EXPECT_EQ(DCalDavErrorCode::AuthenticationFailed,
              DCalDavSyncStatusMapper::errorCodeForFailure(response));
}

TEST(DCalDavSyncStatusMapper, StatusForFailureMapsErrorCodeToSyncStatus)
{
    struct StatusCase {
        DCalDavErrorCode errorCode;
        DCalDavSyncStatus::Value expectedStatus;
    };
    const std::vector<StatusCase> cases = {
        {DCalDavErrorCode::AuthenticationFailed, DCalDavSyncStatus::AuthenticationRequired},
        {DCalDavErrorCode::PermissionDenied, DCalDavSyncStatus::PermissionDenied},
        {DCalDavErrorCode::Conflict, DCalDavSyncStatus::Conflict},
        {DCalDavErrorCode::NoError, DCalDavSyncStatus::Failed},
        {DCalDavErrorCode::NetworkError, DCalDavSyncStatus::Failed},
        {DCalDavErrorCode::Unknown, DCalDavSyncStatus::Failed},
    };

    for (const auto &testCase : cases) {
        auto response = makeResponse(DCalDavTransport::NoError, 0);
        if (testCase.errorCode == DCalDavErrorCode::AuthenticationFailed) {
            response.httpStatus = 401;
        } else if (testCase.errorCode == DCalDavErrorCode::PermissionDenied) {
            response.httpStatus = 403;
        } else if (testCase.errorCode == DCalDavErrorCode::Conflict) {
            response.httpStatus = 409;
        } else if (testCase.errorCode == DCalDavErrorCode::NetworkError) {
            response.error = DCalDavTransport::NetworkError;
        }
        EXPECT_EQ(static_cast<int>(testCase.expectedStatus),
                  DCalDavSyncStatusMapper::statusForFailure(response))
            << "errorCode: " << static_cast<int>(testCase.errorCode);
    }
}
