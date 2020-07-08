/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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
#ifndef CSCHCEDULECTRLDLG_H
#define CSCHCEDULECTRLDLG_H

#include <QObject>
#include <QLabel>
#include <DDialog>
#include <DPushButton>
#include <DSuggestButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
DWIDGET_USE_NAMESPACE

class CSchceduleCtrlDlg : public DDialog
{
    Q_OBJECT
public:
    explicit CSchceduleCtrlDlg(QWidget *parent = nullptr);
    QAbstractButton *addPushButton(QString btName, bool type = false);
    QAbstractButton *addsuggestButton(QString btName, bool type = false);
    QAbstractButton *addWaringButton(QString btName, bool type = false);
    void setTitleName(QString tileName);
    void setText(QString str);
    void setInformativeText(QString str);
    int clickButton();
//    void updatesize();
private:
    void initUI();
    void initConnection();

protected:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;
private slots:
    void buttonJudge(int id);
private:
    QLabel                           *m_firstLabel = nullptr;
    QLabel                           *m_seconLabel = nullptr;
    QLabel                           *m_icon;
    QLabel                           *m_Title;
    int                              m_id = -1;
    QButtonGroup                     *m_Buttongroup;
    QVector<QPushButton *>            m_buttonlist;
    QVBoxLayout                      *m_mainBoxLayout;
    QHBoxLayout                      *m_btBoxLayout;
    QFont labelF;
    QFont labelT;
    QVector<QString> str_btName;
};

#endif // MYSCHCEDULEVIEW_H
