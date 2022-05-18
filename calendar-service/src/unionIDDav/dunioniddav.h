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
#ifndef DUNIONIDDAV_H
#define DUNIONIDDAV_H

/**
 * @brief The DUnionIDDav class
 * Union ID 云同步
 */
class DUnionIDDav
{
public:
    DUnionIDDav();
    //创建帐户信息文件
    void createAccountFile();
    //创建类型文件
    void createTypeFile();
    //创建日程文件
    void createCalendarFile();
    //删除文件夹
    void deleteFolder();
    //创建文件夹
    void createFolder();
    //判断文件夹是否存在
    bool exists();
};

#endif // DUNIONIDDAV_H
