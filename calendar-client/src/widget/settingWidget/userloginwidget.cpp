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
#include "userloginwidget.h"
#include "accountmanager.h"

#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <QHBoxLayout>
#include <QNetworkReply>

UserloginWidget::UserloginWidget(QWidget *parent)
    : QWidget(parent)
    , m_loginStatus(false)
{
    initView();
    initConnect();
    slotAccountUpdate();
}

UserloginWidget::~UserloginWidget()
{

}

void UserloginWidget::initView()
{
    m_userNameLabel = new DLabel();
    m_userNameLabel->setElideMode(Qt::ElideMiddle);
    m_buttonImg = new DIconButton(this);
    m_buttonLogin = new QPushButton(this);
    m_buttonLoginOut = new QPushButton(this);
    m_buttonLogin->setFixedSize(98, 36);
    m_buttonLoginOut->setFixedSize(98, 36);
    QHBoxLayout* layout = new QHBoxLayout(this);
    const QIcon &icon = QIcon::fromTheme(":/resources/icon/account_light.svg");
    m_buttonImg->setIcon(icon);
    m_buttonImg->setIconSize(QSize(36,36));
    m_buttonImg->setStyleSheet("border:0px solid;");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_buttonImg);
    layout->addSpacing(5);
    layout->addWidget(m_userNameLabel);
    layout->addStretch();
    m_buttonLogin->setText(tr("Sign In","button"));
    m_buttonLoginOut->setText(tr("Sign Out","button"));
    m_buttonLoginOut->hide();
    layout->addWidget(m_buttonLogin);
    layout->addWidget(m_buttonLoginOut);
    this->layout()->setAlignment(Qt::AlignLeft);

    m_networkManager = new QNetworkAccessManager(this);
}

void UserloginWidget::initConnect()
{
    connect(m_buttonLogin, &QPushButton::clicked, this, &UserloginWidget::slotLoginBtnClicked);
    connect(m_buttonLoginOut, &QPushButton::clicked, this, &UserloginWidget::slotLogoutBtnClicked);
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, &UserloginWidget::slotAccountUpdate);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &UserloginWidget::slotReplyPixmapLoad);
}

void UserloginWidget::slotLoginBtnClicked()
{
    gAccountManager->login();
}

void UserloginWidget::slotLogoutBtnClicked()
{
    gAccountManager->loginout();
}

void UserloginWidget::slotAccountUpdate()
{
    if (gUosAccountItem) {
        //账户为登录状态
        m_buttonLogin->hide();
        m_buttonLoginOut->show();
        DAccount::Ptr account = gUosAccountItem->getAccount();
        m_userNameLabel->setText(account->accountName());
        // 这里的url一定要带上http://头的， 跟在浏览器里输入其它链接不太一样，浏览器里面会自动转的，这里需要手动加上。
        m_networkManager->get(QNetworkRequest(account->avatar()));

    } else {
        //账户为未登录状态
        m_buttonLoginOut->hide();
        m_buttonLogin->show();
        m_userNameLabel->setText("");
        m_buttonImg->setIcon(QIcon::fromTheme(":/resources/icon/account_light.svg"));
    }
}

void UserloginWidget::slotReplyPixmapLoad(QNetworkReply* reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply->readAll());
    m_buttonImg->setIcon(pixmap);
}

QPair<QWidget*, QWidget*> UserloginWidget::createloginButton(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, new UserloginWidget());
    return optionWidget;
}
