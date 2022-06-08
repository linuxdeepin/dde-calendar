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
#include "selectandquerystate.h"
#include "../globaldef.h"
#include "../task/schedulebasetask.h"
#include "../data/changejsondata.h"

SelectAndQueryState::SelectAndQueryState(scheduleBaseTask *task)
    : scheduleState(task)
{
}

Reply SelectAndQueryState::getReplyByIntent(bool isOK)
{
    Q_UNUSED(isOK)
    Reply reply;
    REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, true)
    return reply;
}

scheduleState::Filter_Flag SelectAndQueryState::eventFilter(const JsonData *jsonData)
{
    //如果语义包含全部关键字则为修改初始状态
    if (jsonData->getPropertyStatus() == JsonData::ALL
        //如果语义包含下一个关键字则为修改初始状态
        || jsonData->getPropertyStatus() == JsonData::NEXT
        || jsonData->isVaild()
        //如果语义包含时间则为修改初始状态
        || jsonData->getDateTime().suggestDatetime.size() > 0
        // 如果语义包含内容则为修改初始状态
        || !jsonData->TitleName().isEmpty()
        //如果语义包含重复类型则为修改初始状态
        || jsonData->getRepeatStatus() != JsonData::NONE) {
        return Filter_Flag::Fileter_Init;
    }
    if (jsonData->getPropertyStatus() == JsonData::LAST)
        return Fileter_Normal;
    JsonData *queryData = const_cast<JsonData *>(jsonData);
    changejsondata *mchangeJsonData = dynamic_cast<changejsondata *>(queryData);
    if (mchangeJsonData->fromDateTime().suggestDatetime.size() > 0) {
        return Filter_Flag::Fileter_Init;
    }
    //根据列表编号判断
    if (m_localData->getOffet() < 0 && jsonData->offset() < 0) {
        return Fileter_Err;
    }
    bool showOpenWidget = m_localData->scheduleInfoVector().size() > ITEM_SHOW_NUM;
    const int showcount = showOpenWidget ? ITEM_SHOW_NUM : m_localData->scheduleInfoVector().size();
    if (jsonData->offset() > showcount) {
        return Fileter_Err;
    }
    return Fileter_Normal;
}

Reply SelectAndQueryState::ErrEvent()
{
    Reply reply;
    REPLY_ONLY_TTS(reply, QUERY_ERR_TTS, QUERY_ERR_TTS, true)
    return reply;
}

Reply SelectAndQueryState::normalEvent(const JsonData *jsonData)
{
    bool showOpenWidget = m_localData->scheduleInfoVector().size() > ITEM_SHOW_NUM;
    const int showcount = showOpenWidget ? ITEM_SHOW_NUM : m_localData->scheduleInfoVector().size();
    int offset = 0;
    //获取第N个日程
    if (jsonData->getPropertyStatus() == JsonData::LAST) {
        offset = showcount;
    } else {
        offset = jsonData->offset();
    }
    if (offset > 0) {
        m_localData->setOffset(offset);
        m_localData->setSelectInfo(m_localData->scheduleInfoVector().at(offset - 1));
        DSchedule::Ptr info = m_localData->SelectInfo();
        //如果语义为“第xx个修改到xxx”，添加对修改信息的获取
        //类型转换
        JsonData *queryData = const_cast<JsonData *>(jsonData);
        changejsondata *mchangeJsonData = dynamic_cast<changejsondata *>(queryData);
        //如果有修改时间的信息则赋值
        if (mchangeJsonData->toDateTime().suggestDatetime.size() > 0) {
            m_localData->setToTime(mchangeJsonData->toDateTime());
        }
        //如果有修改内容的信息则获取
        if (!mchangeJsonData->toPlaceStr().isEmpty()) {
            m_localData->setToTitleName(mchangeJsonData->toPlaceStr());
        }
        return m_Task->getReplyBySelectSchedule(m_localData->SelectInfo());
    } else {
        qDebug() << "offset <=0";
        Reply reply;
        REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, false);
        return reply;
    }
}
