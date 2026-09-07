// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavretrypolicy.h"

#include <QDateTime>

#include <limits>

namespace {

int fallbackDelaySeconds(int retryCount)
{
    switch (retryCount) {
    case 0:
        return 60;
    case 1:
        return 5 * 60;
    default:
        return 15 * 60;
    }
}

} // namespace

DCalDavRetryPolicy::Decision DCalDavRetryPolicy::decide(const DCalDavTransport::Response &response,
                                                        int retryCount)
{
    Decision decision;
    if (retryCount < 0) {
        return decision;
    }

    const bool retryable = response.error == DCalDavTransport::NetworkUnavailable
        || response.error == DCalDavTransport::RequestTimedOut
        || response.error == DCalDavTransport::RateLimited
        || response.error == DCalDavTransport::ServerUnavailable
        || response.error == DCalDavTransport::NetworkError;
    if (!retryable) {
        return decision;
    }

    bool retryAfterParsed = false;
    const int retryAfterSeconds = response.retryAfter.trimmed().toInt(&retryAfterParsed);
    int delaySeconds = fallbackDelaySeconds(retryCount);
    if (retryAfterParsed && retryAfterSeconds >= 0) {
        delaySeconds = retryAfterSeconds;
    } else if (!response.retryAfter.trimmed().isEmpty()) {
        const QDateTime retryAt = QDateTime::fromString(
            QString::fromLatin1(response.retryAfter.trimmed()), Qt::RFC2822Date);
        if (retryAt.isValid()) {
            const qint64 seconds = QDateTime::currentDateTimeUtc().secsTo(retryAt.toUTC());
            delaySeconds = static_cast<int>(qBound<qint64>(
                static_cast<qint64>(0),
                seconds,
                static_cast<qint64>(std::numeric_limits<int>::max())));
        }
    }
    delaySeconds = qBound(
        0,
        delaySeconds,
        std::numeric_limits<int>::max());
    decision.retry = true;
    decision.delaySeconds = delaySeconds;
    return decision;
}
