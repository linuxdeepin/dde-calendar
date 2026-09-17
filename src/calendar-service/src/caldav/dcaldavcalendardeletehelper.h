// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVCALENDARDELETEHELPER_H
#define DCALDAVCALENDARDELETEHELPER_H

#include "dcaldavcalendarinfo.h"
#include "dcaldavcategoryinfo.h"

#include <QStringList>

class DAccountDataBase;
class DAccountManagerDataBase;

namespace DCalDavCalendarDeleteHelper {

QStringList collectTypeIDs(const QString &primaryTypeID,
                           const DCalDavCategoryInfo::List &categories);
QStringList collectScheduleIDs(DAccountDataBase *localDatabase,
                               const QStringList &typeIDs);
bool deleteScheduleTypes(DAccountDataBase *localDatabase,
                         const QStringList &typeIDs,
                         bool hardDelete,
                         bool deleteColors);
bool deleteOutboxItems(DAccountManagerDataBase *accountManagerDatabase,
                       const QString &accountID,
                       const QStringList &scheduleIDs);
bool deleteRemoteManagerData(DAccountManagerDataBase *accountManagerDatabase,
                             const QString &accountID,
                             const QString &calendarID,
                             const QStringList &scheduleIDs);
bool persistLocalDeleteState(DAccountManagerDataBase *accountManagerDatabase,
                             const QString &accountID,
                             const DCalDavCalendarInfo &calendar,
                             const QStringList &scheduleIDs);

} // namespace DCalDavCalendarDeleteHelper

#endif // DCALDAVCALENDARDELETEHELPER_H
