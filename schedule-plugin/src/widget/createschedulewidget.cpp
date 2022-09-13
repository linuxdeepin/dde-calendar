// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "createschedulewidget.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QProcess>
#include <QPushButton>

#include "buttonwidget.h"
#include "../globaldef.h"

createSchedulewidget::createSchedulewidget(QWidget *parent)
    : IconDFrame(parent)
    , m_scheduleDtailInfo(ScheduleDtailInfo())
    , m_scheduleitemwidget(new scheduleitemwidget(this))
{
    connect(m_scheduleitemwidget, &scheduleitemwidget::signalItemPress, this, &createSchedulewidget::slotItemPress);
    //    updateUI();
}

ScheduleDtailInfo &createSchedulewidget::getScheduleDtailInfo()
{
    return m_scheduleDtailInfo;
}

void createSchedulewidget::setTitleName(const QString &titleName)
{
    m_titleName = titleName;
}

void createSchedulewidget::setDateTime(QDateTime begintime, QDateTime endtime)
{
    m_BeginDateTime = begintime;
    m_EndDateTime = endtime;
}

void createSchedulewidget::setRpeat(int rpeat)
{
    m_rpeat = rpeat;
}

void createSchedulewidget::setschedule()
{
    QDateTime m_beginTime;
    QDateTime m_endTime;
    m_beginTime = m_BeginDateTime;
    m_endTime = m_EndDateTime;

    m_scheduleDtailInfo.beginDateTime = m_beginTime;
    m_scheduleDtailInfo.endDateTime = m_endTime;
    m_scheduleDtailInfo.titleName = m_titleName;
    m_scheduleDtailInfo.type.ID = 3;
    m_scheduleDtailInfo.id = 0;
    m_scheduleDtailInfo.RecurID = 0;
    m_scheduleDtailInfo.allday = false;
    m_scheduleDtailInfo.remind = true;
    m_scheduleDtailInfo.remindData.n = 0;
    m_scheduleDtailInfo.rpeat = m_rpeat;
    //    if (m_rpeat != 0) {
    //结束重复于类型为：永不
    m_scheduleDtailInfo.enddata.type = 0;
    //    }
}

void createSchedulewidget::scheduleEmpty(bool isEmpty)
{
    m_scheduleEmpty = isEmpty;
}

void createSchedulewidget::updateUI()
{
    if (m_scheduleEmpty) {
        //获取筛选到的日程信息
        getCreatScheduleFromDbus();
        //如果筛选到的日程不为空，则展示日程插件
        if (!m_scheduleInfo.isEmpty()) {
            QVBoxLayout *mainlayout = new QVBoxLayout();
            m_scheduleitemwidget->setScheduleDtailInfo(m_scheduleInfo);
            m_scheduleitemwidget->addscheduleitem();
            mainlayout->addWidget(m_scheduleitemwidget);
            setCenterLayout(mainlayout);
        } else {
            qCritical() << "There's not the same schedule in scheduleSql!";
        }
    } else {
        //是否创建日程
        QVBoxLayout *mainlayout = new QVBoxLayout();
        buttonwidget *button = new buttonwidget(this);
        button->addbutton(CANCEL_BUTTON_STRING, true, buttonwidget::ButtonRecommend);
        button->addbutton(CONFIRM_BUTTON_STRING);
        connect(button, &buttonwidget::buttonClicked, this, &createSchedulewidget::slotsbuttonchance);
        m_scheduleitemwidget->setScheduleDtailInfo(m_scheduleInfo);
        m_scheduleitemwidget->addscheduleitem();
        mainlayout->addWidget(m_scheduleitemwidget);
        mainlayout->addSpacing(4);
        mainlayout->addWidget(button);
        setCenterLayout(mainlayout);
    }
}

void createSchedulewidget::setScheduleDbus(CSchedulesDBus *dbus)
{
    m_dbus = dbus;
}

bool createSchedulewidget::buttonclicked()
{
    return m_buttonclicked;
}

void createSchedulewidget::slotsbuttonchance(int index, const QString &text)
{
    Q_UNUSED(text)
    if (index == 0) {
        m_buttonclicked = false;
    }
    if (index == 1) {
        m_buttonclicked = true;
    }
}

void createSchedulewidget::slotItemPress(const ScheduleDtailInfo &info)
{
    QProcess proc;
    proc.startDetached(PROCESS_OPEN_CALENDAR);
    QThread::msleep(750);
    QString schedulestr = CSchedulesDBus::createScheduleDtailInfojson(info);
    QDBusMessage message = QDBusMessage::createMethodCall(DBUS_CALENDAR_SERVICE,
                                                          DBUS_CALENDAR_PATCH,
                                                          DBUS_CALENDAR_INTFACE,
                                                          "OpenSchedule");
    message << schedulestr;
    //发送消息
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
}

void createSchedulewidget::getCreatScheduleFromDbus()
{
    //存放查询的日程信息
    QVector<ScheduleDateRangeInfo> out;
    //存放符合筛选条件的日程信息
    QVector<ScheduleDtailInfo> scheduleinfo;
    //清空容器
    scheduleinfo.clear();
    out.clear();
    //通过dbus获取和新建的日程具有相同titlename，beginDateTime，endDateTime的日程
    m_dbus->QueryJobs(m_scheduleDtailInfo.titleName, m_scheduleDtailInfo.beginDateTime, m_scheduleDtailInfo.endDateTime, out);
    //筛选具体日程
    for (int i = 0; i < out.count(); i++) {
        for (int j = 0; j < out.at(i).vData.count(); j++) {
            //筛选条件
            if (out.at(i).vData.at(j).titleName == m_scheduleDtailInfo.titleName
                    && out.at(i).vData.at(j).beginDateTime == m_scheduleDtailInfo.beginDateTime
                    && out.at(i).vData.at(j).endDateTime == m_scheduleDtailInfo.endDateTime
                    && out.at(i).vData.at(j).rpeat == m_scheduleDtailInfo.rpeat
                    && out.at(i).vData.at(j).allday == m_scheduleDtailInfo.allday
                    && out.at(i).vData.at(j).type.ID == m_scheduleDtailInfo.type.ID
                    && out.at(i).vData.at(j).RecurID == m_scheduleDtailInfo.RecurID
                    && out.at(i).vData.at(j).remind == m_scheduleDtailInfo.remind
                    && out.at(i).vData.at(j).remindData.n == m_scheduleDtailInfo.remindData.n) {
                if (m_scheduleDtailInfo.rpeat > 0 && out.at(i).vData.at(j).enddata.type != m_scheduleDtailInfo.enddata.type)
                    continue;
                scheduleinfo.append(out.at(i).vData.at(j));
            }
        }
    }
    //如果和新建日程具有相同信息的日程有多个，则取id最大的那一个
    if (scheduleinfo.count() > 1) {
        for (int i = 0; i < scheduleinfo.count() - 1; i++) {
            if (scheduleinfo.at(i).id < scheduleinfo.at(i + 1).id) {
                m_scheduleInfo.clear();
                m_scheduleInfo.append(scheduleinfo.at(i + 1));
            } else {
                m_scheduleInfo.clear();
                m_scheduleInfo.append(scheduleinfo.at(i));
            }
        }
    } else {
        m_scheduleInfo = scheduleinfo;
    }
}
