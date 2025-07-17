// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountmanagerservice.h"
#include "commondef.h"
#include "units.h"
#include "calendarprogramexitcontrol.h"

#include <QMetaType>
#include <QDBusMetaType>
#include <QDebug>

DAccountManagerService::DAccountManagerService(QObject *parent)
    : DServiceBase(serviceBasePath + "/AccountManager", serviceBaseName + ".AccountManager", parent)
    , m_accountManager(new DAccountManageModule(this))
{
    qCDebug(ServiceLogger) << "Initializing AccountManagerService with path:" << serviceBasePath + "/AccountManager";
    //自动退出
    DServiceExitControl exitControl;
    connect(m_accountManager.data(), &DAccountManageModule::signalLoginStatusChange, this, &DAccountManagerService::accountUpdate);
    qCDebug(ServiceLogger) << "Connected login status change signal";

    connect(m_accountManager.data(), &DAccountManageModule::firstDayOfWeekChange, this, [&]() {
        qCDebug(ServiceLogger) << "First day of week changed, notifying property change";
        notifyPropertyChanged(getInterface(), "firstDayOfWeek");
    });
    connect(m_accountManager.data(), &DAccountManageModule::timeFormatTypeChange, this, [&]() {
        qCDebug(ServiceLogger) << "Time format type changed, notifying property change";
        notifyPropertyChanged(getInterface(), "timeFormatType");
    });
    qCDebug(ServiceLogger) << "AccountManagerService initialization completed";
}

QString DAccountManagerService::getAccountList()
{
    qCDebug(ServiceLogger) << "Getting account list";
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        qCDebug(ServiceLogger) << "Client not whitelisted, returning empty account list";
        return QString();
    }
    QString accountList = m_accountManager->getAccountList();
    qCDebug(ServiceLogger) << "Retrieved account list with length:" << accountList.length();
    return accountList;
}

void DAccountManagerService::remindJob(const QString &accountID, const QString &alarmID)
{
    qCDebug(ServiceLogger) << "Processing remind job for accountID:" << accountID << "alarmID:" << alarmID;
    DServiceExitControl exitControl;
    m_accountManager->remindJob(accountID, alarmID);
    qCDebug(ServiceLogger) << "Completed remind job processing";
}

void DAccountManagerService::updateRemindJob(bool isClear)
{
    qCDebug(ServiceLogger) << "Updating remind job with clear flag:" << isClear;
    DServiceExitControl exitControl;
    m_accountManager->updateRemindSchedules(isClear);
    qCDebug(ServiceLogger) << "Completed remind job update";
}

void DAccountManagerService::notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum)
{
    qCDebug(ServiceLogger) << "Handling notification message for accountID:" << accountID << "alarmID:" << alarmID << "operation:" << operationNum;
    DServiceExitControl exitControl;
    m_accountManager->notifyMsgHanding(accountID, alarmID, operationNum);
    qCDebug(ServiceLogger) << "Completed notification message handling";
}

void DAccountManagerService::downloadByAccountID(const QString &accountID)
{
    qCDebug(ServiceLogger) << "Starting download for accountID:" << accountID;
    //TODO:云同步获取数据
    DServiceExitControl exitControl;
    m_accountManager->downloadByAccountID(accountID);
    qCDebug(ServiceLogger) << "Completed download for accountID:" << accountID;
}

void DAccountManagerService::uploadNetWorkAccountData()
{
    qCDebug(ServiceLogger) << "Starting network account data upload";
    //TODO:云同步上传数据
    DServiceExitControl exitControl;
    m_accountManager->uploadNetWorkAccountData();
    qCDebug(ServiceLogger) << "Completed network account data upload";
}

QString DAccountManagerService::getCalendarGeneralSettings()
{
    qCDebug(ServiceLogger) << "Getting calendar general settings";
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        qCDebug(ServiceLogger) << "Client not whitelisted, returning empty settings";
        return QString();
    }
    QString settings = m_accountManager->getCalendarGeneralSettings();
    qCDebug(ServiceLogger) << "Retrieved calendar general settings with length:" << settings.length();
    return settings;
}

void DAccountManagerService::setCalendarGeneralSettings(const QString &cgSet)
{
    qCDebug(ServiceLogger) << "Setting calendar general settings with length:" << cgSet.length();
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        qCDebug(ServiceLogger) << "Client not whitelisted, canceling settings update";
        return;
    }
    m_accountManager->setFirstDayOfWeekSource(DCalendarGeneralSettings::Source_Database);
    m_accountManager->setTimeFormatTypeSource(DCalendarGeneralSettings::Source_Database);
    m_accountManager->setCalendarGeneralSettings(cgSet);
    qCDebug(ServiceLogger) << "Completed calendar general settings update";
}

void DAccountManagerService::calendarIsShow(const bool &isShow)
{
    qCDebug(ServiceLogger) << "Setting calendar show status to:" << isShow;
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        qCDebug(ServiceLogger) << "Client not whitelisted, canceling show status update";
        return;
    }
    exitControl.setClientIsOpen(isShow);
    m_accountManager->calendarOpen(isShow);
    qCDebug(ServiceLogger) << "Completed calendar show status update";
}

void DAccountManagerService::login()
{
    qCDebug(ServiceLogger) << "Starting user login process";
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        qCDebug(ServiceLogger) << "Client not whitelisted, canceling login";
        return;
    }
    m_accountManager->login();
    qCDebug(ServiceLogger) << "Completed user login process";
}

void DAccountManagerService::logout()
{
    qCDebug(ServiceLogger) << "Starting user logout process";
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        qCDebug(ServiceLogger) << "Client not whitelisted, canceling logout";
        return;
    }
    m_accountManager->logout();
    qCDebug(ServiceLogger) << "Completed user logout process";
}

bool DAccountManagerService::isSupportUid()
{
    qCDebug(ServiceLogger) << "Checking UID support";
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        qCDebug(ServiceLogger) << "Client not whitelisted, returning false for UID support";
        return false;
    }
    bool isSupported = m_accountManager->isSupportUid();
    qCDebug(ServiceLogger) << "UID support status:" << isSupported;
    return isSupported;
}

int DAccountManagerService::getfirstDayOfWeek() const
{
    qCDebug(ServiceLogger) << "Getting first day of week";
    DServiceExitControl exitControl;
    int firstDay = m_accountManager->getfirstDayOfWeek();
    qCDebug(ServiceLogger) << "First day of week:" << firstDay;
    return firstDay;
}

void DAccountManagerService::setFirstDayOfWeek(const int firstday)
{
    qCDebug(ServiceLogger) << "Setting first day of week to:" << firstday;
    DServiceExitControl exitControl;
    m_accountManager->setFirstDayOfWeekSource(DCalendarGeneralSettings::Source_Database);
    m_accountManager->setFirstDayOfWeek(firstday);
    qCDebug(ServiceLogger) << "Completed first day of week update";
}

int DAccountManagerService::getTimeFormatType() const
{
    qCDebug(ServiceLogger) << "Getting time format type";
    DServiceExitControl exitControl;
    int timeType = m_accountManager->getTimeFormatType();
    qCDebug(ServiceLogger) << "Time format type:" << timeType;
    return timeType;
}

void DAccountManagerService::setTimeFormatType(const int timeType)
{
    qCDebug(ServiceLogger) << "Setting time format type to:" << timeType;
    DServiceExitControl exitControl;
    m_accountManager->setTimeFormatTypeSource(DCalendarGeneralSettings::Source_Database);
    m_accountManager->setTimeFormatType(timeType);
    qCDebug(ServiceLogger) << "Completed time format type update";
}

int DAccountManagerService::getFirstDayOfWeekSource()
{
    qCDebug(ServiceLogger) << "Getting first day of week source";
    int source = static_cast<int>(m_accountManager->getFirstDayOfWeekSource());
    qCDebug(ServiceLogger) << "First day of week source:" << source;
    return source;
}

void DAccountManagerService::setFirstDayOfWeekSource(const int source)
{
    qCDebug(ServiceLogger) << "Setting first day of week source to:" << source;
    if (source >= 0 && source < DCalendarGeneralSettings::GeneralSettingSource::Source_Unknown) {
        qCDebug(ServiceLogger) << "Valid source value, applying setting";
        auto val = static_cast<DCalendarGeneralSettings::GeneralSettingSource>(source);
        m_accountManager->setFirstDayOfWeekSource(val);
    } else {
        qCWarning(ServiceLogger) << "Invalid source value:" << source << ", ignoring request";
    }
    qCDebug(ServiceLogger) << "Completed first day of week source update";
}

int DAccountManagerService::getTimeFormatTypeSource()
{
    qCDebug(ServiceLogger) << "Getting time format type source";
    int source = static_cast<int>(m_accountManager->getTimeFormatTypeSource());
    qCDebug(ServiceLogger) << "Time format type source:" << source;
    return source;
}

void DAccountManagerService::setTimeFormatTypeSource(const int source)
{
    qCDebug(ServiceLogger) << "Setting time format type source to:" << source;
    if (source >= 0 && source < DCalendarGeneralSettings::GeneralSettingSource::Source_Unknown) {
        qCDebug(ServiceLogger) << "Valid source value, applying setting";
        auto val = static_cast<DCalendarGeneralSettings::GeneralSettingSource>(source);
        m_accountManager->setTimeFormatTypeSource(val);
    } else {
        qCWarning(ServiceLogger) << "Invalid source value:" << source << ", ignoring request";
    }
    qCDebug(ServiceLogger) << "Completed time format type source update";
}