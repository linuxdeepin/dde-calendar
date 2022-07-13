/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dservicemanager.h"
#include "ddatabasemanagement.h"

#include <DLog>

#include <QDBusConnection>
#include <QDBusError>
#include <QTranslator>
#include <QCoreApplication>

#ifndef LINGLONG_PREFIX
#define LINGLONG_PREFIX "/usr/"
#endif

const static QString CalendarServiceTranslationsDir = QString("%1share/dde-calendar/translations").arg(LINGLONG_PREFIX);

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

    //加载翻译
    if (!loadTranslator(&a)) {
        qDebug() << "loadtranslator failed";
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
