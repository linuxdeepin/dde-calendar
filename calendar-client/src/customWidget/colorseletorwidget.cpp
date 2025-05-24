// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "colorseletorwidget.h"
#include "configsettings.h"
#include "units.h"
#include <DLog>

#include <QPushButton>

Q_LOGGING_CATEGORY(colorSelectorLog, "calendar.widget.colorselector")

ColorSeletorWidget::ColorSeletorWidget(QWidget *parent) : QWidget(parent)
{
    qCDebug(colorSelectorLog) << "Initializing ColorSeletorWidget";
    init();
}

void ColorSeletorWidget::init()
{
    qCDebug(colorSelectorLog) << "Setting up color selector widget";
    initView();
    m_colorGroup = new QButtonGroup(this);
    m_colorGroup->setExclusive(true);
    connect(m_colorGroup, &QButtonGroup::idClicked, this, &ColorSeletorWidget::slotButtonClicked);

    m_colorInfo.reset(new DTypeColor());
}

void ColorSeletorWidget::resetColorButton(const AccountItem::Ptr &account)
{
    qCDebug(colorSelectorLog) << "Resetting color buttons for account";
    reset();
    if (nullptr == account) {
        qCWarning(colorSelectorLog) << "Account is null, skipping color button reset";
        return;
    }

    DTypeColor::List colorList =  account->getColorTypeList();
    //添加默认颜色控件
    for (DTypeColor::Ptr &var : colorList) {
        if (DTypeColor::PriSystem == var->privilege()) {
            addColor(var);
        }
    }

    //自定义控件单独添加
    m_colorGroup->addButton(m_userColorBtn, m_userColorBtnId);
    m_colorLayout->addWidget(m_userColorBtn);

    if (m_colorGroup->buttons().size() > 0) {
        m_colorGroup->buttons().at(0)->click();
    }
}

void ColorSeletorWidget::reset()
{
    qCDebug(colorSelectorLog) << "Resetting color selector widget state";
    //清空所有的色彩实体和控件
    m_colorEntityMap.clear();
    QList<QAbstractButton *> buttons = m_colorGroup->buttons();
    for (QAbstractButton *btn : buttons) {
        m_colorGroup->removeButton(btn);
        m_colorLayout->removeWidget(btn);
        //自定义控件内存不释放
        if (btn == m_userColorBtn) {
            continue;
        }
        delete btn;
        btn = nullptr;
    }
    if (nullptr == m_userColorBtn) {
        qCDebug(colorSelectorLog) << "Creating new user color button";
        m_userColorBtn = new CRadioButton(this);
        m_userColorBtn->setFixedSize(18, 18);
    }
    m_userColorBtn->hide();
}

void ColorSeletorWidget::addColor(const DTypeColor::Ptr &cInfo)
{
    static int count = 0;   //静态变量，充当色彩控件id
    count++;
    qCDebug(colorSelectorLog) << "Adding new color with ID:" << count << "Color code:" << cInfo->colorCode();
    m_colorEntityMap.insert(count, cInfo); //映射id与控件,从1开始
    CRadioButton *radio = new CRadioButton(this);
    radio->setColor(QColor(cInfo->colorCode())); //设置控件颜色
    radio->setFixedSize(18, 18);
    m_colorGroup->addButton(radio, count);
    m_colorLayout->addWidget(radio);
}

DTypeColor::Ptr ColorSeletorWidget::getSelectedColorInfo()
{
    qCDebug(colorSelectorLog) << "Getting selected color info";
    if ( m_colorInfo->privilege() == DTypeColor::PriSystem) {
        CConfigSettings::getInstance()->setOption("LastSysColorTypeNo", m_colorInfo->colorID());
    } else if (!m_colorInfo->colorCode().isEmpty() ){
        CConfigSettings::getInstance()->setOption("LastUserColor", m_colorInfo->colorCode());
        CConfigSettings::getInstance()->setOption("LastSysColorTypeNo", "");
    }
    return m_colorInfo;
}

void ColorSeletorWidget::setSelectedColorByIndex(int index)
{
    qCDebug(colorSelectorLog) << "Setting selected color by index:" << index;
    if (index >= 0 && index < m_colorGroup->buttons().size()) {
        QAbstractButton *but = m_colorGroup->buttons().at(index);
        if (nullptr != but) {
            but->click();
        }
    } else {
        qCWarning(colorSelectorLog) << "Invalid color index:" << index;
    }
}

void ColorSeletorWidget::setSelectedColorById(int colorId)
{
    qCDebug(colorSelectorLog) << "Setting selected color by ID:" << colorId;
    //默认选择第一个
    if( colorId < 0 ) {
        if (m_colorGroup->buttons().size() > 0) {
            m_colorGroup->buttons().at(0)->click();
        }
        return;
    } else if ( colorId == 9 ) {
        m_userColorBtn->click();
        return;
    }

    //系统颜色则向后移一位
    if (colorId == 8) {
        colorId = 0;
    } else {
        ++colorId;
    }
    if (m_colorGroup->buttons().size() > colorId) {
         m_colorGroup->buttons().at(colorId)->click();
    } else {
        qCWarning(colorSelectorLog) << "Invalid color ID:" << colorId;
    }
}

void ColorSeletorWidget::setSelectedColor(const DTypeColor &colorInfo)
{
    qCDebug(colorSelectorLog) << "Setting selected color with color info, ID:" << colorInfo.colorID();
    bool finding = false;
    auto iterator = m_colorEntityMap.begin();
    while (iterator != m_colorEntityMap.end()) {
        if (iterator.value()->colorID() == colorInfo.colorID()) {
            QAbstractButton *btn = m_colorGroup->button(iterator.key());
            if (btn) {
                btn->click();
                finding = true;
            }
            break;
        }
        iterator++;
    }
    if (!finding) {
        qCDebug(colorSelectorLog) << "Color not found in existing buttons, creating new user color";
        DTypeColor::Ptr ptr;
        ptr.reset(new DTypeColor(colorInfo));
        setUserColor(ptr);
    }
}

void ColorSeletorWidget::initView()
{
    qCDebug(colorSelectorLog) << "Initializing color selector view";
    m_colorLayout = new QHBoxLayout();

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addLayout(m_colorLayout);
    QPushButton *addColorBut = new QPushButton();
    addColorBut->setIcon(DStyle().standardIcon(DStyle::SP_IncreaseElement));
    addColorBut->setFixedSize(18, 18);
    addColorBut->setIconSize(QSize(10, 10));
    hLayout->addWidget(addColorBut);
    hLayout->addStretch(1);

    m_colorLayout->setContentsMargins(0, 0, 0, 0);
    m_colorLayout->setSpacing(3);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(3);

    this->setLayout(hLayout);

    connect(addColorBut, &QPushButton::clicked, this, &ColorSeletorWidget::slotAddColorButClicked);
}

void ColorSeletorWidget::slotButtonClicked(int butId)
{
    qCDebug(colorSelectorLog) << "Color button clicked with ID:" << butId;
    auto it = m_colorEntityMap.find(butId);
    if (m_colorEntityMap.end() == it) {
        qCWarning(colorSelectorLog) << "Invalid button ID:" << butId;
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
    qCDebug(colorSelectorLog) << "Add color button clicked, showing color picker";
    CColorPickerWidget colorPicker;

    if (colorPicker.exec()) {
        DTypeColor::Ptr typeColor;
        typeColor.reset(new DTypeColor());
        typeColor->setColorCode(colorPicker.getSelectedColor().name());
        typeColor->setPrivilege(DTypeColor::PriUser);
        setUserColor(typeColor);
        m_userColorBtn->click();
    }
}

void ColorSeletorWidget::setUserColor(const DTypeColor::Ptr &colorInfo)
{
    qCDebug(colorSelectorLog) << "Setting user color with code:" << colorInfo->colorCode();
    if (nullptr == m_userColorBtn || DTypeColor::PriUser != colorInfo->privilege()) {
        qCWarning(colorSelectorLog) << "Invalid user color button or privilege";
        return;
    }
    if (!m_userColorBtn->isVisible()) {
        m_userColorBtn->show();
    }
    m_userColorBtn->setColor(colorInfo->colorCode());
    m_colorEntityMap[m_userColorBtnId] = colorInfo;
    m_userColorBtn->click();
}
