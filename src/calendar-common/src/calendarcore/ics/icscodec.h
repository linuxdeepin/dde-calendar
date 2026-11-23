// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "calstore.h"

namespace CalendarCore {

class IcsCodec
{
public:
    enum class ReadMode { Strict, ExistingData };

    // Callers serialize access to libical's shared timezone state.
    // When provided, diagnostic reports a failure or a recoverable parse warning.
    bool load(const CalStore::Ptr &calendar, const QString &path, QString *diagnostic = nullptr) const;
    bool save(const CalStore::Ptr &calendar, const QString &path) const;
    bool fromString(const CalStore::Ptr &calendar, const QString &text, bool deleted = false,
                    QString *diagnostic = nullptr, ReadMode mode = ReadMode::Strict) const;
    bool fromString(RRule *rule, const QString &text) const;
    QString toString(const CalStore::Ptr &calendar) const;
    QString toString(const RRule *rule) const;

private:
    struct Reader;
    struct Writer;
};

} // namespace CalendarCore
