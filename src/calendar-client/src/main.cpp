// SPDX-FileCopyrightText: 2017 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "environments.h"
#include "calendarmainwindow.h"
#include "exportedinterface.h"
#include "configsettings.h"
#include "accessible/accessible.h"
#include "tabletconfig.h"
#include "schedulemanager.h"
#include "commondef.h"
#include "logger.h"
#include "calendarmanage.h"
#include "lunarmanager.h"
#include "constants.h"

#include <DApplication>
#include <DLog>
#include <DGuiApplicationHelper>

#include <QDBusConnection>
#include <QTimer>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

static constexpr int LUNAR_INFO_READY_TIMEOUT_MS = 800;

int main(int argc, char *argv[])
{
    // 日志处理要放在app之前，否则QApplication内部可能进行了日志打印，导致环境变量设置不生效
    CalendarLogger("org.deepin.dde.calendar.client");

    // qCDebug(ClientLogger) << "Starting dde-calendar application";
    //在root下或者非deepin/uos环境下运行不会发生异常，需要加上XDG_CURRENT_DESKTOP=Deepin环境变量；
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        // qCInfo(ClientLogger) << "Setting XDG_CURRENT_DESKTOP to Deepin environment";
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
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
    //设置合理的超时时间，防止热启动卡死
    DGuiApplicationHelper::setSingleInstanceInterval(2000);
#endif

    if (DGuiApplicationHelper::setSingleInstance(app->applicationName(), DGuiApplicationHelper::UserScope)) {
        // qCDebug(ClientLogger) << "Initializing application as single instance";

        app->setOrganizationName("deepin");
        app->setApplicationName("dde-calendar");
        app->loadTranslator();
        qCInfo(ClientLogger) << "Application initialized with name: dde-calendar, version:" << VERSION;
#ifdef QT_DEBUG
        // 在开发调试时使用项目内的翻译文件
        auto tf = "../translations/dde-calendar_zh_CN";
        // qCDebug(ClientLogger) << "load translate" << tf;
        QTranslator translator;
        translator.load(tf);
        app->installTranslator(&translator);
#endif
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

        // Initialize logging system
        CalendarLogger::initLogger();

        bool isOk = false;
        int viewtype = CConfigSettings::getInstance()->value("base.view").toInt(&isOk);
        if (!isOk)
            viewtype = 2;

        // Register the caller identity before constructing the main window.
        // The constructor may issue D-Bus requests before the window is ready.
        QDBusConnection dbus = QDBusConnection::sessionBus();
        if (!dbus.registerService("com.deepin.Calendar")) {
            qCWarning(ClientLogger) << "Failed to register DBus service:" << dbus.lastError().message();
        }

        //为了与老版本配置兼容
        Calendarmainwindow ww(viewtype - 1);
        ExportedInterface einterface(&ww);
        einterface.registerAction("CREATE", "create a new schedule");
        einterface.registerAction("VIEW", "check a date on calendar");
        einterface.registerAction("QUERY", "find a schedule information");
        einterface.registerAction("CANCEL", "cancel a schedule");
        qCDebug(ClientLogger) << "DBus actions registered: CREATE, VIEW, QUERY, CANCEL";

        if (!dbus.registerObject("/com/deepin/Calendar", &ww)) {
            qCWarning(ClientLogger) << "Failed to register DBus object:" << dbus.lastError().message();
        }
        ww.slotTheme(DGuiApplicationHelper::instance()->themeType());
        //中文环境下等待预取的农历数据就绪后再显示窗口，避免首帧无农历、
        //数据返回后再补画；超时兜底保证服务异常时窗口仍能及时显示。
        if (CalendarManager::getInstance()->getShowLunar()) {
            // The callback is intentionally idempotent. The window context
            // removes the connection when it is destroyed, so no local
            // connection handle needs to be captured by reference.
            auto showOnce = [&ww]() {
                if (!ww.isVisible())
                    ww.show();
            };
            QTimer::singleShot(LUNAR_INFO_READY_TIMEOUT_MS, &ww, showOnce);

            int defaultViewIndex = viewtype - 1;
            if (defaultViewIndex < DDECalendar::CalendarYearWindow
                || defaultViewIndex > DDECalendar::CalendarDayWindow) {
                defaultViewIndex = DDECalendar::CalendarMonthWindow;
            }
            const QDate selectedDate = CalendarManager::getInstance()->getSelectDate();
            QDate requiredStart = selectedDate;
            QDate requiredEnd = selectedDate;
            switch (defaultViewIndex) {
            case DDECalendar::CalendarYearWindow:
                requiredStart = QDate(selectedDate.year(), 1, 1);
                requiredEnd = QDate(selectedDate.year(), 12, 31);
                break;
            case DDECalendar::CalendarMonthWindow: {
                const QVector<QDate> monthDates =
                    CalendarManager::getInstance()->getMonthDate(selectedDate.year(), selectedDate.month());
                requiredStart = monthDates.first();
                requiredEnd = monthDates.last();
                break;
            }
            case DDECalendar::CalendarWeekWindow: {
                const QVector<QDate> weekDates = CalendarManager::getInstance()->getWeekDate(selectedDate);
                requiredStart = weekDates.first();
                requiredEnd = weekDates.last();
                break;
            }
            case DDECalendar::CalendarDayWindow:
                break;
            default:
                break;
            }

            if (gLunarManager->hasHuangLiRange(requiredStart, requiredEnd)) {
                showOnce();
            } else {
                QObject::connect(gLunarManager, &LunarManager::lunarInfoReady,
                                 &ww, [showOnce, requiredStart, requiredEnd](const QDate &startDate,
                                                                              const QDate &endDate) {
                    if (startDate <= requiredStart && endDate >= requiredEnd) {
                        showOnce();
                    }
                }, Qt::QueuedConnection);
            }
        } else {
            ww.show();
        }

        // Defer account data loading to event loop for faster startup
        QMetaObject::invokeMethod(gAccountManager, &AccountManager::resetAccount,
                                   Qt::QueuedConnection);

        qCInfo(ClientLogger) << "dde-calendar application started successfully";
        return app->exec();
    }
    qCWarning(ClientLogger) << "Application startup failed: Another instance is already running";
    return 0;
}
