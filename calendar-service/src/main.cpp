// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicemanager.h"
#include "ddatabasemanagement.h"
#include "commondef.h"
#include <DLog>
#include <QDebug>
#include <QLoggingCategory>

#include <QDBusConnection>
#include <QDBusError>
#include <QTranslator>
#include <QCoreApplication>

Q_LOGGING_CATEGORY(MainLog, "calendar.main")

bool loadTranslator(QCoreApplication *app, QList<QLocale> localeFallback = QList<QLocale>() << QLocale::system())
{
    qCDebug(MainLog) << "Loading translations for locales:" << localeFallback;
    bool bsuccess = false;
    QString CalendarServiceTranslationsDir = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  		    QString("dde-calendar/translations"),
								    QStandardPaths::LocateDirectory);
    qCDebug(MainLog) << "Translation directory:" << CalendarServiceTranslationsDir;
    
    for (auto &locale : localeFallback) {
        QString translateFilename = QString("%1_%2").arg(app->applicationName()).arg(locale.name());
        QString translatePath = QString("%1/%2.qm").arg(CalendarServiceTranslationsDir).arg(translateFilename);
        qCDebug(MainLog) << "Trying translation file:" << translatePath;
        
        if (QFile(translatePath).exists()) {
            QTranslator *translator = new QTranslator(app);
            if (translator->load(translatePath)) {
                qCInfo(MainLog) << "Successfully loaded translation for" << locale.name();
                app->installTranslator(translator);
                bsuccess = true;
            } else {
                qCWarning(MainLog) << "Failed to load translation file:" << translatePath;
            }
        }
        
        QStringList parseLocalNameList = locale.name().split("_", Qt::SkipEmptyParts);
        if (parseLocalNameList.length() > 0 && !bsuccess) {
            translateFilename = QString("%1_%2").arg(app->applicationName()).arg(parseLocalNameList.at(0));
            QString parseTranslatePath = QString("%1/%2.qm").arg(CalendarServiceTranslationsDir).arg(translateFilename);
            qCDebug(MainLog) << "Trying fallback translation file:" << parseTranslatePath;
            
            if (QFile::exists(parseTranslatePath)) {
                QTranslator *translator = new QTranslator(app);
                if (translator->load(parseTranslatePath)) {
                    qCInfo(MainLog) << "Successfully loaded fallback translation for" << parseLocalNameList.at(0);
                    app->installTranslator(translator);
                    bsuccess = true;
                } else {
                    qCWarning(MainLog) << "Failed to load fallback translation file:" << parseTranslatePath;
                }
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

    qCInfo(MainLog) << "Starting dde-calendar-service";
    qCDebug(MainLog) << "Application name:" << a.applicationName();
    qCDebug(MainLog) << "Organization name:" << a.organizationName();

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();
    Dtk::Core::DLogManager::registerJournalAppender();

    //加载翻译
    if (!loadTranslator(&a)) {
        qCWarning(MainLog) << "Failed to load translations";
    }

    DDataBaseManagement dbManagement;

    DServiceManager serviceManager;

    //如果存在迁移，则更新提醒
    if(dbManagement.hasTransfer()){
        qCInfo(MainLog) << "Database migration detected, updating remind jobs";
        //延迟处理
        QTimer::singleShot(0,[&](){
          serviceManager.updateRemindJob();
        });
    }
    qCInfo(MainLog) << "dde-calendar-service started successfully";
    return a.exec();
}
