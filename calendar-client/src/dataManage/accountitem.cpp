// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountitem.h"
#include "doanetworkdbus.h"
#include "commondef.h"

AccountItem::AccountItem(const DAccount::Ptr &account, QObject *parent)
    : QObject(parent)
    , m_account(account)
    , m_dbusRequest(new DbusAccountRequest(account->dbusPath(), account->dbusInterface(), this))
{
    qCDebug(ClientLogger) << "Creating AccountItem for account:" << account->accountName() << "type:" << account->accountType();
    initConnect();
}

void AccountItem::initConnect()
{
    qCDebug(ClientLogger) << "Initializing connections for account:" << m_account->accountName();
    connect(m_dbusRequest, &DbusAccountRequest::signalGetAccountInfoFinish, this, &AccountItem::slotGetAccountInfoFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetScheduleTypeListFinish, this, &AccountItem::slotGetScheduleTypeListFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetScheduleListFinish, this, &AccountItem::slotGetScheduleListFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalGetSysColorsFinish, this, &AccountItem::slotGetSysColorsFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalSearchScheduleListFinish, this, &AccountItem::slotSearchScheduleListFinish);
    connect(m_dbusRequest, &DbusAccountRequest::signalDtLastUpdate, this, &AccountItem::signalDtLastUpdate);
    connect(m_dbusRequest, &DbusAccountRequest::signalAccountStateChange, this, &AccountItem::signalAccountStateChange);
    connect(m_dbusRequest, &DbusAccountRequest::signalSyncStateChange, this, &AccountItem::slotSyncStateChange);
    connect(m_dbusRequest, &DbusAccountRequest::signalAccountStateChange, this, &AccountItem::slotAccountStateChange);
    connect(m_dbusRequest, &DbusAccountRequest::signalSearchUpdate, this, &AccountItem::slotSearchUpdata);
}

/**
 * @brief AccountItem::getSyncMsg
 * 同步状态码转状态说明
 * @param code 状态码
 * @return
 */
QString AccountItem::getSyncMsg(DAccount::AccountSyncState code)
{
    qCDebug(ClientLogger) << "Getting sync message for state code:" << code;
    QString msg = "";
    switch (code) {
    case DAccount::Sync_Normal:
        qCDebug(ClientLogger) << "Sync state: Normal";
        msg = tr("Sync successful");
        break;
    case DAccount::Sync_NetworkAnomaly:
        qCDebug(ClientLogger) << "Sync state: Network anomaly";
        msg = tr("Network error");
        break;
    case DAccount::Sync_ServerException:
        qCDebug(ClientLogger) << "Sync state: Server exception";
        msg = tr("Server exception");
        break;
    case DAccount::Sync_StorageFull:
        qCDebug(ClientLogger) << "Sync state: Storage full";
        msg = tr("Storage full");
        break;
    }
    return msg;
}

/**
 * @brief AccountItem::resetAccount
 * 重新获取帐户相关数据
 */
void AccountItem::resetAccount()
{
    qCDebug(ClientLogger) << "Resetting account data for:" << m_account->accountName();
    querySchedulesWithParameter(QDate().currentDate().year());
    m_dbusRequest->getScheduleTypeList();
    m_dbusRequest->getSysColors();
}

/**
 * @brief AccountItem::getAccount
 * 获取帐户数据
 * @return
 */
DAccount::Ptr AccountItem::getAccount()
{
    return m_account;
}

//获取日程
QMap<QDate, DSchedule::List> AccountItem::getScheduleMap()
{
    // qCDebug(ClientLogger) << "Getting schedule map with" << m_scheduleMap.size() << "dates for account:" << m_account->accountName();
    return m_scheduleMap;
}

QMap<QDate, DSchedule::List> AccountItem::getSearchScheduleMap()
{
    // qCDebug(ClientLogger) << "Getting search schedule map with" << m_searchedScheduleMap.size() << "dates for account:" << m_account->accountName();
    return m_searchedScheduleMap;
}

/**
 * @brief getScheduleTypeList      获取日程类型信息集
 * @return
 */
DScheduleType::List AccountItem::getScheduleTypeList()
{
    qCDebug(ClientLogger) << "Getting schedule type list for account:" << m_account->accountName();
    DScheduleType::List list;
    for (DScheduleType::Ptr p : m_scheduleTypeList) {
        if (p->privilege() != DScheduleType::None) {
            list.push_back(p);
        }
    }
    qCDebug(ClientLogger) << "Returning" << list.size() << "schedule types";
    return list;
}

//根据类型ID获取日程类型
DScheduleType::Ptr AccountItem::getScheduleTypeByID(const QString &typeID)
{
    qCDebug(ClientLogger) << "Getting schedule type by ID:" << typeID;
    for (DScheduleType::Ptr p : m_scheduleTypeList) {
        if (p->typeID() == typeID) {
            qCDebug(ClientLogger) << "Found schedule type:" << p->displayName();
            return p;
        }
    }
    qCDebug(ClientLogger) << "Schedule type not found for ID:" << typeID;
    return nullptr;
}

DScheduleType::Ptr AccountItem::getScheduleTypeByName(const QString &typeName)
{
    qCDebug(ClientLogger) << "Getting schedule type by name:" << typeName;
    for (DScheduleType::Ptr p : m_scheduleTypeList) {
        if (p->typeName() == typeName || p->displayName() == typeName) {
            qCDebug(ClientLogger) << "Found schedule type with ID:" << p->typeID();
            return p;
        }
    }
    qCDebug(ClientLogger) << "Schedule type not found for name:" << typeName;
    return nullptr;
}

DTypeColor::List AccountItem::getColorTypeList()
{
    // qCDebug(ClientLogger) << "Getting color type list with" << m_typeColorList.size() << "colors";
    return m_typeColorList;
}

/**
 * @brief AccountItem::isCanSyncShedule
 * 获取日程是否可以同步
 * @return
 */
bool AccountItem::isCanSyncShedule()
{
    qCDebug(ClientLogger) << "Checking if can sync schedule for account:" << m_account->accountName();
    if (getAccount()->accountType() != DAccount::Account_UnionID) {
        qCDebug(ClientLogger) << "Not a UnionID account, can sync schedule";
        return true;
    }
    DOANetWorkDBus netManger;
    bool canSync = getAccount()->accountState().testFlag(DAccount::Account_Calendar)
           && getAccount()->accountState().testFlag(DAccount::Account_Open) && netManger.getNetWorkState() == DOANetWorkDBus::Active;
    qCDebug(ClientLogger) << "Can sync schedule:" << canSync;
    return canSync;
}

/**
 * @brief AccountItem::isCanSyncSetting
 * 获取通用设置是否可以同步
 * @return
 */
bool AccountItem::isCanSyncSetting()
{
    qCDebug(ClientLogger) << "Checking if can sync settings for account:" << m_account->accountName();
    if (!getAccount().isNull() && getAccount()->accountType() != DAccount::Account_UnionID) {
        qCDebug(ClientLogger) << "Not a UnionID account, can sync settings";
        return true;
    }
    DOANetWorkDBus netManger;
    bool canSync = getAccount()->accountState().testFlag(DAccount::Account_Setting)
           && getAccount()->accountState().testFlag(DAccount::Account_Open) && netManger.getNetWorkState() == DOANetWorkDBus::Active;
    qCDebug(ClientLogger) << "Can sync settings:" << canSync;
    return canSync;
}

bool AccountItem::isEnableForUosAccount()
{
    qCDebug(ClientLogger) << "Checking if account is enabled for UOS account:" << m_account->accountName();
    if (getAccount()->accountType() != DAccount::Account_UnionID) {
        qCDebug(ClientLogger) << "Not a UnionID account, not enabled for UOS";
        return false;
    }

    DOANetWorkDBus netManger;
    bool isEnabled = getAccount()->accountState().testFlag(DAccount::Account_Open) && netManger.getNetWorkState() == DOANetWorkDBus::Active;
    qCDebug(ClientLogger) << "Is enabled for UOS account:" << isEnabled;
    return isEnabled;
}

/**
 * @brief AccountItem::setAccountExpandStatus
 * 更新帐户列表展开状态
 * @param expandStatus 展开状态
 */
void AccountItem::setAccountExpandStatus(bool expandStatus)
{
    qCDebug(ClientLogger) << "Setting account expand status to:" << expandStatus << "for account:" << m_account->accountName();
    m_account->setIsExpandDisplay(expandStatus);
    m_dbusRequest->setAccountExpandStatus(expandStatus);
}

void AccountItem::setAccountState(DAccount::AccountStates state)
{
    qCDebug(ClientLogger) << "Setting account state to:" << state << "for account:" << m_account->accountName();
    m_account->setAccountState(state);
    m_dbusRequest->setAccountState(state);
    emit signalAccountStateChange();
}

void AccountItem::setSyncFreq(DAccount::SyncFreqType freq)
{
    qCDebug(ClientLogger) << "Setting sync frequency to:" << freq << "for account:" << m_account->accountName();
    m_account->setSyncFreq(freq);
    QString syncFreq = DAccount::syncFreqToJsonString(m_account);
    m_dbusRequest->setSyncFreq(syncFreq);
}

DAccount::AccountStates AccountItem::getAccountState()
{
    DAccount::AccountStates state = m_dbusRequest->getAccountState();
    // qCDebug(ClientLogger) << "Getting account state:" << state << "for account:" << m_account->accountName();
    return state;
}

bool AccountItem::getSyncState()
{
    bool syncState = m_dbusRequest->getSyncState();
    // qCDebug(ClientLogger) << "Getting sync state:" << syncState << "for account:" << m_account->accountName();
    return syncState;
}

DAccount::SyncFreqType AccountItem::getSyncFreq()
{
    qCDebug(ClientLogger) << "Getting sync frequency for account:" << m_account->accountName();
    QString syncFreq = m_dbusRequest->getSyncFreq();
    DAccount::syncFreqFromJsonString(m_account, syncFreq);
    qCDebug(ClientLogger) << "Sync frequency:" << m_account->syncFreq();
    return m_account->syncFreq();
}

/**
 * @brief AccountItem::createJobType
 * 创建日程类型
 * @param typeInfo 日程类型数据
 * @param callback
 */
void AccountItem::createJobType(const DScheduleType::Ptr &typeInfo, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Creating job type:" << typeInfo->displayName() << "for account:" << m_account->accountName();
    m_dbusRequest->setCallbackFunc(callback);
    typeInfo->setPrivilege(DScheduleType::User);
    m_dbusRequest->createScheduleType(typeInfo);
}

/**
 * @brief AccountItem::updateScheduleType
 * 更新类型信息
 * @param typeInfo 新的日程类型数据
 * @param callback
 */
void AccountItem::updateScheduleType(const DScheduleType::Ptr &typeInfo, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Updating schedule type:" << typeInfo->displayName() << "ID:" << typeInfo->typeID() << "for account:" << m_account->accountName();
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->updateScheduleType(typeInfo);
}

/**
 * @brief AccountItem::updateScheduleTypeShowState
 * 更新类型显示状态
 * @param scheduleInfo
 * @param callback
 */
void AccountItem::updateScheduleTypeShowState(const DScheduleType::Ptr &scheduleInfo, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Updating schedule type show state for type:" << scheduleInfo->displayName() << "ID:" << scheduleInfo->typeID();
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->updateScheduleTypeShowState(scheduleInfo);
}

/**
 * @brief AccountItem::deleteScheduleTypeByID
 * 根据类型ID删除日程类型
 * @param typeID    日程类型id
 * @param callback 回调函数
 */
void AccountItem::deleteScheduleTypeByID(const QString &typeID, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Deleting schedule type ID:" << typeID << "for account:" << m_account->accountName();
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->deleteScheduleTypeByID(typeID);
}

/**
 * @brief AccountItem::scheduleTypeIsUsed
 * 类型是否被日程使用
 * @param typeID 日程类型id
 * @param callback 回调函数
 */
bool AccountItem::scheduleTypeIsUsed(const QString &typeID)
{
    qCDebug(ClientLogger) << "Checking if schedule type is used, ID:" << typeID;
    bool isUsed = m_dbusRequest->scheduleTypeByUsed(typeID);
    qCDebug(ClientLogger) << "Schedule type is used:" << isUsed;
    return isUsed;
}

/**
 * @brief AccountItem::createSchedule
 * 创建日程
 * @param scheduleInfo  日程数据
 * @param callback 回调函数
 */
void AccountItem::createSchedule(const DSchedule::Ptr &scheduleInfo, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Creating schedule:" << scheduleInfo->summary() << "for account:" << m_account->accountName();
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->createSchedule(scheduleInfo);
}

/**
 * @brief AccountItem::updateSchedule
 * 更新日程
 * @param scheduleInfo 新的日程数据
 * @param callback 回调函数
 */
void AccountItem::updateSchedule(const DSchedule::Ptr &scheduleInfo, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Updating schedule:" << scheduleInfo->summary() << "ID:" << scheduleInfo->scheduleTypeID() << "for account:" << m_account->accountName();
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->updateSchedule(scheduleInfo);
}

DSchedule::Ptr AccountItem::getScheduleByScheduleID(const QString &scheduleID)
{
    // qCDebug(ClientLogger) << "Getting schedule by ID:" << scheduleID;
    return m_dbusRequest->getScheduleByScheduleID(scheduleID);
}

/**
 * @brief AccountItem::deleteScheduleByID
 * 根据日程ID删除日程
 * @param schedule ID日程id
 * @param callback 回调函数
 */
void AccountItem::deleteScheduleByID(const QString &scheduleID, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Deleting schedule ID:" << scheduleID << "for account:" << m_account->accountName();
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->deleteScheduleByScheduleID(scheduleID);
}

/**
 * @brief AccountItem::deleteSchedulesByTypeID
 * 根据类型ID删除日程
 * @param typeID 日程类型id
 * @param callback 回调函数
 */
void AccountItem::deleteSchedulesByTypeID(const QString &typeID, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Deleting all schedules for type ID:" << typeID << "for account:" << m_account->accountName();
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->deleteSchedulesByScheduleTypeID(typeID);
}

void AccountItem::querySchedulesWithParameter(const int year, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Querying schedules for year:" << year << "account:" << m_account->accountName();
    QDateTime start(QDate(year, 1, 1), QTime(0, 0, 0));
    QDateTime end(QDate(year, 12, 31), QTime(23, 59, 59));
    querySchedulesWithParameter(start, end, callback);
}

void AccountItem::querySchedulesWithParameter(const QDateTime &start, const QDateTime &end, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Querying schedules from" << start.toString() << "to" << end.toString() << "for account:" << m_account->accountName();
    querySchedulesWithParameter("", start, end, callback);
}

void AccountItem::querySchedulesWithParameter(const QString &key, const QDateTime &start, const QDateTime &end, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Querying schedules with key:" << key << "from" << start.toString() << "to" << end.toString() << "for account:" << m_account->accountName();
    DScheduleQueryPar::Ptr ptr;
    ptr.reset(new DScheduleQueryPar);
    ptr->setKey(key);
    ptr->setDtStart(start);
    ptr->setDtEnd(end);
    querySchedulesWithParameter(ptr, callback);
}

/**
 * @brief AccountItem::querySchedulesWithParameter
 * 根据查询条件查询数据
 * @param params 查询条件
 * @param callback 回调函数
 */
void AccountItem::querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Querying schedules with parameters for account:" << m_account->accountName()
                          << "key:" << params->key()
                          << "start:" << params->dtStart().toString()
                          << "end:" << params->dtEnd().toString();
    m_preQuery = params;
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->querySchedulesWithParameter(params);
}

QString AccountItem::querySchedulesByExternal(const QString &key, const QDateTime &start, const QDateTime &end)
{
    qCDebug(ClientLogger) << "Querying schedules by external with key:" << key << "from" << start.toString() << "to" << end.toString() << "for account:" << m_account->accountName();
    DScheduleQueryPar::Ptr ptr;
    ptr.reset(new DScheduleQueryPar);
    ptr->setKey(key);
    ptr->setDtStart(start);
    ptr->setDtEnd(end);
    QString json;
    m_dbusRequest->querySchedulesByExternal(ptr, json);
    return json;
}

bool AccountItem::querySchedulesByExternal(const QString &key, const QDateTime &start, const QDateTime &end, QMap<QDate, DSchedule::List> &out)
{
    qCDebug(ClientLogger) << "Querying schedules by external with key:" << key << "from" << start.toString() << "to" << end.toString() << "for account:" << m_account->accountName();
    DScheduleQueryPar::Ptr ptr;
    ptr.reset(new DScheduleQueryPar);
    ptr->setKey(key);
    ptr->setDtStart(start);
    ptr->setDtEnd(end);
    QString json;
    if (m_dbusRequest->querySchedulesByExternal(ptr, json)) {
        out = DSchedule::fromMapString(json);
        qCDebug(ClientLogger) << "Successfully queried external schedules, got" << out.size() << "dates";
        return true;
    }
    qCDebug(ClientLogger) << "Failed to query external schedules";
    return false;
}

/**
 * @brief AccountItem::slotGetAccountInfoFinish
 * 获取帐户信息完成事件
 * @param account 帐户数据
 */
void AccountItem::slotGetAccountInfoFinish(DAccount::Ptr account)
{
    qCDebug(ClientLogger) << "Received account info update for:" << account->accountName() << "type:" << account->accountType();
    m_account = account;
    emit signalAccountDataUpdate();
}

/**
 * @brief AccountItem::slotGetScheduleTypeListFinish
 * 获取日程类型数据完成事件
 * @param scheduleTypeList 日程类型数据
 */
void AccountItem::slotGetScheduleTypeListFinish(DScheduleType::List scheduleTypeList)
{
    qCDebug(ClientLogger) << "Received" << scheduleTypeList.size() << "schedule types for account:" << m_account->accountName();
    m_scheduleTypeList = scheduleTypeList;
    emit signalScheduleTypeUpdate();
}

/**
 * @brief AccountItem::slotGetScheduleListFinish
 * 获取日程数据完成事件
 * @param map 日程数据
 */
void AccountItem::slotGetScheduleListFinish(QMap<QDate, DSchedule::List> map)
{
    qCDebug(ClientLogger) << "Received schedule list with" << map.size() << "dates for account:" << m_account->accountName();
    m_scheduleMap = map;
    emit signalScheduleUpdate();
}

/**
 * @brief AccountItem::slotSearchScheduleListFinish
 * 搜索日程数据完成事件
 * @param map 日程数据
 */
void AccountItem::slotSearchScheduleListFinish(QMap<QDate, DSchedule::List> map)
{
    qCDebug(ClientLogger) << "Received search schedule list with" << map.size() << "dates for account:" << m_account->accountName();
    m_searchedScheduleMap = map;
    emit signalSearchScheduleUpdate();
}

/**
 * @brief AccountItem::slotGetSysColorsFinish
 * 获取系统颜色完成事件
 */
void AccountItem::slotGetSysColorsFinish(DTypeColor::List typeColorList)
{
    qCDebug(ClientLogger) << "Received" << typeColorList.size() << "system colors for account:" << m_account->accountName();
    m_typeColorList = typeColorList;
}

void AccountItem::slotAccountStateChange(DAccount::AccountStates state)
{
    qCDebug(ClientLogger) << "Account state changed to" << state << "for account:" << m_account->accountName();
    getAccount()->setAccountState(state);
    emit signalAccountStateChange();
}

void AccountItem::slotSyncStateChange(DAccount::AccountSyncState state)
{
    qCDebug(ClientLogger) << "Sync state changed to:" << state << "for account:" << m_account->accountName();
    getAccount()->setSyncState(state);
    emit signalSyncStateChange(state);
}

QString AccountItem::getDtLastUpdate()
{
    QString lastUpdate = m_dbusRequest->getDtLastUpdate();
    // qCDebug(ClientLogger) << "Getting last update time:" << lastUpdate << "for account:" << m_account->accountName();
    return lastUpdate;
}

void AccountItem::slotSearchUpdata()
{
    //如果存在查询则更新查询
    if (nullptr != m_preQuery) {
        qCDebug(ClientLogger) << "Updating search results for account:" << m_account->accountName();
        querySchedulesWithParameter(m_preQuery);
    }
}

void AccountItem::importSchedule(QString icsFilePath, QString typeID, bool cleanExists, CallbackFunc func)
{
    qCDebug(ClientLogger) << "Importing schedule from file:" << icsFilePath << "to type:" << typeID << "clean exists:" << cleanExists << "for account:" << m_account->accountName();
    m_dbusRequest->setCallbackFunc(func);
    m_dbusRequest->importSchedule(icsFilePath, typeID, cleanExists);
}

void AccountItem::exportSchedule(QString icsFilePath, QString typeID, CallbackFunc func)
{
    qCDebug(ClientLogger) << "Exporting schedule to file:" << icsFilePath << "from type:" << typeID << "for account:" << m_account->accountName();
    m_dbusRequest->setCallbackFunc(func);
    m_dbusRequest->exportSchedule(icsFilePath, typeID);
}
