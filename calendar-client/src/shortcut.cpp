// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "shortcut.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(shortcutLog, "calendar.shortcut")

Shortcut::Shortcut(QObject *parent)
    : QObject(parent)
{
    qCDebug(shortcutLog) << "Initializing Shortcut";
    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;
    ShortcutGroup group4;

    group1.groupName = "";
    group1.groupItems << ShortcutItem(tr("Help"),  "F1") <<
                      ShortcutItem(tr("Delete event"),       "Delete");
    qCDebug(shortcutLog) << "Created group1 with" << group1.groupItems.size() << "items";

    group2.groupName = "";
    group2.groupItems << ShortcutItem(tr("Copy"), "Ctrl+C") <<
                      ShortcutItem(tr("Cut"),  "Ctrl+X") <<
                      ShortcutItem(tr("Paste"), "Ctrl+V") ;
    qCDebug(shortcutLog) << "Created group2 with" << group2.groupItems.size() << "items";

    group3.groupName = "";
    group3.groupItems << ShortcutItem(tr("Delete"), "Delete") <<
                      ShortcutItem(tr("Select all"), "Ctrl+A");
    qCDebug(shortcutLog) << "Created group3 with" << group3.groupItems.size() << "items";

    m_shortcutGroups << group1 << group2 << group3;
    qCDebug(shortcutLog) << "Added" << m_shortcutGroups.size() << "groups to shortcut groups";

    //convert to json object
    QJsonArray jsonGroups;
    for (auto scg : m_shortcutGroups) {
        QJsonObject jsonGroup;
        QJsonArray jsonGroupItems;
        for (auto sci : scg.groupItems) {
            QJsonObject jsonItem;
            jsonItem.insert("name", sci.name);
            jsonItem.insert("value", sci.value);
            jsonGroupItems.append(jsonItem);
        }
        jsonGroup.insert("groupItems", jsonGroupItems);
        jsonGroups.append(jsonGroup);
    }
    m_shortcutObj.insert("shortcut", jsonGroups);
    qCDebug(shortcutLog) << "Converted shortcut groups to JSON object";
}

QString Shortcut::toStr()
{
    QJsonDocument doc(m_shortcutObj);
    QString jsonStr = doc.toJson().data();
    qCDebug(shortcutLog) << "Converting shortcut object to string, length:" << jsonStr.length();
    return jsonStr;
}
