// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "environments.h"
#include "calendarmainwindow.h"
#include "exportedinterface.h"
#include "configsettings.h"
#include "accessible/accessible.h"
#include "tabletconfig.h"
#include "schedulemanager.h"
#include "loggerClient.h"
#include "ddlogClient.h"

#include <DApplication>
#include <DLog>
#include <DGuiApplicationHelper>
#include <DApplicationSettings>
#include <DWidgetUtil>

#include <QDBusConnection>
#include <QElapsedTimer>

using namespace DDLOG_CLIENT;
DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    MLogger();// 日志处理要放在app之前，否则QApplication
              // 内部可能进行了日志打印，导致环境变量设置不生效

    QElapsedTimer timer;
    timer.start();
    QElapsedTimer ttimer;
    ttimer.start();

    //在root下或者非deepin/uos环境下运行不会发生异常，需要加上XDG_CURRENT_DESKTOP=Deepin环境变量；
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    //在创建窗口前开启ForceRasterWidgets，让Qt不要创建gl窗口。
    QApplication::setAttribute(Qt::AA_ForceRasterWidgets, true);

    //适配deepin-turbo启动加速
    DApplication *app = nullptr;
#if(DTK_VERSION < DTK_VERSION_CHECK(5,4,0,0))
    app = new DApplication(argc, argv);
#else
    app = DApplication::globalApplication(argc, argv);
#endif

    app->setAutoActivateWindows(true);


    //如果dtk版本为5.2.0.1以上则使用新的dtk接口
#if (DTK_VERSION > DTK_VERSION_CHECK(5, 2, 0, 1))
    //设置为-1将永久等待
    DGuiApplicationHelper::setSingleInstanceInterval(-1);
#endif

    if (DGuiApplicationHelper::setSingleInstance(app->applicationName(), DGuiApplicationHelper::UserScope)) {
        //准备数据
        gAccountManager->resetAccount();

        app->setOrganizationName("deepin");
        app->setApplicationName("dde-calendar");
        app->loadTranslator();
        app->setApplicationVersion(VERSION);
        // meta information that necessary to create the about dialog.
        app->setProductName(QApplication::translate("CalendarWindow", "Calendar"));
        QIcon t_icon = QIcon::fromTheme("dde-calendar");
        app->setProductIcon(t_icon);
        app->setApplicationDescription(QApplication::translate("CalendarWindow", "Calendar is a tool to view dates, and also a smart daily planner to schedule all things in life. "));
        app->setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/dde-calendar");
        //命令行参数
        QCommandLineParser _commandLine; //建立命令行解析
        _commandLine.addHelpOption(); //增加-h/-help解析命令
        _commandLine.addVersionOption(); //增加-v 解析命令
        _commandLine.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
        _commandLine.process(*app);

        app->setAutoActivateWindows(true);

        qCInfo(calendar_client) << "main config" << timer.elapsed();
        timer.restart();

        bool isOk = false;
        int viewtype = CConfigSettings::getInstance()->value("base.view").toInt(&isOk);
        if (!isOk)
            viewtype = 2;

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

        DLogManager::registerConsoleAppender();

        //获取应用配置
        DApplicationSettings applicationset(app);

        qCInfo(calendar_client) << "main viewtype" << timer.elapsed();
        timer.restart();

        //为了与老版本配置兼容
        Calendarmainwindow ww(viewtype - 1);

        qCInfo(calendar_client) << "main created" << timer.elapsed();
        qCWarning(calendar_client) << "!!!!!!!!!!!!!!";
        timer.restart();
        ww.show();

        qCInfo(calendar_client) << "main shown" << timer.elapsed();
        qCInfo(calendar_client) << "main" << ttimer.elapsed();
        return app->exec();
    }
    return 0;
}
