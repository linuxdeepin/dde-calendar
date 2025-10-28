// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSCALENDAR_ADAPTOR_H
#define DBUSCALENDAR_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.Calendar
 */
class CalendarAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.Calendar")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name=\"com.deepin.Calendar\">\n"
                "    <method name=\"ActiveWindow\">\n"
                "    </method>\n"
                "    <method name=\"RaiseWindow\">\n"
                "    </method>\n"
                "    <method name=\"OpenSchedule\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"job\"/>\n"
                "    </method>\n"
                "    <method name=\"QuerySchedules\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"query\"/>\n"
                "      <arg direction=\"out\" type=\"s\" name=\"result\"/>\n"
                "    </method>\n"
                "    <method name=\"CreateSchedule\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"scheduleData\"/>\n"
                "      <arg direction=\"out\" type=\"s\" name=\"scheduleId\"/>\n"
                "    </method>\n"
                "    <method name=\"ModifySchedule\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"scheduleId\"/>\n"
                "      <arg direction=\"in\" type=\"s\" name=\"operation\"/>\n"
                "      <arg direction=\"in\" type=\"s\" name=\"data\"/>\n"
                "      <arg direction=\"out\" type=\"b\" name=\"success\"/>\n"
                "    </method>\n"
                "    <method name=\"GetCalendarView\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"viewType\"/>\n"
                "      <arg direction=\"in\" type=\"s\" name=\"date\"/>\n"
                "      <arg direction=\"out\" type=\"s\" name=\"viewData\"/>\n"
                "    </method>\n"
                "    <method name=\"GetLunarInfo\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"date\"/>\n"
                "      <arg direction=\"out\" type=\"s\" name=\"lunarData\"/>\n"
                "    </method>\n"
                "    <method name=\"GetReminders\">\n"
                "      <arg direction=\"in\" type=\"i\" name=\"hours\"/>\n"
                "      <arg direction=\"out\" type=\"s\" name=\"reminders\"/>\n"
                "    </method>\n"
                "  </interface>\n"
                "")
public:
    explicit CalendarAdaptor(QObject *parent);
    virtual ~CalendarAdaptor();
public: // PROPERTIES
public Q_SLOTS: // METHODS
    void ActiveWindow();
    void RaiseWindow();
    void OpenSchedule(QString job);

    // DBus APIs for AI Integration
    /**
     * @brief QuerySchedules Query schedules by various criteria
     * @param query Query string in formats:
     *        - "today" - today's schedules
     *        - "YYYY-MM-DD" - specific date
     *        - "YYYY-MM-DD,YYYY-MM-DD" - date range
     *        - "search:keyword" - keyword search
     * @return JSON string containing schedule list
     */
    QString QuerySchedules(const QString &query);

    /**
     * @brief CreateSchedule Create a new schedule
     * @param scheduleData JSON string containing schedule information
     * @return Created schedule ID
     */
    QString CreateSchedule(const QString &scheduleData);

    /**
     * @brief ModifySchedule Modify, delete or snooze a schedule
     * @param scheduleId Schedule ID to modify
     * @param operation Operation type: "delete", "update", "snooze"
     * @param data Additional data (JSON for update, minutes for snooze)
     * @return Operation success status
     */
    bool ModifySchedule(const QString &scheduleId, const QString &operation, const QString &data);

    /**
     * @brief GetCalendarView Get calendar view data including schedules and lunar info
     * @param viewType View type: "day", "week", "month"
     * @param date Target date (optional, defaults to current date)
     * @return JSON string containing complete calendar view data
     */
    QString GetCalendarView(const QString &viewType, const QString &date);

    /**
     * @brief GetLunarInfo Get detailed lunar calendar information
     * @param date Date in "YYYY-MM-DD" format
     * @return JSON string containing lunar and huangli information
     */
    QString GetLunarInfo(const QString &date);

    /**
     * @brief GetReminders Get upcoming reminders
     * @param hours Time range in hours (default 24)
     * @return JSON string containing reminder list
     */
    QString GetReminders(int hours);

Q_SIGNALS: // SIGNALS
};

#endif
