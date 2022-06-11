/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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
#pragma once

#include "ccustomtimeedit.h"

#include <DComboBox>
#include <DLineEdit>
#include <DListWidget>

#include <QTime>
#include <QTimeEdit>

DWIDGET_USE_NAMESPACE

class CTimeEdit : public DComboBox
{
    Q_OBJECT
public:
    explicit CTimeEdit(QWidget *parent = nullptr);
    ~CTimeEdit() override;

    /**
     * @brief setMineTime   设置下拉列表顶端时间
     * @param mineTime
     */
    void setMineTime(const QTime &mineTime);

    /**
     * @brief setTime       设置显示时间
     * @param time
     */
    void setTime(const QTime &time);

    /**
     * @brief getTime       获取时间
     * @return
     */
    QTime getTime();

    /**
     * @brief updateListItem        更新列表项
     * @param isShowTimeInterval     是否显示时间间隔
     */
    void updateListItem(bool isShowTimeInterval = false);

    /**
     * @brief setSelectItem     设置选中项目和scroll定位位置
     * @param time
     */
    void setSelectItem(const QTime &time);
public slots:
    void setTimeFormat(int value);
    //焦点获取效果绘制
    void slotFocusDraw(bool showFocus);
    

    /**
     * @brief slotSetPlainText      设置当前编辑框显示文本     
     * @param arg
     */
    void slotSetPlainText(const QString &arg);

    /**
     * @brief slotActivated         Combobox选择项处理槽
     * @param arg
     */
    void slotActivated(const QString &arg);

    /**
     * @brief slotEditingFinished       编辑框编辑结束时，处理槽
     */
    void slotEditingFinished();
signals:
    void signalFocusOut();
    void signaleditingFinished();
private:
    void initUI();
    void initConnection();
protected:
    void showPopup() override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *e) override;
private:
    QString m_timeFormat = "hh:mm";
    QTime m_time;
    CCustomTimeEdit *m_timeEdit = nullptr;
    bool m_hasFocus;
    QTime m_miniTime;
    bool m_isShowTimeInterval = false;
    QModelIndex scrollPosition;     //下拉列表需要滚动到的位置
};
