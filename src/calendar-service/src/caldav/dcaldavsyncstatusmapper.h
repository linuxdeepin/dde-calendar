// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVSYNCSTATUSMAPPER_H
#define DCALDAVSYNCSTATUSMAPPER_H

#include "dcaldaverrorcode.h"
#include "dcaldavtransport.h"

class DCalDavSyncStatusMapper
{
public:
    static DCalDavErrorCode errorCodeForFailure(
        const DCalDavTransport::Response &response);
    static int statusForFailure(const DCalDavTransport::Response &response);
};

#endif // DCALDAVSYNCSTATUSMAPPER_H
