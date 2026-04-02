// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <QString>
#include <QStandardPaths>
#include <QLoggingCategory>

// Qt5/Qt6 兼容性宏
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #define QT_SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
    #define QT_SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif

const QString CalendarServiceName = "com.deepin.dataserver.Calendar";
const QString CalendarPath = "/com/deepin/dataserver/Calendar";

const QLoggingCategory CommonLogger("org.deepin.dde.calendar");
const QLoggingCategory ClientLogger("org.deepin.dde.calendar.client");
const QLoggingCategory ServiceLogger("org.deepin.dde.calendar.service");
const QLoggingCategory PluginLogger("org.deepin.dde.calendar.plugin");


#endif // COMMONDEF_H
