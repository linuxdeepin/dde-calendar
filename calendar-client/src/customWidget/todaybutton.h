// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TODYBUTTON_H
#define TODYBUTTON_H

#include <DPushButton>

DWIDGET_USE_NAMESPACE
class CTodayButton : public DPushButton
{
    Q_OBJECT
public:
    explicit CTodayButton(QWidget *parent = nullptr);
    void setBColor(QColor normalC, QColor hoverC, QColor pressc, QColor normalC1, QColor hoverC1, QColor pressc1);
    void setTColor(QColor normalC, QColor hoverC, QColor pressc);
    void setshadowColor(QColor sc);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QColor                   m_bnormalColor = "#FFFFFF";
    QColor                   m_bhoverColor = "#FFFFFF";
    QColor                   m_bPressColor = "#FFFFFF";
    QColor                   m_dbnormalColor = "#FFFFFF";
    QColor                   m_dbhoverColor = "#FFFFFF";
    QColor                   m_dbPressColor = "#FFFFFF";
    QColor                   m_tnormalColor = "#000000";
    QColor                   m_thoverColor = "#000000";
    QColor                   m_tPressColor = "#000000";
    QColor                   m_shadowColor  = "#FFFFFF";
    QFont                    m_font;
    bool                     m_pressstate = false;
    bool                     m_hovertate = false;
};

#endif // TODYBUTTON_H
