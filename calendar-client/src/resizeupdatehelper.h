// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef RESIZEUPDATEHELPER_H
#define RESIZEUPDATEHELPER_H

#include <QApplication>

namespace DDECalendar {

static const int ResizeUpdateDelay = 120;

inline bool shouldDelayResizeUpdate()
{
    return QApplication::platformName().startsWith("wayland", Qt::CaseInsensitive);
}

} // namespace DDECalendar

#endif // RESIZEUPDATEHELPER_H
