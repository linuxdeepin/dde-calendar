// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldaveventreconciler.h"

#include <QSet>

namespace {

QHash<QString, DCalDavEventMappingInfo> mappingByHref(const DCalDavEventMappingInfo::List &mappings)
{
    QHash<QString, DCalDavEventMappingInfo> result;
    for (const DCalDavEventMappingInfo &mapping : mappings) {
        if (!mapping.href.isEmpty()) {
            result.insert(mapping.href, mapping);
        }
    }
    return result;
}

QHash<QString, DCalDavEventMappingInfo> mappingByUid(const DCalDavEventMappingInfo::List &mappings)
{
    QHash<QString, DCalDavEventMappingInfo> result;
    QSet<QString> duplicateUids;
    for (const DCalDavEventMappingInfo &mapping : mappings) {
        if (mapping.uid.isEmpty() || duplicateUids.contains(mapping.uid)) {
            continue;
        }
        if (result.contains(mapping.uid)) {
            result.remove(mapping.uid);
            duplicateUids.insert(mapping.uid);
            continue;
        }
        result.insert(mapping.uid, mapping);
    }
    return result;
}

} // namespace

bool DCalDavEventReconciler::buildActions(const DCalDavCalendarQuery::RemoteEventList &remoteEvents,
                                          const DCalDavEventMappingInfo::List &existingMappings,
                                          ActionList &actions, QString *errorMessage)
{
    QSet<QString> remoteHrefs;
    const QHash<QString, DCalDavEventMappingInfo> mappings = mappingByHref(existingMappings);
    const QHash<QString, DCalDavEventMappingInfo> mappingsByRemoteUid = mappingByUid(existingMappings);
    QSet<QString> matchedMappingHrefs;
    ActionList parsedActions;
    for (const DCalDavCalendarQuery::RemoteEvent &remoteEvent : remoteEvents) {
        if (remoteEvent.href.isEmpty() || remoteHrefs.contains(remoteEvent.href)) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Remote event href is empty or duplicated.");
            }
            return false;
        }
        remoteHrefs.insert(remoteEvent.href);

        auto mapping = mappings.constFind(remoteEvent.href);
        if (mapping == mappings.constEnd() && !remoteEvent.uid.isEmpty()) {
            const auto uidMapping = mappingsByRemoteUid.constFind(remoteEvent.uid);
            if (uidMapping != mappingsByRemoteUid.constEnd()) {
                mapping = mappings.constFind(uidMapping->href);
            }
        }
        if (remoteEvent.deleted) {
            if (mapping != mappings.constEnd() && !matchedMappingHrefs.contains(mapping->href)) {
                parsedActions.append({DeleteAction, remoteEvent, mapping.value()});
                matchedMappingHrefs.insert(mapping->href);
            }
            continue;
        }

        if (mapping == mappings.constEnd()) {
            parsedActions.append({CreateAction, remoteEvent, DCalDavEventMappingInfo()});
        } else {
            parsedActions.append({UpdateAction, remoteEvent, mapping.value()});
            matchedMappingHrefs.insert(mapping->href);
        }
    }

    actions = parsedActions;
    return true;
}
