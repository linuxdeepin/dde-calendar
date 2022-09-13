// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CREATESCHEDULE_H
#define CREATESCHEDULE_H

#include <QObject>
#include <DFrame>
#include <QPaintEvent>
#include "../data/schedulestructs.h"
#include "icondframe.h"
#include "scheduleitemwidget.h"
#include "../dbus/schedulesdbus.h"

DWIDGET_USE_NAMESPACE

class createSchedulewidget : public IconDFrame
{
    Q_OBJECT
public:
    explicit createSchedulewidget(QWidget *parent = nullptr);

public:
    ScheduleDtailInfo &getScheduleDtailInfo();
    void setTitleName(const QString &titleName);
    void setDateTime(QDateTime begintime, QDateTime endtime);
    void setRpeat(int rpeat);
    void setschedule();
    void scheduleEmpty(bool isEmpty);
    void updateUI();
    void setScheduleDbus(CSchedulesDBus *dbus);
    bool buttonclicked();
public slots:
    void slotsbuttonchance(int index, const QString &text);
    void slotItemPress(const ScheduleDtailInfo &info);
    /**
     * @brief getCreatScheduleFromDbus 通过dbus获取新建的日程信息
     */
    void getCreatScheduleFromDbus();

private:
    ScheduleDtailInfo m_scheduleDtailInfo;
    QDateTime m_BeginDateTime;
    QDateTime m_EndDateTime;
    QString m_titleName;
    int m_rpeat;
    bool m_scheduleEmpty = false;
    scheduleitemwidget *m_scheduleitemwidget;
    QVector<ScheduleDtailInfo> m_scheduleInfo;
    CSchedulesDBus *m_dbus = nullptr;
    bool m_buttonclicked = false;

signals:

public slots:
};

#endif // CREATESCHEDULE_H
