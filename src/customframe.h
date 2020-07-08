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
#ifndef CustomFrame_H
#define CustomFrame_H

#include <QFrame>

class CustomFrame : public QFrame
{
    Q_OBJECT
public:
    CustomFrame(QWidget *parent = nullptr);
    void setBColor(QColor normalC);
    void setRoundState(bool lstate, bool tstate, bool rstate, bool bstate);
    void setTextStr(QFont font, QColor tc, QString strc, int flag = Qt::AlignCenter);
    void setTextStr(QString strc);
    void setTextColor(QColor tc);
    void setTextFont(QFont font);
    void setTextAlign(int flag = Qt::AlignCenter);
    void setRadius(int radius = 8);
    void setboreder(int framew = 0);
    QString getTextStr()
    {
        return  m_text;
    }
    void setFixedSize(int w, int h);
protected:
    void paintEvent ( QPaintEvent *e);
private:
    QColor                   m_bnormalColor = "#FFFFFF";
    QColor                   m_tnormalColor = "#000000";
    QFont                    m_font;
    bool                     m_bflag = false;
    bool                     m_fixsizeflag = false;
    int                      m_textflag = Qt::AlignCenter;//对齐方式
    QString                  m_text;
    int                      m_radius = 8;
    int                      m_borderframew = 0;
    bool                     m_lstate = false;
    bool                     m_tstate = false;
    bool                     m_rstate = false;
    bool                     m_bstate = false;
};

#endif // TODYBUTTON_H
