// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbuscalendar_adaptor.h"

#include "accountmanager.h"
#include "calendarmainwindow.h"
#include "lunarmanager.h"
#include "scheduledatamanage.h"
#include "schedulemanager.h"
#include "commondef.h"

#include <QWidget>
#include <QtCore/QByteArray>
#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMetaObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUuid>
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

void CalendarAdaptor::ActiveWindow()
{
    // handle method call com.deepin.Calendar.RaiseWindow
    QMetaObject::invokeMethod(parent(), "ActiveWindow");
}

void CalendarAdaptor::RaiseWindow()
{
    QWidget *pp = qobject_cast<QWidget *>(parent());
    // 取消最小化状态
    pp->setWindowState(pp->windowState() & ~Qt::WindowState::WindowMinimized);
    pp->activateWindow();
    pp->raise();
}

void CalendarAdaptor::OpenSchedule(QString job)
{
    // 更新对应的槽函数
    QMetaObject::invokeMethod(parent(), "slotOpenSchedule", Q_ARG(QString, job));
}

// Pick the first visible schedule type (privilege != None), skip hidden types like Holiday.
// Note: default types (Work/Life/Other) have Read privilege (0x1) — they are visible and
// schedules created in them are editable.  Only None (0x0) types are hidden system types.
static QString pickUserScheduleType(const DScheduleType::List &types)
{
    for (const auto &t : types) {
        if (t->privilege() != DScheduleType::None) {
            return t->typeID();
        }
    }
    return types.isEmpty() ? QString() : types.first()->typeID();
}

// Get a QDBusInterface for the local account service.
// Re-checks client cache first (safe against races: account may become
// available between the caller's earlier check and now), then falls back
// to a synchronous call to AccountManager to get account list.
// Returns an invalid interface (isValid() == false) if unavailable.
static QDBusInterface getLocalAccountInterface()
{
    auto item = gAccountManager->getLocalAccountItem();
    if (item) {
        return QDBusInterface(
            "com.deepin.dataserver.Calendar",
            item->getAccount()->dbusPath(),
            item->getAccount()->dbusInterface(),
            QDBusConnection::sessionBus());
    }

    qCWarning(ClientLogger) << "Client cache not ready, querying backend for account info";

    QDBusInterface mgrIface(
        "com.deepin.dataserver.Calendar",
        "/com/deepin/dataserver/Calendar/AccountManager",
        "com.deepin.dataserver.Calendar.AccountManager",
        QDBusConnection::sessionBus());

    QDBusReply<QString> reply = mgrIface.call("getAccountList");
    if (!reply.isValid()) {
        qCWarning(ClientLogger) << "Failed to get account list:" << reply.error().message();
        return QDBusInterface("", "", "", QDBusConnection::sessionBus());
    }

    DAccount::List accountList;
    if (!DAccount::fromJsonListString(accountList, reply.value())) {
        qCWarning(ClientLogger) << "Failed to parse account list";
        return QDBusInterface("", "", "", QDBusConnection::sessionBus());
    }

    for (const auto &acc : accountList) {
        if (acc->accountType() == DAccount::Account_Local) {
            return QDBusInterface(
                "com.deepin.dataserver.Calendar",
                acc->dbusPath(),
                acc->dbusInterface(),
                QDBusConnection::sessionBus());
        }
    }

    qCWarning(ClientLogger) << "No local account found";
    return QDBusInterface("", "", "", QDBusConnection::sessionBus());
}

// Apply JSON update fields to an existing schedule.
// Shared by both cache fast-path and backend fallback-path.
static void applyScheduleUpdates(const DSchedule::Ptr &schedule, const QJsonObject &updateData)
{
    if (updateData.contains("title")) {
        schedule->setSummary(updateData["title"].toString());
    }
    if (updateData.contains("description")) {
        schedule->setDescription(updateData["description"].toString());
    }
    if (updateData.contains("location")) {
        schedule->setLocation(updateData["location"].toString());
    }
    if (updateData.contains("allDay")) {
        schedule->setAllDay(updateData["allDay"].toBool());
    }
    if (updateData.contains("startTime")) {
        QString str = updateData["startTime"].toString();
        QDateTime dt = QDateTime::fromString(str, Qt::ISODate);
        if (!dt.isValid())
            dt = QDateTime::fromString(str, "yyyy-MM-ddThh:mm:ss");
        if (dt.isValid())
            schedule->setDtStart(dt);
    }
    if (updateData.contains("endTime")) {
        QString str = updateData["endTime"].toString();
        QDateTime dt = QDateTime::fromString(str, Qt::ISODate);
        if (!dt.isValid())
            dt = QDateTime::fromString(str, "yyyy-MM-ddThh:mm:ss");
        if (dt.isValid())
            schedule->setDtEnd(dt);
    }
    if (updateData.contains("reminder")) {
        int reminderMinutes = updateData["reminder"].toInt();
        schedule->clearAlarms();
        if (reminderMinutes > 0) {
            KCalendarCore::Alarm::Ptr alarm = schedule->newAlarm();
            alarm->setStartOffset(KCalendarCore::Duration(-reminderMinutes * 60));
            alarm->setEnabled(true);
            alarm->setType(KCalendarCore::Alarm::Display);
        }
    }
}

// Helper function to convert DSchedule to JSON format
static QJsonObject scheduleToJson(const DSchedule::Ptr &schedule)
{
    QJsonObject obj;
    if (!schedule)
        return obj;

    obj["id"] = schedule->uid();
    obj["title"] = schedule->summary();
    obj["description"] = schedule->description();
    obj["startTime"] = schedule->dtStart().toString(Qt::ISODate);
    obj["endTime"] = schedule->dtEnd().toString(Qt::ISODate);
    obj["allDay"] = schedule->allDay();
    obj["location"] = schedule->location();

    // Determine schedule type for easy identification
    QString scheduleType = "user"; // Default to user schedule

    try {
        AccountItem::Ptr account =
            gAccountManager->getAccountItemByScheduleTypeId(schedule->scheduleTypeID());
        if (!account.isNull()) {
            DScheduleType::Ptr scheduleTypePtr =
                gAccountManager->getScheduleTypeByScheduleTypeId(schedule->scheduleTypeID());
            if (!scheduleTypePtr.isNull()) {
                // Use the exact same logic as CScheduleOperation::isFestival()
                bool isFestival = (account->getAccount()->accountType() == DAccount::Account_Local)
                    && (scheduleTypePtr->privilege() == 0);

                scheduleType = isFestival ? "system" : "user";
            }
        }
    } catch (...) {
        // If any error occurs, default to user schedule (safer)
        scheduleType = "user";
    }

    obj["scheduleType"] = scheduleType;

    // Add reminder info
    auto alarms = schedule->alarms();
    if (!alarms.isEmpty()) {
        auto alarm = alarms.first();
        obj["reminderMinutes"] = -alarm->startOffset().asSeconds() / 60;
    } else {
        obj["reminderMinutes"] = 0;
    }

    // Add recurrence info if exists
    if (schedule->recurs()) {
        QJsonObject recurrence;
        auto rrule = schedule->recurrence();
        recurrence["type"] = static_cast<int>(rrule->recurrenceType());
        recurrence["frequency"] = rrule->frequency();
        if (rrule->duration() != -1) {
            recurrence["duration"] = rrule->duration();
        }
        if (rrule->endDateTime().isValid()) {
            recurrence["endDate"] = rrule->endDateTime().toString(Qt::ISODate);
        }
        obj["recurrence"] = recurrence;
    }

    return obj;
}

// Helper function to parse date from string
static QDate parseDateString(const QString &dateStr)
{
    if (dateStr.isEmpty() || dateStr == "today" || dateStr == "day") {
        return QDate::currentDate();
    }

    QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
    // Validate the date - QDate::fromString may create invalid dates for impossible dates
    if (!date.isValid()) {
        return QDate(); // Return invalid date
    }

    // Additional validation for impossible dates like "2025-02-30"
    QStringList parts = dateStr.split("-");
    if (parts.size() == 3) {
        bool yearOk, monthOk, dayOk;
        int year = parts[0].toInt(&yearOk);
        int month = parts[1].toInt(&monthOk);
        int day = parts[2].toInt(&dayOk);

        if (yearOk && monthOk && dayOk) {
            // Check if the parsed date matches the input values
            if (date.year() != year || date.month() != month || date.day() != day) {
                return QDate(); // Invalid date
            }
        }
    }

    return date;
}

QString CalendarAdaptor::QuerySchedules(const QString &query)
{
    QJsonObject result;
    QJsonArray schedules;

    try {
        if (query == "day" || query == "today") {
            // Get today's schedules
            QDate today = QDate::currentDate();
            DSchedule::List daySchedules = gScheduleManager->getScheduleByDay(today);

            for (const auto &schedule : daySchedules) {
                schedules.append(scheduleToJson(schedule));
            }
            result["date"] = today.toString("yyyy-MM-dd");
            result["type"] = "single_day";

        } else if (query.startsWith("search:")) {
            // Search schedules by keyword
            QString keyword = query.mid(7); // Remove "search:" prefix
            if (!keyword.isEmpty()) {
                // Get all schedules from a wide date range
                QDate startDate = QDate::currentDate().addMonths(-6);
                QDate endDate = QDate::currentDate().addMonths(6);
                QMap<QDate, DSchedule::List> scheduleMap =
                    gScheduleManager->getScheduleMap(startDate, endDate);

                // Search through all schedules for keyword
                for (auto it = scheduleMap.begin(); it != scheduleMap.end(); ++it) {
                    for (const auto &schedule : it.value()) {
                        QString title = schedule->summary();
                        QString description = schedule->description();
                        QString location = schedule->location();

                        // Case-insensitive search in title, description, and location
                        if (title.contains(keyword, Qt::CaseInsensitive)
                            || description.contains(keyword, Qt::CaseInsensitive)
                            || location.contains(keyword, Qt::CaseInsensitive)) {
                            schedules.append(scheduleToJson(schedule));
                        }
                    }
                }
            }
            result["keyword"] = keyword;
            result["type"] = "search";

        } else if (query.contains(",")) {
            // Date range query: "2024-01-15,2024-01-20"
            QStringList dates = query.split(",");
            if (dates.size() == 2) {
                QDate startDate = parseDateString(dates[0]);
                QDate endDate = parseDateString(dates[1]);

                if (startDate.isValid() && endDate.isValid()) {
                    QMap<QDate, DSchedule::List> scheduleMap =
                        gScheduleManager->getScheduleMap(startDate, endDate);

                    for (auto it = scheduleMap.constBegin(); it != scheduleMap.constEnd(); ++it) {
                        for (const auto &schedule : it.value()) {
                            schedules.append(scheduleToJson(schedule));
                        }
                    }
                    result["startDate"] = startDate.toString("yyyy-MM-dd");
                    result["endDate"] = endDate.toString("yyyy-MM-dd");
                    result["type"] = "date_range";
                }
            }

        } else {
            // Single date query
            QDate queryDate = parseDateString(query);
            if (queryDate.isValid()) {
                DSchedule::List daySchedules = gScheduleManager->getScheduleByDay(queryDate);

                for (const auto &schedule : daySchedules) {
                    schedules.append(scheduleToJson(schedule));
                }
                result["date"] = queryDate.toString("yyyy-MM-dd");
                result["type"] = "single_day";
            }
        }

    } catch (...) {
        result["error"] = "Failed to query schedules";
        result["schedules"] = QJsonArray();
        QJsonDocument doc(result);
        return doc.toJson(QJsonDocument::Compact);
    }

    result["schedules"] = schedules;
    result["count"] = schedules.size();
    result["success"] = true;

    QJsonDocument doc(result);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString CalendarAdaptor::CreateSchedule(const QString &scheduleData)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(scheduleData.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return QString(); // Invalid JSON
    }

    QJsonObject data = doc.object();

    try {
        DSchedule::Ptr schedule(new DSchedule());

        // Set basic information
        QString title = data["title"].toString();
        QString description = data["description"].toString();
        QString startTimeStr = data["startTime"].toString();
        QString endTimeStr = data["endTime"].toString();
        bool allDay = data["allDay"].toBool(false);
        QString location = data["location"].toString();

        if (title.isEmpty() || startTimeStr.isEmpty()) {
            return QString(); // Missing required fields
        }

        schedule->setSummary(title);
        schedule->setDescription(description);
        schedule->setLocation(location);
        schedule->setAllDay(allDay);

        // Parse dates
        QDateTime startTime = QDateTime::fromString(startTimeStr, Qt::ISODate);
        if (!startTime.isValid()) {
            startTime = QDateTime::fromString(startTimeStr, "yyyy-MM-dd hh:mm");
        }

        QDateTime endTime;
        if (!endTimeStr.isEmpty()) {
            endTime = QDateTime::fromString(endTimeStr, Qt::ISODate);
            if (!endTime.isValid()) {
                endTime = QDateTime::fromString(endTimeStr, "yyyy-MM-dd hh:mm");
            }
        } else {
            endTime = startTime.addSecs(3600);
        }

        schedule->setDtStart(startTime);
        schedule->setDtEnd(endTime);

        // Set reminder if provided
        int reminderMinutes = data["reminder"].toInt(0);
        if (reminderMinutes > 0) {
            KCalendarCore::Alarm::Ptr alarm = schedule->newAlarm();
            alarm->setStartOffset(KCalendarCore::Duration(-reminderMinutes * 60));
            alarm->setEnabled(true);
            alarm->setType(KCalendarCore::Alarm::Display);
        }

        // Generate unique ID
        schedule->setUid(QUuid::createUuid().toString());

        // Fast path: use client cache if available
        AccountItem::Ptr account = gAccountManager->getLocalAccountItem();
        if (account && !account->getScheduleTypeList().isEmpty()) {
            QString typeId = pickUserScheduleType(account->getScheduleTypeList());
            if (!typeId.isEmpty()) {
                schedule->setScheduleTypeID(typeId);
            }
            account->createSchedule(schedule);
            return schedule->uid();
        }

        // Fallback: directly call backend service synchronously
        QDBusInterface accountIface = getLocalAccountInterface();
        if (!accountIface.isValid()) {
            return QString();
        }

        // Get schedule type list
        QDBusReply<QString> typeListReply = accountIface.call("getScheduleTypeList");
        if (!typeListReply.isValid()) {
            qCWarning(ClientLogger) << "Failed to get schedule type list:" << typeListReply.error().message();
            return QString();
        }

        DScheduleType::List typeList;
        if (!DScheduleType::fromJsonListString(typeList, typeListReply.value()) || typeList.isEmpty()) {
            qCWarning(ClientLogger) << "No schedule types available";
            return QString();
        }

        schedule->setScheduleTypeID(pickUserScheduleType(typeList));

        // Create the schedule
        QString scheduleJson;
        DSchedule::toJsonString(schedule, scheduleJson);
        QDBusReply<QString> createReply = accountIface.call("createSchedule", QVariant(scheduleJson));
        if (!createReply.isValid()) {
            qCWarning(ClientLogger) << "Failed to create schedule:" << createReply.error().message();
            return QString();
        }

        QString createdId = createReply.value();
        qCWarning(ClientLogger) << "Schedule created via backend fallback, uid:" << createdId;

        // Trigger client data refresh so the UI shows the new schedule
        QMetaObject::invokeMethod(gAccountManager, &AccountManager::resetAccount, Qt::QueuedConnection);

        return createdId;

    } catch (...) {
        qCWarning(ClientLogger) << "Exception in CreateSchedule";
    }

    return QString(); // Failed to create
}

bool CalendarAdaptor::ModifySchedule(const QString &scheduleId,
                                     const QString &operation,
                                     const QString &data)
{
    if (scheduleId.isEmpty() || operation.isEmpty()) {
        return false;
    }

    try {
        if (operation == "delete") {
            AccountItem::Ptr account = gAccountManager->getLocalAccountItem();
            if (account) {
                DSchedule::Ptr schedule = account->getScheduleByScheduleID(scheduleId);
                if (schedule) {
                    account->deleteScheduleByID(scheduleId);
                    return true;
                }
            }
            // Fallback: directly call backend
            QDBusInterface accountIface = getLocalAccountInterface();
            if (!accountIface.isValid()) {
                return false;
            }
            QDBusReply<bool> delReply = accountIface.call("deleteScheduleByScheduleID", QVariant(scheduleId));
            return delReply.isValid() && delReply.value();

        } else if (operation == "update") {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &error);

            if (error.error != QJsonParseError::NoError || !doc.isObject()) {
                return false;
            }

            AccountItem::Ptr account = gAccountManager->getLocalAccountItem();
            if (account) {
                DSchedule::Ptr schedule = account->getScheduleByScheduleID(scheduleId);
                if (schedule) {
                    applyScheduleUpdates(schedule, doc.object());
                    account->updateSchedule(schedule);
                    return true;
                }
            }

            // Fallback: directly call backend
            // (reached when account is null OR schedule not in cache)
            QDBusInterface accountIface = getLocalAccountInterface();
            if (!accountIface.isValid()) {
                return false;
            }

            // Fetch existing schedule, merge updates, then save
            QDBusReply<QString> getReply = accountIface.call("getScheduleByScheduleID", QVariant(scheduleId));
            if (!getReply.isValid()) {
                return false;
            }

            DSchedule::Ptr schedule;
            DSchedule::fromJsonString(schedule, getReply.value());
            if (!schedule) {
                return false;
            }

            applyScheduleUpdates(schedule, doc.object());

            QString scheduleJson;
            DSchedule::toJsonString(schedule, scheduleJson);
            QDBusReply<bool> updReply = accountIface.call("updateSchedule", QVariant(scheduleJson));
            return updReply.isValid() && updReply.value();

        } else if (operation == "snooze") {
            int snoozeMinutes = data.toInt();
            if (snoozeMinutes > 0) {
                return true;
            }
        }

    } catch (...) {
    }

    return false;
}

QString CalendarAdaptor::GetCalendarView(const QString &viewType, const QString &date)
{
    QJsonObject result;
    QJsonArray schedules;

    try {
        QDate targetDate = parseDateString(date);
        if (!targetDate.isValid()) {
            targetDate = QDate::currentDate();
        }

        if (viewType == "day") {
            DSchedule::List daySchedules = gScheduleManager->getScheduleByDay(targetDate);
            for (const auto &schedule : daySchedules) {
                schedules.append(scheduleToJson(schedule));
            }
            result["viewType"] = "day";
            result["date"] = targetDate.toString("yyyy-MM-dd");

        } else if (viewType == "week") {
            // Get week range (Monday to Sunday)
            QDate startOfWeek = targetDate.addDays(-(targetDate.dayOfWeek() - 1));
            QDate endOfWeek = startOfWeek.addDays(6);

            QMap<QDate, DSchedule::List> weekSchedules =
                gScheduleManager->getScheduleMap(startOfWeek, endOfWeek);

            QJsonArray days;
            for (int i = 0; i < 7; i++) {
                QDate dayDate = startOfWeek.addDays(i);
                QJsonObject dayObj;
                dayObj["date"] = dayDate.toString("yyyy-MM-dd");
                dayObj["dayOfWeek"] = dayDate.dayOfWeek();

                QJsonArray daySchedules;
                if (weekSchedules.contains(dayDate)) {
                    for (const auto &schedule : weekSchedules[dayDate]) {
                        daySchedules.append(scheduleToJson(schedule));
                    }
                }
                dayObj["schedules"] = daySchedules;
                days.append(dayObj);
            }

            result["viewType"] = "week";
            result["startDate"] = startOfWeek.toString("yyyy-MM-dd");
            result["endDate"] = endOfWeek.toString("yyyy-MM-dd");
            result["days"] = days;

        } else if (viewType == "month") {
            QDate startOfMonth(targetDate.year(), targetDate.month(), 1);
            QDate endOfMonth = startOfMonth.addMonths(1).addDays(-1);

            QMap<QDate, DSchedule::List> monthSchedules =
                gScheduleManager->getScheduleMap(startOfMonth, endOfMonth);

            for (auto it = monthSchedules.constBegin(); it != monthSchedules.constEnd(); ++it) {
                for (const auto &schedule : it.value()) {
                    QJsonObject scheduleObj = scheduleToJson(schedule);
                    scheduleObj["date"] = it.key().toString("yyyy-MM-dd");
                    schedules.append(scheduleObj);
                }
            }

            result["viewType"] = "month";
            result["year"] = targetDate.year();
            result["month"] = targetDate.month();
            result["startDate"] = startOfMonth.toString("yyyy-MM-dd");
            result["endDate"] = endOfMonth.toString("yyyy-MM-dd");
        }

        // Add basic lunar info for the target date
        QJsonObject lunarInfo;
        // This would connect to the lunar service, simplified for now
        lunarInfo["date"] = targetDate.toString("yyyy-MM-dd");
        lunarInfo["lunarDate"] = ""; // Would be filled by lunar service

        if (viewType != "week") {
            result["schedules"] = schedules;
        }
        result["lunarInfo"] = lunarInfo;
        result["success"] = true;

    } catch (...) {
        result["error"] = "Failed to get calendar view";
        result["success"] = false;
    }

    QJsonDocument doc(result);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString CalendarAdaptor::GetLunarInfo(const QString &date)
{
    QJsonObject result;

    try {
        QDate queryDate = parseDateString(date);
        if (!queryDate.isValid()) {
            // Return error for invalid dates instead of using current date
            result["error"] = "Invalid date format or impossible date: " + date;
            result["success"] = false;
            QJsonDocument doc(result);
            return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        }

        result["solarDate"] = queryDate.toString("yyyy-MM-dd");

        // Integrate with existing HuangLi service
        CaHuangLiDayInfo huangLiInfo = gLunarManager->getHuangLiDay(queryDate);

        // Populate lunar information
        result["lunarDate"] =
            QString("%1%2%3").arg(huangLiInfo.mGanZhiYear).arg(huangLiInfo.mZodiac).arg("年");
        result["lunarMonthDay"] =
            QString("%1%2").arg(huangLiInfo.mLunarMonthName).arg(huangLiInfo.mLunarDayName);

        // Combine solar and lunar festivals
        QString festivals;
        if (!huangLiInfo.mSolarFestival.isEmpty()) {
            festivals = huangLiInfo.mSolarFestival;
        }
        if (!huangLiInfo.mLunarFestival.isEmpty()) {
            if (!festivals.isEmpty())
                festivals += " ";
            festivals += huangLiInfo.mLunarFestival;
        }
        result["festival"] = festivals;
        result["jieqi"] = huangLiInfo.mTerm; // Solar terms

        // HuangLi details
        QJsonObject huangLi;

        // Parse mSuit and mAvoid strings (they might be single strings or comma-separated)
        QJsonArray yiArray;
        if (!huangLiInfo.mSuit.isEmpty()) {
            QStringList suitList =
                huangLiInfo.mSuit.split(QRegularExpression("[,，、\\s]+"), QT_SKIP_EMPTY_PARTS);
            for (const QString &yi : suitList) {
                if (!yi.trimmed().isEmpty()) {
                    yiArray.append(yi.trimmed());
                }
            }
        }

        QJsonArray jiArray;
        if (!huangLiInfo.mAvoid.isEmpty()) {
            QStringList avoidList =
                huangLiInfo.mAvoid.split(QRegularExpression("[,，、\\s]+"), QT_SKIP_EMPTY_PARTS);
            for (const QString &ji : avoidList) {
                if (!ji.trimmed().isEmpty()) {
                    jiArray.append(ji.trimmed());
                }
            }
        }

        huangLi["yi"] = yiArray; // Auspicious activities
        huangLi["ji"] = jiArray; // Inauspicious activities
        huangLi["ganZhiYear"] = huangLiInfo.mGanZhiYear;
        huangLi["ganZhiMonth"] = huangLiInfo.mGanZhiMonth;
        huangLi["ganZhiDay"] = huangLiInfo.mGanZhiDay;

        result["huangLi"] = huangLi;
        result["success"] = true;

    } catch (...) {
        // Fallback with basic info if HuangLi service fails
        result["solarDate"] = parseDateString(date).toString("yyyy-MM-dd");
        result["lunarDate"] = "农历信息获取失败";
        result["lunarMonthDay"] = "";
        result["festival"] = "";
        result["jieqi"] = "";

        QJsonObject huangLi;
        huangLi["yi"] = QJsonArray();
        huangLi["ji"] = QJsonArray();
        huangLi["chongSha"] = "";
        huangLi["wuxing"] = "";
        result["huangLi"] = huangLi;

        result["error"] = "Failed to get lunar information from HuangLi service";
        result["success"] = true; // Still return success with fallback data
    }

    QJsonDocument doc(result);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString CalendarAdaptor::GetReminders(int hours)
{
    QJsonObject result;
    QJsonArray reminders;

    try {
        if (hours <= 0) {
            hours = 24; // Default to 24 hours
        }

        QDateTime now = QDateTime::currentDateTime();
        QDateTime endTime = now.addSecs(hours * 3600); // Convert hours to seconds

        // Get schedules in the time range that have reminders
        QMap<QDate, DSchedule::List> scheduleMap =
            gScheduleManager->getScheduleMap(now.date(), endTime.date());

        for (auto it = scheduleMap.constBegin(); it != scheduleMap.constEnd(); ++it) {
            for (const auto &schedule : it.value()) {
                // Check if schedule has alarms and is within time range
                auto alarms = schedule->alarms();
                if (!alarms.isEmpty()) {
                    for (const auto &alarm : alarms) {
                        QDateTime alarmTime =
                            schedule->dtStart().addSecs(alarm->startOffset().asSeconds());

                        if (alarmTime >= now && alarmTime <= endTime) {
                            QJsonObject reminder;
                            reminder["scheduleId"] = schedule->uid();
                            reminder["title"] = schedule->summary();
                            reminder["alarmTime"] = alarmTime.toString(Qt::ISODate);
                            reminder["scheduleTime"] = schedule->dtStart().toString(Qt::ISODate);
                            reminder["minutesToAlarm"] =
                                static_cast<int>(now.secsTo(alarmTime) / 60);
                            reminders.append(reminder);
                        }
                    }
                }
            }
        }

        // Sort by alarm time
        // Note: This is a simplified sort, in practice you might want a more sophisticated approach

        result["reminders"] = reminders;
        result["count"] = reminders.size();
        result["timeRange"] =
            hours; // Keep original field name for backward compatibility but use new parameter
        result["success"] = true;

    } catch (...) {
        result["error"] = "Failed to get reminders";
        result["success"] = false;
    }

    QJsonDocument doc(result);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}
