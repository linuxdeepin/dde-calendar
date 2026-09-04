// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVVALIDATIONERROR_H
#define DCALDAVVALIDATIONERROR_H

class DCalDavValidationError
{
public:
    enum Type {
        NoError = 0,
        NetworkUnavailable,
        AuthenticationFailed,
        UnsupportedCalDav,
        CertificateInvalid,
        Other,
        ServerRejected,
        ParseError,
    };
};

class DCalDavScheduleCreateError
{
public:
    enum Type {
        NoError = 0,
        NetworkUnavailable,
        PermissionDenied,
    };
};

#endif // DCALDAVVALIDATIONERROR_H
