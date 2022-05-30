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
#ifndef DTYPECOLOR_H
#define DTYPECOLOR_H

#include <QString>
#include <QSharedPointer>
#include <QVector>

/**
 * @brief The DTypeColor class
 * 类型颜色
 */
class DTypeColor
{
public:
    //用户自定义的可读可编辑
    //系统默认只读
    enum Privilege {
        PriSystem = 1, //系统默认
        PriUser = 7 //用户自定义
    };

    typedef QSharedPointer<DTypeColor> Ptr;
    typedef QVector<Ptr> List;

    DTypeColor();

    QString colorCode() const;
    void setColorCode(const QString &colorCode);

    int colorID() const;
    void setColorID(int colorID);

    Privilege privilege() const;
    void setPrivilege(const Privilege &privilege);

    bool isSysColorInfo();

    bool operator!=(const DTypeColor &color) const;

    static List fromJsonString(const QString &colorJson);
    static QString toJsonString(const List &colorList);

private:
    int m_colorID;
    QString m_colorCode; //颜色码
    Privilege m_privilege;
};

#endif // DTYPECOLOR_H
