// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingwidgets.h"
#include "accountmanager.h"
#include "calendarmanage.h"
#include "units.h"
#include "commondef.h"

#include <DSettingsWidgetFactory>
#include <DSettingsOption>


#include <qglobal.h>

DWIDGET_USE_NAMESPACE
using namespace SettingWidget;

SettingWidgets::SettingWidgets(QWidget *parent) : QObject(parent)
{
    qCDebug(ClientLogger) << "SettingWidgets constructed";
}


SyncTagRadioButton::SyncTagRadioButton(QWidget *parent)
    : QWidget(parent)
{
    qCDebug(ClientLogger) << "SyncTagRadioButton constructed";
}

bool SyncTagRadioButton::isChecked()
{
    return m_checked;
}

void SyncTagRadioButton::setChecked(bool checked)
{
    if (m_checked == checked)
        return;

    m_checked = checked;
    qCDebug(ClientLogger) << "SyncTagRadioButton checked state changed to:" << checked;
    update();
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
    // qCDebug(ClientLogger) << "SyncTagRadioButton mouse released, toggling state";
    setChecked(!m_checked);
    emit clicked(isChecked());
}

void CalendarSettingSettings::removeGroup(const QString &groupName, const QString &groupName2)
{
    qCDebug(ClientLogger) << "Removing group:" << groupName << "and subgroup:" << groupName2;
    int index = this->indexOf(*this, groupName);
    if (index < 0) {
        qCDebug(ClientLogger) << "Group not found:" << groupName;
        return;
    }
    CalendarSettingGroups &groups = this->operator[](index)._groups;
    {
        int index = indexOf(groups, groupName2);
        if (index < 0) {
            qCDebug(ClientLogger) << "Subgroup not found:" << groupName2;
            return;
        }
        groups.removeAt(index);
        qCDebug(ClientLogger) << "Subgroup removed:" << groupName2;
    }
    if (groups.isEmpty()) {
        this->removeAt(index);
        qCDebug(ClientLogger) << "Group also removed (empty):" << groupName;
    }
}

void CalendarSettingSettings::removeGroup(const QString &groupName)
{
    qCDebug(ClientLogger) << "Removing group:" << groupName;
    int index = this->indexOf(*this, groupName);
    if (index < 0) {
        qCDebug(ClientLogger) << "Group not found:" << groupName;
        return;
    }
    this->removeAt(index);
    qCDebug(ClientLogger) << "Group removed:" << groupName;
}

int CalendarSettingSettings::indexOf(const CalendarSettingGroups &groups, const QString groupName)
{
    qCDebug(ClientLogger) << "Searching for subgroup:" << groupName;
    for (int k = 0; k < groups.count(); k++) {
        if (groups[k]._key == groupName) {
            qCDebug(ClientLogger) << "Subgroup found:" << groupName;
            return k;
        }
    }
    qCDebug(ClientLogger) << "Subgroup not found:" << groupName;
    return -1;
}

int CalendarSettingSettings::indexOf(const CalendarSettingSettings &groups, const QString groupName)
{
    qCDebug(ClientLogger) << "Searching for group:" << groupName;
    for (int k = 0; k < groups.count(); k++) {
        if (groups[k]._key == groupName) {
            qCDebug(ClientLogger) << "Group found:" << groupName;
            return k;
        }
    }
    qCDebug(ClientLogger) << "Group not found:" << groupName;
    return -1;
}
