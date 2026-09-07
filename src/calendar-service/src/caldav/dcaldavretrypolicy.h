// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVRETRYPOLICY_H
#define DCALDAVRETRYPOLICY_H

#include "dcaldavtransport.h"

class DCalDavRetryPolicy
{
public:
    struct Decision {
        bool retry = false;
        int delaySeconds = 0;
    };

    static Decision decide(const DCalDavTransport::Response &response, int retryCount);
};

#endif // DCALDAVRETRYPOLICY_H
