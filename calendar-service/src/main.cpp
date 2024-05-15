// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicemanager.h"
#include "ddatabasemanagement.h"
#include "loggerService.h"
#include "ddlogService.h"

#include <DLog>

#include <QDBusConnection>
#include <QDBusError>
#include <QTranslator>
#include <QCoreApplication>

using namespace DDLOG_SERVICE;


bool loadTranslator(QCoreApplication *app, QList<QLocale> localeFallback = QList<QLocale>() << QLocale::system())
{
    bool bsuccess = false;
    QString CalendarServiceTranslationsDir = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  		    QString("dde-calendar/translations"),
								    QStandardPaths::LocateDirectory);
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
    MLogger(); // 日志处理要放在app之前，否则QApplication
               // 内部可能进行了日志打印，导致环境变量设置不生效

    QCoreApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("dde-calendar-service");

    // DtkCore 5.6.8版本，支持journal方式日志存储，
    #if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
        DLogManager::registerJournalAppender();// 开启journal日志存储
    #ifdef QT_DEBUG
        DLogManager::registerConsoleAppender();
    #endif
    // 为保证兼容性，在该版本以下，自研应用应采用原有log文件日志输出方式保存日志
    #else
        DLogManager::registerConsoleAppender();
        DLogManager::registerFileAppender();
    #endif

    //加载翻译
    if (!loadTranslator(&a)) {
        qCDebug(calendar_service) << "loadtranslator failed";
    }

    DDataBaseManagement dbManagement;

    DServiceManager serviceManager;

    //如果存在迁移，则更新提醒
    if(dbManagement.hasTransfer()){
        //延迟处理
        QTimer::singleShot(0,[&](){
          serviceManager.updateRemindJob();
        });
    }

    return a.exec();
}
