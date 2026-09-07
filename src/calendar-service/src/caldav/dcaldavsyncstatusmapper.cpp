// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavsyncstatusmapper.h"

#include "dcaldavaccountstatus.h"

DCalDavErrorCode DCalDavSyncStatusMapper::errorCodeForFailure(
    const DCalDavTransport::Response &response)
{
    switch (response.httpStatus) {
    case 401:
        return DCalDavErrorCode::AuthenticationFailed;
    case 403:
        return DCalDavErrorCode::PermissionDenied;
    case 409:
    case 412:
        return DCalDavErrorCode::Conflict;
    case 429:
        return DCalDavErrorCode::RateLimited;
    case 503:
        return DCalDavErrorCode::ServerUnavailable;
    default:
        break;
    }

    switch (response.error) {
    case DCalDavTransport::InvalidUrl:
    case DCalDavTransport::InsecureUrl:
        return DCalDavErrorCode::InvalidRequest;
    case DCalDavTransport::CertificateInvalid:
        return DCalDavErrorCode::CertificateInvalid;
    case DCalDavTransport::NetworkUnavailable:
        return DCalDavErrorCode::NetworkUnavailable;
    case DCalDavTransport::RequestTimedOut:
        return DCalDavErrorCode::RequestTimedOut;
    case DCalDavTransport::AuthenticationFailed:
        return DCalDavErrorCode::AuthenticationFailed;
    case DCalDavTransport::PermissionDenied:
        return DCalDavErrorCode::PermissionDenied;
    case DCalDavTransport::RateLimited:
        return DCalDavErrorCode::RateLimited;
    case DCalDavTransport::ServerUnavailable:
        return DCalDavErrorCode::ServerUnavailable;
    case DCalDavTransport::ResponseTooLarge:
        return DCalDavErrorCode::ResponseTooLarge;
    case DCalDavTransport::NetworkError:
        return DCalDavErrorCode::NetworkError;
    case DCalDavTransport::NoError:
        return DCalDavErrorCode::NoError;
    default:
        return DCalDavErrorCode::Unknown;
    }
}

int DCalDavSyncStatusMapper::statusForFailure(const DCalDavTransport::Response &response)
{
    return DCalDavSyncStatus::fromErrorCode(errorCodeForFailure(response));
}
