/*
   * Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
   *
   * Author:     chenhaifeng <chenhaifeng@uniontech.com>
   *
   * Maintainer: chenhaifeng <chenhaifeng@uniontech.com>
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
#ifndef CTITLEWIDGET_H
#define CTITLEWIDGET_H

#include "cbuttonbox.h"

#include <QWidget>
#include <DSearchEdit>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

class CTitleWidget : public QWidget
{
    Q_OBJECT
public:
    enum Title_State {
        Title_State_Mini, //最小状态显示
        Title_State_Normal //正常状态显示
    };

    explicit CTitleWidget(QWidget *parent = nullptr);
    /**
     * @brief setShowState      设置显示状态
     * @param state
     */
    void setShowState(Title_State state);
    //设置侧边栏状态，status:显示状态
    void setSidebarStatus(bool status);
    void setSidebarCanDisplay(bool can);

    bool getSidevarStatus();

    DButtonBox *buttonBox() const;

    DSearchEdit *searchEdit() const;

    DIconButton *newScheduleBtn() const;

private:
    /**
     * @brief stateUpdate   切换显示状态
     */
    void stateUpdate();

    /**
     * @brief miniStateShowSearchEdit       最小状态下搜索框显示
     */
    void miniStateShowSearchEdit();

    /**
     * @brief normalStateUpdateSearchEditWidth  正常状态下搜索显示
     */
    void normalStateUpdateSearchEditWidth();

    /**
     * @brief updateSidebarIconStatus
     * 更新侧边栏控制图标按钮的状态
     */
    void updateSidebarIconStatus();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *o, QEvent *e) override;
signals:
    void signalSetButtonFocus();
    void signalSearchFocusSwitch();
    //侧边栏状态发生改变
    void signalSidebarStatusChange(bool);
public slots:
    /**
     * @brief slotShowSearchEdit    搜索按钮点击下切换搜索框
     */
    void slotShowSearchEdit();
    /**
     * @brief slotSearchEditFocusChanged        状态切换下，根据搜索框是否有内容或焦点切换显示状态
     * @param onFocus                           搜索框是否有焦点
     */
    void slotSearchEditFocusChanged(bool onFocus);

    void slotSidebarIconClicked();

private:
    bool m_sidebarstatus = true; //侧边栏状态
    bool m_sidebarCanDisplay = true; //侧边栏状态

    DIconButton *m_sidebarIcon {nullptr};   //侧边栏状态控制按钮
    CButtonBox *m_buttonBox {nullptr};
    DSearchEdit *m_searchEdit {nullptr};
    DIconButton *m_newScheduleBtn {nullptr}; //全局的新建日程按钮
    DIconButton *m_searchPush {nullptr};
    Title_State m_showState {Title_State_Normal};
};

#endif // CTITLEWIDGET_H
