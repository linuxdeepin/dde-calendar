// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbuscalendar_adaptor.h"
#include "calendarmainwindow.h"
#include "scheduledatamanage.h"

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QWidget>

// Add logging category
Q_LOGGING_CATEGORY(calendarAdaptor, "calendar.dbus.adaptor")

/*
 * Implementation of adaptor class CalendarAdaptor
 */

CalendarAdaptor::CalendarAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(calendarAdaptor) << "Initializing Calendar DBus Adaptor";
    // constructor
    setAutoRelaySignals(true);
}

CalendarAdaptor::~CalendarAdaptor()
{
    qCDebug(calendarAdaptor) << "Destroying Calendar DBus Adaptor";
    // destructor
}

void CalendarAdaptor::ActiveWindow()
{
    qCDebug(calendarAdaptor) << "Activating window";
    // handle method call com.deepin.Calendar.RaiseWindow
    QMetaObject::invokeMethod(parent(), "ActiveWindow");
}

void CalendarAdaptor::RaiseWindow()
{
    qCDebug(calendarAdaptor) << "Raising window";
    QWidget *pp = qobject_cast<QWidget *>(parent());
    //取消最小化状态
    pp->setWindowState(pp->windowState() & ~Qt::WindowState::WindowMinimized);
    pp->activateWindow();
    pp->raise();
}

void CalendarAdaptor::OpenSchedule(QString job)
{
    qCDebug(calendarAdaptor) << "Opening schedule with job:" << job;
    //更新对应的槽函数
    QMetaObject::invokeMethod(parent(), "slotOpenSchedule", Q_ARG(QString, job));
}



