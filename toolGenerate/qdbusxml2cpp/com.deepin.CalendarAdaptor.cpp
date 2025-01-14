/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp ./dde-calendar/calendar-client/assets/dbus/com.deepin.Calendar.xml -a ./dde-calendar/toolGenerate/qdbusxml2cpp/com.deepin.CalendarAdaptor -i ./dde-calendar/toolGenerate/qdbusxml2cpp/com.deepin.Calendar.h
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "./dde-calendar/toolGenerate/qdbusxml2cpp/com.deepin.CalendarAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class CalendarAdaptor
 */

CalendarAdaptor::CalendarAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

CalendarAdaptor::~CalendarAdaptor()
{
    // destructor
}

void CalendarAdaptor::OpenSchedule(const QString &job)
{
    // handle method call com.deepin.dde.Calendar.OpenSchedule
    QMetaObject::invokeMethod(parent(), "OpenSchedule", Q_ARG(QString, job));
}

void CalendarAdaptor::RaiseWindow()
{
    // handle method call com.deepin.dde.Calendar.RaiseWindow
    QMetaObject::invokeMethod(parent(), "RaiseWindow");
}

