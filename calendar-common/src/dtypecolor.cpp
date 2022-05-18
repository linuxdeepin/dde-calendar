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
#include "dtypecolor.h"

DTypeColor::DTypeColor()
{
}

QString DTypeColor::colorCode() const
{
    return m_colorCode;
}

void DTypeColor::setColorCode(const QString &colorCode)
{
    m_colorCode = colorCode;
}

int DTypeColor::colorID() const
{
    return m_colorID;
}

void DTypeColor::setColorID(int colorID)
{
    m_colorID = colorID;
}

DTypeColor::Privilege DTypeColor::privilege() const
{
    return m_privilege;
}

void DTypeColor::setPrivilege(const Privilege &privilege)
{
    m_privilege = privilege;
}

bool DTypeColor::isSysColorInfo()
{
}
