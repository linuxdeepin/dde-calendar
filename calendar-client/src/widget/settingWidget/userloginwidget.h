/*
* Copyright (C) 2019 ~ 2022 Uniontech Software Technology Co.,Ltd.
*
* Author:     xuezifan  <xuezifan@uniontech.com>
*
* Maintainer: xuezifan  <xuezifan@uniontech.com>
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
#ifndef USERLOGIN_H
#define USERLOGIN_H

#include "dsettingsdialog.h"
#include "doanetworkdbus.h"
#include <DLabel>
#include <DIconButton>
#include <DPushButton>
#include <QNetworkAccessManager>
DWIDGET_USE_NAMESPACE

//配置界面账户登录部件
class UserloginWidget: public QWidget
{
    Q_OBJECT
public:
    explicit UserloginWidget(QWidget *parent = nullptr);
    virtual ~UserloginWidget();

    static QPair<QWidget *, QWidget *> createloginButton(QObject *obj);

signals:

public slots:
    void slotLoginBtnClicked();
    void slotLogoutBtnClicked();
    void slotAccountUpdate();
    //网络图片请求完成事件
    void slotReplyPixmapLoad(QNetworkReply *);
    // 网络状态发生改变
    void slotNetworkStateChange(DOANetWorkDBus::NetWorkState state);


private:
    void initView();
    void initConnect();

private:
    bool m_loginStatus;
    bool m_isManualQuit = false;
    DLabel *m_userNameLabel = nullptr;
    DIconButton *m_buttonImg = nullptr;
    QPushButton *m_buttonLogin = nullptr;
    QPushButton *m_buttonLoginOut = nullptr;
    QNetworkAccessManager *m_networkManager;
    DOANetWorkDBus   * m_ptrDoaNetwork;
};

#endif // USERLOGIN_H
