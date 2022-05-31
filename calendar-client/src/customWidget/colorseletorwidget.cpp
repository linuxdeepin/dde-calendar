/*
* Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
*
* Author:     leilong <leilong@uniontech.com>
*
* Maintainer: leilong <leilong@uniontech.com>
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
#include "colorseletorwidget.h"
#include <QPushButton>

ColorSeletorWidget::ColorSeletorWidget(QWidget *parent) : QWidget(parent)
{
    init();
}

void ColorSeletorWidget::init()
{
    initView();
    m_colorGroup = new QButtonGroup(this);
    m_colorGroup->setExclusive(true);
    connect(m_colorGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &ColorSeletorWidget::slotButtonClicked);

    m_colorInfo.reset(new DTypeColor());
}

void ColorSeletorWidget::resetColorButton(const AccountItem::Ptr& account)
{
    reset();
    if (nullptr == account) {
        return;
    }

    DTypeColor::List colorList =  account->getColorTypeList();
    for (DTypeColor::Ptr &var : colorList) {
        if (DTypeColor::PriSystem == var->privilege()) {
            addColor(var);
        }
    }

    if (m_colorGroup->buttons().size() > 0) {
        m_colorGroup->buttons().at(0)->click();
    }
}

void ColorSeletorWidget::reset()
{
    //清空所有的色彩实体和控件
    m_colorEntityMap.clear();
    QList<QAbstractButton *> buttons = m_colorGroup->buttons();
    for (QAbstractButton *btn : buttons) {
        m_colorGroup->removeButton(btn);
        m_colorLayout->removeWidget(btn);
        delete btn;
    }
    if (m_userColorBtn) {
        m_userColorBtn->deleteLater();
    }
    m_userColorBtn = nullptr;
    m_userColorBtnId = -1;
}

void ColorSeletorWidget::addColor(const DTypeColor::Ptr &cInfo)
{
    static int count = 0;   //静态变量，充当色彩控件id
    count++;
    m_colorEntityMap.insert(count, cInfo);      //映射id与控件
    CRadioButton *radio = new CRadioButton(this);
    radio->setColor(QColor(cInfo->colorCode())); //设置控件颜色
    radio->setFixedSize(18, 18);
    m_colorGroup->addButton(radio, count);
    m_colorLayout->addWidget(radio);
    //TODO
    if (DTypeColor::PriUser == cInfo->privilege()) {
        m_userColorBtn = radio;             //记录用户色彩指针
        m_userColorBtnId = count;
    }
}

DTypeColor::Ptr ColorSeletorWidget::getSelectedColorInfo()
{
    return m_colorInfo;
}

void ColorSeletorWidget::setSelectedColorByIndex(int index)
{
    if (index >= 0 && index < m_colorGroup->buttons().size()) {
        QAbstractButton *but = m_colorGroup->buttons().at(index);
        if (nullptr != but) {
            but->click();
        }
    }
}

void ColorSeletorWidget::setSelectedColorById(int colorId)
{
    //如果是用户自定义颜色则直接选中
    if ((colorId > 9 || colorId < 1) && m_userColorBtn) {
        m_userColorBtn->click();
        return;
    }

    //系统颜色则向后移一位
    auto iterator = m_colorEntityMap.begin();
    while (iterator != m_colorEntityMap.end()) {
        if (iterator.value()->colorID() == colorId) {
            //向后移一位
            iterator++;
            if (iterator == m_colorEntityMap.end() || iterator.key() == m_userColorBtnId) {
                iterator = m_colorEntityMap.begin();
            }
            QAbstractButton *btn = m_colorGroup->button(iterator.key());
            if (btn) {
                btn->click();
            }
            break;
        }
        iterator++;
    }
}

void ColorSeletorWidget::setSelectedColor(const DTypeColor::Ptr &colorInfo)
{
    if (DTypeColor::PriUser == colorInfo->privilege()) {
        setUserColor(colorInfo);
    }

    bool isFind = false;
    //遍历所有控件
    for (QAbstractButton *but : m_colorGroup->buttons()) {
        if (nullptr != but && qobject_cast<CRadioButton *>(but)->getColor().name() == colorInfo->colorCode()) {
            but->click();
            isFind = true;
            break;
        }
    }
    if (!isFind) {
        //TODO:设置颜色
        //        setUserColor(JobTypeColorInfo(0, colorInfo.getColorHex(), TypeUser));
    }
}

void ColorSeletorWidget::initView()
{
    m_colorLayout = new QHBoxLayout();

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addLayout(m_colorLayout);
    QPushButton *addColorBut = new QPushButton();
    addColorBut->setIcon(DStyle().standardIcon(DStyle::SP_IncreaseElement));
    addColorBut->setFixedSize(18, 18);
    addColorBut->setIconSize(QSize(10, 10));
    hLayout->addWidget(addColorBut);
    hLayout->addStretch(1);

    m_colorLayout->setMargin(0);
    m_colorLayout->setSpacing(3);
    hLayout->setMargin(0);
    hLayout->setSpacing(3);

    this->setLayout(hLayout);

    connect(addColorBut, &QPushButton::clicked, this, &ColorSeletorWidget::slotAddColorButClicked);
}

void ColorSeletorWidget::slotButtonClicked(int butId)
{
    auto it = m_colorEntityMap.find(butId);
    if (m_colorEntityMap.end() == it) {
        return;
    }
    DTypeColor::Ptr info = it.value();
    if (info->colorCode() != m_colorInfo->colorCode()) {
        m_colorInfo = info;
        emit signalColorChange(info);
    }
}

void ColorSeletorWidget::slotAddColorButClicked()
{
    CColorPickerWidget colorPicker;

    if (colorPicker.exec()) {
        DTypeColor::Ptr typeColor;
        typeColor.reset(new DTypeColor());
        typeColor->setColorCode("0");
        typeColor->setColorCode(colorPicker.getSelectedColor().name());
        typeColor->setPrivilege(DTypeColor::PriUser);
        setUserColor(typeColor);
        m_userColorBtn->click();
    }
}

void ColorSeletorWidget::setUserColor(const DTypeColor::Ptr &colorInfo)
{
    if (DTypeColor::PriUser != colorInfo->privilege()) {
        return;
    }
    if (nullptr == m_userColorBtn) {
        addColor(colorInfo);
    }
    m_userColorBtn->setColor(colorInfo->colorCode());
    m_colorEntityMap[m_userColorBtnId] = colorInfo;
    m_userColorBtn->click();
}
