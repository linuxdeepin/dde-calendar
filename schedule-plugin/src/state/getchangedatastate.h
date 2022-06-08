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
#ifndef GETCHANGEDATASTATE_H
#define GETCHANGEDATASTATE_H

#include "schedulestate.h"
/**
 * @brief The getChangeDataState class      获取修改信息状态
 */
class getChangeDataState : public scheduleState
{
public:
    explicit getChangeDataState(scheduleBaseTask *task);
    /**
     * @brief getReplyByIntent  根据意图判断回复
     * @param isOK  确认 或 取消
     * @return  回复
     */
    Reply getReplyByIntent(bool isOK) override;
protected:
    /**
     * @brief eventFilter   根据json过滤事件
     * @param jsonData      json对象
     * @return  返回过滤结果 初始化，错误 或正常
     */
    Filter_Flag eventFilter(const JsonData *jsonData) override;
    /**
     * @brief ErrEvent  错误事件处理
     * @return  给助手的回复
     */
    Reply ErrEvent() override;
    /**
     * @brief normalEvent   正常事件处理
     * @param jsonData      需要处理的json数据
     * @return  给助手的回复
     */
    Reply normalEvent(const JsonData *jsonData) override;
};
#endif // GETCHANGEDATASTATE_H
