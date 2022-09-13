// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULESDBUS_H
#define SCHEDULESDBUS_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include "../data/schedulestructs.h"

/*
 * Proxy class for interface com.deepin.dataserver.Calendar
 */
class CSchedulesDBus : public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage &msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName != "com.deepin.dataserver.Calendar")
            return;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        foreach (const QString &prop, changedProps.keys()) {
            const QMetaObject *self = metaObject();
            for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
                    Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
    }

public:
    static inline const char *staticInterfaceName()
    {
        return "com.deepin.dataserver.Calendar";
    }

public:
    CSchedulesDBus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~CSchedulesDBus();
    QString toconvertIGData(QDateTime date);
    QDateTime fromconvertiIGData(QString str);
    static QString createScheduleDtailInfojson(const ScheduleDtailInfo &info);
    ScheduleDtailInfo parsingScheduleDtailInfojsonID(QJsonObject &obj);

private:
    QString createScheduleTypejson(const ScheduleType &info);
    ScheduleType parsingScheduleTypejson(QJsonObject &object);

    static QString createScheduleRRule(const ScheduleDtailInfo &info);
    void parsingScheduleRRule(QString str, ScheduleDtailInfo &info);
    static QString createScheduleRemind(const ScheduleDtailInfo &info);
    void parsingScheduleRemind(QString str, ScheduleDtailInfo &info);
    static QString toconvertData(QDateTime date);
    QDateTime fromconvertData(QString str);
public Q_SLOTS: // METHODS

    qint64 CreateJob(const ScheduleDtailInfo &info);
    bool GetJobs(int startYear, int startMonth, int startDay, int endYear, int endMonth, int endDay, QVector<ScheduleDateRangeInfo> &out);
    bool GetJob(qint64 jobId, ScheduleDtailInfo &out);
    bool UpdateJob(const ScheduleDtailInfo &info);
    bool DeleteJob(qint64 jobId);
    bool QueryJobs(QString key, QDateTime starttime, QDateTime endtime, QVector<ScheduleDateRangeInfo> &out);
    bool QueryJobs(QString key, QDateTime starttime, QDateTime endtime, QString &out);
    bool QueryJobsWithLimit(QDateTime starttime, QDateTime endtime, qint32 maxNum, QVector<ScheduleDateRangeInfo> &out);
    bool QueryJobsWithRule(QDateTime starttime, QDateTime endtime, const QString &rule, QVector<ScheduleDateRangeInfo> &out);

    bool GetTypes(QVector<ScheduleType> &out);
    bool GetType(qint64 jobId, ScheduleType &out);
    qint64 CreateType(const ScheduleType &info);
    bool DeleteType(qint64 jobId);
    bool UpdateType(const ScheduleType &info);

Q_SIGNALS: // SIGNALS
    // begin property changed signals
};
#endif
