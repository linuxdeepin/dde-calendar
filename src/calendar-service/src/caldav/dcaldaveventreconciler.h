// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVEVENTRECONCILER_H
#define DCALDAVEVENTRECONCILER_H

#include "dcaldavcalendarquery.h"
#include "dcaldaveventmappinginfo.h"

#include <QHash>
#include <QString>
#include <QVector>

class DCalDavEventReconciler
{
public:
    enum ActionType {
        CreateAction,
        UpdateAction,
        DeleteAction,
    };

    struct Action {
        ActionType type;
        DCalDavCalendarQuery::RemoteEvent remoteEvent;
        DCalDavEventMappingInfo existingMapping;
    };

    typedef QVector<Action> ActionList;

    /**
     * @brief Reconciles remote events with persisted mappings into local actions.
     * @param remoteEvents Events returned by the CalDAV server.
     * @param existingMappings Locally persisted remote-event mappings.
     * @param actions Output list of create, update, or delete actions.
     * @param errorMessage Optional validation or duplicate-event error output.
     * @return true when all remote events are valid and actions were generated.
     */
    static bool buildActions(const DCalDavCalendarQuery::RemoteEventList &remoteEvents,
                             const DCalDavEventMappingInfo::List &existingMappings,
                             ActionList &actions, QString *errorMessage = nullptr);
};

#endif // DCALDAVEVENTRECONCILER_H
