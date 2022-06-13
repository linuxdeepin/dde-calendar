/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
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
#include "settingwidgets.h"
#include "accountmanager.h"
#include "calendarmanage.h"
#include "units.h"

#include <DSettingsWidgetFactory>
#include <DSettingsOption>
#include <DHiDPIHelper>

#include <qglobal.h>

DWIDGET_USE_NAMESPACE
using namespace SettingWidget;

SettingWidgets::SettingWidgets(QWidget *parent) : QObject(parent)
{
}


SyncTagRadioButton::SyncTagRadioButton(DAccount::AccountState type, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
{

    setObjectName("SyncTagRadioButton");
    if (gUosAccountItem) {
        m_state = DAccount::Account_Close;
        m_state = gUosAccountItem->getAccount()->accountState();
        connect(gUosAccountItem.get(), &AccountItem::signalAccountStateChange, this, &SyncTagRadioButton::updateAccountState);
        updateAccountState();
    }
}

void SyncTagRadioButton::updateAccountState()
{
    if (!gUosAccountItem) {
        return;
    }
    m_state = gUosAccountItem->getAccount()->accountState();
    setChecked(m_state & m_type);
    //TODO:是否联网
    //setEnabled((m_state & DAccount::Account_Open) && m_isOnline);
}

void SyncTagRadioButton::updateOnLineState(bool isOnline)
{
    m_isOnline = isOnline;

    updateAccountState();
}

bool SyncTagRadioButton::isChecked()
{
    return m_checked;
}

DAccount::AccountState SyncTagRadioButton::type()
{
    return m_type;
}

void SyncTagRadioButton::setChecked(bool checked)
{
    if (m_checked == checked)
        return;
    if (!gUosAccountItem)
        return;

    m_checked = checked;
    update();

    //实现遍历所有的radiobutton获取account state
    DAccount::AccountStates states = gUosAccountItem->getAccount()->accountState();
    QObject *parent = this->parent();
    parent = parent == nullptr ? nullptr : parent->parent();
    if (parent) {
        for (auto obj : parent->findChildren<QWidget *>("SyncTagRadioButton")) {
            SyncTagRadioButton *rb = static_cast<SyncTagRadioButton *>(obj);
            if (rb->isChecked())
                states |= rb->type();
            else
                states &= ~rb->type();
        }
    }
    gUosAccountItem->setAccountState(states);

}

void SyncTagRadioButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QIcon icon = DStyle::standardIcon(this->style(), m_checked ? DStyle::SP_IndicatorChecked : DStyle::SP_IndicatorUnchecked);
    int y = (this->height() - 16) / 2;
    int x = (this->width() - 16) / 2;
    icon.paint(&painter, QRect(x, y, 16, 16), Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

void SyncTagRadioButton::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    setChecked(!m_checked);
}

void CalendarSettingSettings::removeGroup(const QString &groupName, const QString &groupName2)
{
    int index = this->indexOf(*this, groupName);
    if (index < 0)
        return;
    CalendarSettingGroups &groups = this->operator[](index)._groups;
    {
        int index = indexOf(groups, groupName2);
        if (index < 0)
            return;
        groups.removeAt(index);
    }
    if (groups.isEmpty()) {
        this->removeAt(index);
    }
}

void CalendarSettingSettings::removeGroup(const QString &groupName)
{
    int index = this->indexOf(*this, groupName);
    if (index < 0)
        return;
    this->removeAt(index);
}

int CalendarSettingSettings::indexOf(const CalendarSettingGroups &groups, const QString groupName)
{
    for (int k = 0; k < groups.count(); k++) {
        if (groups[k]._key == groupName)
            return k;
    }
    return -1;
}

int CalendarSettingSettings::indexOf(const CalendarSettingSettings &groups, const QString groupName)
{
    for (int k = 0; k < groups.count(); k++) {
        if (groups[k]._key == groupName)
            return k;
    }
    return -1;
}
