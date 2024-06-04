// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "shortcut.h"

Shortcut::Shortcut(QObject *parent)
    : QObject(parent)
{
    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;

    group1.groupName = tr("System");
    group1.groupItems << ShortcutItem(tr("Display shortcuts"), "Ctrl+Shift+?") <<
                      ShortcutItem(tr("Help"),  "F1") <<
                      ShortcutItem(tr("Delete event"), "Delete");

    group2.groupName = tr("File");
    group2.groupItems << ShortcutItem(tr("Copy"), "Ctrl+C") <<
                      ShortcutItem(tr("Cut"),  "Ctrl+X") <<
                      ShortcutItem(tr("Paste"), "Ctrl+V") <<
                      ShortcutItem(tr("Delete"), "Delete") <<
                      ShortcutItem(tr("Select all"), "Ctrl+A");

    group3.groupName = tr("Search");
    group3.groupItems << ShortcutItem(tr("Search"), "Ctrl+F");
    m_shortcutGroups << group1 << group2 << group3;
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
        jsonGroup.insert("groupName", scg.groupName);
        jsonGroups.append(jsonGroup);
    }
    m_shortcutObj.insert("shortcut", jsonGroups);
}
QString Shortcut::toStr()
{
    QJsonDocument doc(m_shortcutObj);
    return doc.toJson().data();
}
