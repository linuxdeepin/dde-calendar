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

#include <DCommandLinkButton>
#include <DSettingsWidgetFactory>
#include <DPushButton>
#include <QHBoxLayout>
#include <DSettingsOption>
#include <DIconButton>
#include <DApplicationHelper>

Userlogin::Userlogin(QObject *parent)
    : QObject(parent)
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
    QPushButton* button = new QPushButton(widget);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    QPixmap pixmaplight;
    QPixmap pixmapdark;

    pixmaplight.load(":/resources/icon/account_light.svg");
//    pixmap = pixmap.scaled(59,100,Qt::IgnoreAspectRatio);
    pixmapdark.load(":/resources/icon/account_dark.svg");
    buttonImg->setStyleSheet("border:0px solid;");
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        buttonImg->setIcon(pixmaplight);
    }else {
        buttonImg->setIcon(pixmapdark);
    }
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(buttonImg);
    layout->addStretch();
    button->setText(tr("login","button"));
    layout->addWidget(button);
    widget->layout()->setAlignment(Qt::AlignLeft);

    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, widget);
    // 获取初始值
    option->setValue(option->defaultValue());
    if (widget != nullptr)
        widget->deleteLater();
    return optionWidget;
}