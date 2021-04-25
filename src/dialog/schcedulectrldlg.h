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

#include <DDialog>
#include <DPushButton>
#include <DSuggestButton>

#include <QObject>
#include <QLabel>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
/**
 * @brief The CSchceduleCtrlDlg class
 */
class CSchceduleCtrlDlg : public DDialog
{
    Q_OBJECT
public:
    explicit CSchceduleCtrlDlg(QWidget *parent = nullptr);
    QAbstractButton *addPushButton(QString btName, bool type = false);
    QAbstractButton *addsuggestButton(QString btName, bool type = false);
    QAbstractButton *addWaringButton(QString btName, bool type = false);
    void setText(QString str);
    void setInformativeText(QString str);
    int clickButton();
private:
    void initUI();
    void initConnection();
    void setTheMe(const int type);
    void setPaletteTextColor(QWidget *widget,QColor textColor);
protected:
    void changeEvent(QEvent *event) override;
private slots:
    void buttonJudge(int id);
private:
    /**
     * @brief m_firstLabel
     *
     * 标题下面的第一行label
     */
    QLabel                           *m_firstLabel = nullptr;
    /**
     * @brief m_seconLabel
     *
     * 标题下面的第二行label
     */
    QLabel                           *m_seconLabel = nullptr;
    //图表
    QLabel *m_icon = nullptr;
    //title label
    QLabel *m_Title = nullptr;
    /**
     * @brief m_id
     *
     * 按钮id
     *
     * 根据id，判断点击的按钮
     */
    int                              m_id = -1;
    //buttongroup
    QButtonGroup *m_Buttongroup = nullptr;
    //button list
    QVector<QPushButton *> m_buttonlist;
    //main布局
    QVBoxLayout *m_mainBoxLayout = nullptr;
    //button布局
    QHBoxLayout *m_btBoxLayout = nullptr;
    //字体
    QFont labelF;
    QFont labelT;
    //按钮文字
    QVector<QString> str_btName;
};

#endif // MYSCHCEDULEVIEW_H
