// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavcolorallocator.h"

#include "daccountdatabase.h"
#include "dscheduletype.h"
#include "units.h"

#include <QSet>

namespace {

QStringList availableColors(const QSet<QString> &usedColors)
{
    QStringList colors;
    for (const QString &color : GCalDavTypeColors) {
        if (!usedColors.contains(color.toLower())) {
            colors.append(color);
        }
    }
    return colors;
}

QSet<QString> usedAccountColors(DAccountDataBase *database)
{
    QSet<QString> usedColors;
    if (database == nullptr) {
        return usedColors;
    }

    const DScheduleType::List types = database->getScheduleTypeList();
    for (const DScheduleType::Ptr &type : types) {
        if (!type.isNull() && !type->typePath().isEmpty()) {
            usedColors.insert(type->getColorCode().trimmed().toLower());
        }
    }
    return usedColors;
}

} // namespace

QString DCalDavColorAllocator::nextColor(DAccountDataBase *database)
{
    const QSet<QString> usedColors = usedAccountColors(database);
    const QStringList available = availableColors(usedColors);
    if (!available.isEmpty()) {
        return available.first();
    }

    // Keep allocation stable and follow the documented nine-color order.
    return GCalDavTypeColors.at(usedColors.size() % GCalDavTypeColors.size());
}
