// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicemanager.h"
#include "ddatabasemanagement.h"
#include "commondef.h"
#include "logger.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QTranslator>
#include <QCoreApplication>
#include <QTimer>
#include <DLog>

DCORE_USE_NAMESPACE

bool loadTranslator(QCoreApplication *app, QList<QLocale> localeFallback = QList<QLocale>() << QLocale::system())
{
    qCDebug(ServiceLogger) << "Loading translator...";
    bool bsuccess = false;
    QString CalendarServiceTranslationsDir = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  		    QString("dde-calendar/translations"),
								    QStandardPaths::LocateDirectory);
    qCDebug(ServiceLogger) << "Translations directory:" << CalendarServiceTranslationsDir;
    for (auto &locale : localeFallback) {
        QString translateFilename = QString("%1_%2").arg(app->applicationName()).arg(locale.name());
        QString translatePath = QString("%1/%2.qm").arg(CalendarServiceTranslationsDir).arg(translateFilename);
        // qCDebug(ServiceLogger) << "Attempting to load translation file:" << translatePath;
        if (QFile(translatePath).exists()) {
            QTranslator *translator = new QTranslator(app);
            translator->load(translatePath);
            app->installTranslator(translator);
            bsuccess = true;
        }
        QStringList parseLocalNameList = locale.name().split("_", Qt::SkipEmptyParts);
        if (parseLocalNameList.length() > 0 && !bsuccess) {
            translateFilename = QString("%1_%2").arg(app->applicationName()).arg(parseLocalNameList.at(0));
            QString parseTranslatePath = QString("%1/%2.qm").arg(CalendarServiceTranslationsDir).arg(translateFilename);
            // qCDebug(ServiceLogger) << "Attempting to load fallback translation file:" << parseTranslatePath;
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
    // 日志处理要放在app之前，否则QApplication内部可能进行了日志打印，导致环境变量设置不生效
    CalendarLogger("org.deepin.dde.calendar.service");
    
    qCInfo(ServiceLogger) << "Starting dde-calendar-service.";
    QCoreApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("dde-calendar-service");

    // Initialize logging system
    CalendarLogger::initLogger();

    //加载翻译
    if (!loadTranslator(&a)) {
        qCWarning(ServiceLogger) << "Translation load failed.";
    }

    qCDebug(ServiceLogger) << "Initializing DDataBaseManagement...";
    DDataBaseManagement dbManagement;
    qCDebug(ServiceLogger) << "DDataBaseManagement initialized.";

    qCDebug(ServiceLogger) << "Initializing DServiceManager...";
    DServiceManager serviceManager;
    qCDebug(ServiceLogger) << "DServiceManager initialized.";

    //如果存在迁移，则更新提醒
    if(dbManagement.hasTransfer()){
        qCInfo(ServiceLogger) << "Database transfer detected, scheduling remind job update.";
        //延迟处理
        QTimer::singleShot(0,[&](){
          qCDebug(ServiceLogger) << "Executing delayed remind job update.";
          serviceManager.updateRemindJob();
          qCDebug(ServiceLogger) << "Delayed remind job update finished.";
        });
    }
    qCInfo(ServiceLogger) << "dde-calendar-service start.";
    return a.exec();
}
