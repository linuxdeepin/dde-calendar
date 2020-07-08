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
#ifndef CDYNAMICICON_H
#define CDYNAMICICON_H

#include <QPixmap>
#include <QDate>
#include <QIcon>
#include <QSvgRenderer>

#include <DApplication>
#include <DTitlebar>
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class CDynamicIcon
{
private:
    CDynamicIcon(int width, int height);
    ~CDynamicIcon();
public:
    static CDynamicIcon *getInstance();
    static void releaseInstance();
public:
    void setDate(const QDate &date);
    QDate getDate() const
    {
        return m_Date;
    }
    void setIcon();
    void setTitlebar(DTitlebar *titlebar);
    QPixmap getPixmap() const
    {
        return *m_pixmap;
    }
public:
    static CDynamicIcon *m_Icon;
private:
    void paintPixmap(QPixmap *pixmap);
private:
    QPixmap         *m_pixmap;
    QDate           m_Date;

    QSvgRenderer    *m_Dayrenderer;
    QSvgRenderer    *m_Weekrenderer;
    QSvgRenderer    *m_Monthrenderer;
    QSvgRenderer    *m_backgroundrenderer;

    DTitlebar       *m_Titlebar {nullptr};


};

#endif // CDYNAMICICON_H
