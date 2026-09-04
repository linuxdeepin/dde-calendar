// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVERRORCODE_H
#define DCALDAVERRORCODE_H

enum class DCalDavErrorCode {
        NoError = 0,
        InvalidRequest,
        NetworkUnavailable,
        RequestTimedOut,
        CertificateInvalid,
        AuthenticationFailed,
        PermissionDenied,
        RateLimited,
        ServerUnavailable,
        Conflict,
        ResponseTooLarge,
        NetworkError,
        UnsupportedCalDav,
        ParseError,
        StorageError,
    Unknown,
};

#endif // DCALDAVERRORCODE_H
