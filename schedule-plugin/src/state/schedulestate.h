/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
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
#ifndef SCHEDULESTATE_H
#define SCHEDULESTATE_H

#include "../data/jsondata.h"
#include "../interface/reply.h"
#include "../data/clocaldata.h"

class JsonData;
class CSchedulesDBus;
class scheduleBaseTask;
class scheduleState
{
public:
    enum Filter_Flag { Fileter_Err,
                       Fileter_Normal,
                       Fileter_Init
    };

public:
    scheduleState(scheduleBaseTask *task);
    virtual ~scheduleState();
    Reply process(const JsonData *jsonData);
    void setNextState(scheduleState *nextState);
    scheduleState *getNextState() const;
    void setLocalData(const CLocalData::Ptr &localData);
    CLocalData::Ptr getLocalData() const;
    virtual Reply getReplyByIntent(bool isOK) = 0;

protected:
    virtual Filter_Flag eventFilter(const JsonData *jsonData) = 0;
    virtual Reply ErrEvent() = 0;
    virtual Reply normalEvent(const JsonData *jsonData) = 0;
    Reply initEvent(const JsonData *jsonData);
    /**
     * @brief changeDateErrJudge        修改数据错误判断，在非获取修改数据状态下，输入“修改到xxx”返回错误信息
     * @param jsonData          json数据
     * @param defaultflag       默认返回标志
     * @return          过滤标志
     */
    Filter_Flag changeDateErrJudge(const JsonData *jsonData, const Filter_Flag &defaultflag);

protected:
    scheduleBaseTask *m_Task {nullptr};
    scheduleState *m_nextState {nullptr};
    CLocalData::Ptr m_localData {nullptr};
};

#endif // SCHEDULESTATE_H
