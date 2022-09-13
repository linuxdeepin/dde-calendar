// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <DLog>
#include <QDBusConnection>
#include <QDBusError>
#include <QTranslator>
#include "calendarservice.h"

const static QString CalendarServiceTranslationsDir = "/usr/share/dde-calendar/translations";

bool loadTranslator(QCoreApplication *app, QList<QLocale> localeFallback = QList<QLocale>() << QLocale::system())
{
    bool bsuccess = false;
    for (auto &locale : localeFallback) {
        QString translateFilename = QString("%1_%2").arg(app->applicationName()).arg(locale.name());
        QString translatePath = QString("%1/%2.qm").arg(CalendarServiceTranslationsDir).arg(translateFilename);
        if (QFile(translatePath).exists()) {
            QTranslator *translator = new QTranslator(app);
            translator->load(translatePath);
            app->installTranslator(translator);
            bsuccess = true;
        }
        QStringList parseLocalNameList = locale.name().split("_", QString::SkipEmptyParts);
        if (parseLocalNameList.length() > 0 && !bsuccess) {
            translateFilename = QString("%1_%2").arg(app->applicationName()).arg(parseLocalNameList.at(0));
            QString parseTranslatePath = QString("%1/%2.qm").arg(CalendarServiceTranslationsDir).arg(translateFilename);
            if (QFile::exists(parseTranslatePath)) {
                QTranslator *translator = new QTranslator(app);
                translator->load(parseTranslatePath);
                app->installTranslator(translator);
                bsuccess = true;
            }
        }
    }

    return bsuccess;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("dde-calendar-service");

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    if (!loadTranslator(&a)) {
        qDebug() << "loadtranslator failed";
    }
    qDebug() << "write log to" << Dtk::Core::DLogManager::getlogFilePath();
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService(CalendarServiceName)) {
        qCritical() << "registerService failed:" << sessionBus.lastError();
        exit(0x0001);
    }
    CalendarService service;

    qDebug() << "sessionBus.registerService success" << Dtk::Core::DLogManager::getlogFilePath();
    if (!sessionBus.registerObject(CalendarPath,
                                   &service,
                                   QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties)) {
        qCritical() << "registerObject failed:" << sessionBus.lastError();
        exit(0x0002);
    }
    return a.exec();
}
