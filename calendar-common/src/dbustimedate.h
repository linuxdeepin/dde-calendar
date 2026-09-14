// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSTIMEDATE_H
#define DBUSTIMEDATE_H

#include <QDBusAbstractInterface>
#include <Qt>

//通过dbus获取控制中心相关时间设置
class DBusTimedate : public QDBusAbstractInterface
{
    Q_OBJECT
    Q_PROPERTY(int ShortTimeFormat READ shortTimeFormat NOTIFY ShortTimeFormatChanged)
    Q_PROPERTY(int ShortDateFormat READ shortDateFormat NOTIFY ShortDateFormatChanged)
    Q_PROPERTY(int WeekBegins READ weekBegins NOTIFY WeekBeginsChanged)
    Q_PROPERTY(bool Use24HourFormat READ use24HourFormat NOTIFY use24HourFormatChanged)
public:
    explicit DBusTimedate(QObject *parent = nullptr);
    int shortTimeFormat();
    int shortDateFormat();
    Qt::DayOfWeek weekBegins();
    //当前守护进程是否提供 Use24HourFormat 属性（12/24小时制的真实载体）
    bool hasUse24HourFormat();
    bool use24HourFormat();
signals:
    void ShortDateFormatChanged(int  value) const;
    void ShortTimeFormatChanged(int  value) const;
    void WeekBeginsChanged(int value) const;
    void use24HourFormatChanged(bool value) const;

public slots:
    void propertiesChanged(const QDBusMessage &msg);
private:
    QVariant getPropertyByName(const char *porpertyName);
    void initPropertyFlags();
private:
    bool m_hasDateTimeFormat = false;       //是否含有时间格式属性组
    bool m_hasUse24HourFormat = false;      //是否含有 Use24HourFormat 属性
};

#endif // DBUSTIMEDATE_H
