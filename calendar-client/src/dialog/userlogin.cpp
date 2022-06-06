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
#include "userlogin.h"

#include <QLabel>
#include <DIconButton>
#include <DPushButton>
#include <QHBoxLayout>
#include <DSettingsOption>
#include <DCommandLinkButton>
#include <DApplicationHelper>
#include <DSettingsWidgetFactory>

Userlogin::Userlogin(QObject *parent)
    : QObject(parent)
    , m_loginStatus(false)
{

}

Userlogin::~Userlogin()
{

}

QPair<QWidget*, QWidget*> Userlogin::createloginButton(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    // 构建自定义Item
    QWidget* widget = new QWidget();

    DIconButton *buttonImg = new DIconButton(widget);
    QPushButton* buttonLogin = new QPushButton(widget);
    QHBoxLayout* layout = new QHBoxLayout(widget);

    const QIcon &icon = QIcon::fromTheme(":/resources/icon/account_light.svg");
    buttonImg->setIcon(icon);
    buttonImg->setIconSize(QSize(36,36));
    buttonImg->setStyleSheet("border:0px solid;");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(buttonImg);
    layout->addStretch();
    buttonLogin->setText(tr("Sign In","button"));
    layout->addWidget(buttonLogin);
    widget->layout()->setAlignment(Qt::AlignLeft);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, widget);
    // 获取初始值
    option->connect(buttonLogin,&DIconButton::clicked, option, [=](bool checked){
        if (checked == option->value().toBool())
        {
            buttonLogin->setHidden(true);
            buttonImg->setHidden(true);
            QPushButton* buttonLogout = new QPushButton(widget);
            layout->addWidget(buttonLogout);
            buttonLogout->setText(tr("Sign Out","button"));
        }
    });
    option->setValue(option->defaultValue());
    if (widget != nullptr)
        widget->deleteLater();
    return optionWidget;
}

void Userlogin::userInfoChanged(const QString &str, const QVariantMap &map, const QStringList &list)
{
    Q_UNUSED(str);
    Q_UNUSED(list);
    if (!map.contains("UserInfo"))
        return;

    const QDBusArgument &dbusArg = map.value("UserInfo").value<QDBusArgument>();
    addUserInfo(dbusArg);
}

void Userlogin::addUserInfo(const QDBusArgument &argument)
{
    QVariantMap userInfoMap;
    argument >> userInfoMap;

    if (userInfoMap.value("IsLoggedIn").toBool()) {
        m_loginStatus = true;
        m_isManualQuit = false;
        m_accountInfo.userName = userInfoMap.value("Username").toString();
        m_accountInfo.avatarPath = userInfoMap.value("ProfileImage").toString();
        m_accountInfo.nickName = userInfoMap.value("Nickname").toString();
        m_accountInfo.userId = userInfoMap.value("UserID").toString();
//        emit loggedIn();
    } else {
        m_loginStatus = false;
        m_accountInfo.clear();
//        emit loggedOut(m_isManualQuit);
    }
}
