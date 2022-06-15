/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
#ifndef DDATASYNCBASE_H
#define DDATASYNCBASE_H

#include "daccount.h"
#include "daccountdatabase.h"
#include "syncfilemanage.h"

#include <QObject>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QFlags>

#ifndef Q_MOC_RUN
#define Q_DECLARE_FLAGS(Flags, Enum)\
typedef QFlags<Enum> Flags;
#endif

/**
 * @brief The DDataSyncBase class 云同步的基类
 */
class DDataSyncBase : public QObject
{
    Q_OBJECT
public:
    enum UpdateType {
        Update_None = 0, /*无*/
        Update_Schedule = 1, /*更新日程*/
        Update_ScheduleType = 2, /*更新日程类型*/
        Update_Setting = 4, /*更新设置*/
    };
    Q_DECLARE_FLAGS(UpdateTypes, UpdateType)

    enum SyncType {
        Sync_None = 0, /*无*/
        Sync_Upload = 1, /*上传*/
        Sync_Download = 2, /*下载*/
    };
    Q_DECLARE_FLAGS(SyncTypes, SyncType)
    DDataSyncBase();
    virtual ~DDataSyncBase();
    virtual void syncData(QString accountId, QString accountName, int accountState, DDataSyncBase::SyncTypes syncType) = 0;
signals:
    //
    void signalUpdate(const UpdateTypes updateType);
    void signalSyncState(const int syncState);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DDataSyncBase::UpdateTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(DDataSyncBase::SyncTypes)


#endif // DDATASYNCBASE_H
