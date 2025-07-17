// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "colorseletorwidget.h"
#include "configsettings.h"
#include "units.h"
#include "commondef.h"

#include <QPushButton>

ColorSeletorWidget::ColorSeletorWidget(QWidget *parent) : QWidget(parent)
{
    qCDebug(ClientLogger) << "ColorSeletorWidget constructor initialized";
    init();
}

void ColorSeletorWidget::init()
{
    qCDebug(ClientLogger) << "ColorSeletorWidget::init - Initializing widget";
    initView();
    m_colorGroup = new QButtonGroup(this);
    m_colorGroup->setExclusive(true);
    connect(m_colorGroup, &QButtonGroup::idClicked, this, &ColorSeletorWidget::slotButtonClicked);

    m_colorInfo.reset(new DTypeColor());
    qCDebug(ClientLogger) << "ColorSeletorWidget initialization completed";
}

void ColorSeletorWidget::resetColorButton(const AccountItem::Ptr &account)
{
    qCDebug(ClientLogger) << "ColorSeletorWidget::resetColorButton - Resetting color buttons";
    reset();
    if (nullptr == account) {
        qCDebug(ClientLogger) << "Account is null, returning";
        return;
    }

    DTypeColor::List colorList = account->getColorTypeList();
    qCDebug(ClientLogger) << "Got color list with" << colorList.size() << "colors";
    //添加默认颜色控件
    for (DTypeColor::Ptr &var : colorList) {
        if (DTypeColor::PriSystem == var->privilege()) {
            // qCDebug(ClientLogger) << "Adding system color:" << var->colorCode();
            addColor(var);
        }
    }

    //自定义控件单独添加
    qCDebug(ClientLogger) << "Adding user color button with ID:" << m_userColorBtnId;
    m_colorGroup->addButton(m_userColorBtn, m_userColorBtnId);
    m_colorLayout->addWidget(m_userColorBtn);

    if (m_colorGroup->buttons().size() > 0) {
        qCDebug(ClientLogger) << "Clicking first button";
        m_colorGroup->buttons().at(0)->click();
    }
}

void ColorSeletorWidget::reset()
{
    qCDebug(ClientLogger) << "ColorSeletorWidget::reset - Clearing color entities and widgets";
    //清空所有的色彩实体和控件
    m_colorEntityMap.clear();
    QList<QAbstractButton *> buttons = m_colorGroup->buttons();
    for (QAbstractButton *btn : buttons) {
        m_colorGroup->removeButton(btn);
        m_colorLayout->removeWidget(btn);
        //自定义控件内存不释放
        if (btn == m_userColorBtn) {
            // qCDebug(ClientLogger) << "Skipping user color button";
            continue;
        }
        delete btn;
        btn = nullptr;
    }
    if (nullptr == m_userColorBtn) {
        qCDebug(ClientLogger) << "Creating new user color button";
        m_userColorBtn = new CRadioButton(this);
        m_userColorBtn->setFixedSize(18, 18);
    }
    m_userColorBtn->hide();
    qCDebug(ClientLogger) << "Reset completed";
}

void ColorSeletorWidget::addColor(const DTypeColor::Ptr &cInfo)
{
    static int count = 0;   //静态变量，充当色彩控件id
    count++;
    qCDebug(ClientLogger) << "ColorSeletorWidget::addColor - Adding color with ID:" << count << "Color:" << cInfo->colorCode();
    m_colorEntityMap.insert(count, cInfo); //映射id与控件,从1开始
    CRadioButton *radio = new CRadioButton(this);
    radio->setColor(QColor(cInfo->colorCode())); //设置控件颜色
    radio->setFixedSize(18, 18);
    m_colorGroup->addButton(radio, count);
    m_colorLayout->addWidget(radio);
}

DTypeColor::Ptr ColorSeletorWidget::getSelectedColorInfo()
{
    qCDebug(ClientLogger) << "ColorSeletorWidget::getSelectedColorInfo - Getting selected color info";
    if (m_colorInfo->privilege() == DTypeColor::PriSystem) {
        qCDebug(ClientLogger) << "System color selected, saving color ID:" << m_colorInfo->colorID();
        CConfigSettings::getInstance()->setOption("LastSysColorTypeNo", m_colorInfo->colorID());
    } else if (!m_colorInfo->colorCode().isEmpty()) {
        qCDebug(ClientLogger) << "User color selected, saving color code:" << m_colorInfo->colorCode();
        CConfigSettings::getInstance()->setOption("LastUserColor", m_colorInfo->colorCode());
        CConfigSettings::getInstance()->setOption("LastSysColorTypeNo", "");
    }
    return m_colorInfo;
}

void ColorSeletorWidget::setSelectedColorByIndex(int index)
{
    qCDebug(ClientLogger) << "ColorSeletorWidget::setSelectedColorByIndex - Setting color by index:" << index;
    if (index >= 0 && index < m_colorGroup->buttons().size()) {
        QAbstractButton *but = m_colorGroup->buttons().at(index);
        if (nullptr != but) {
            qCDebug(ClientLogger) << "Clicking button at index:" << index;
            but->click();
        }
    } else {
        qCDebug(ClientLogger) << "Index out of range";
    }
}

void ColorSeletorWidget::setSelectedColorById(int colorId)
{
    qCDebug(ClientLogger) << "ColorSeletorWidget::setSelectedColorById - Setting color by ID:" << colorId;
    //默认选择第一个
    if (colorId < 0) {
        qCDebug(ClientLogger) << "Invalid color ID, selecting first button";
        if (m_colorGroup->buttons().size() > 0) {
            m_colorGroup->buttons().at(0)->click();
        }
        return;
    } else if (colorId == 9) {
        qCDebug(ClientLogger) << "User color ID detected, clicking user color button";
        m_userColorBtn->click();
        return;
    }

    //系统颜色则向后移一位
    if (colorId == 8) {
        colorId = 0;
    } else {
        ++colorId;
    }
    qCDebug(ClientLogger) << "Adjusted color ID:" << colorId;
    if (m_colorGroup->buttons().size() > colorId) {
        qCDebug(ClientLogger) << "Clicking button at adjusted ID";
        m_colorGroup->buttons().at(colorId)->click();
    } else {
        qCDebug(ClientLogger) << "Adjusted ID out of range";
    }
}

void ColorSeletorWidget::setSelectedColor(const DTypeColor &colorInfo)
{
    qCDebug(ClientLogger) << "ColorSeletorWidget::setSelectedColor - Setting color with ID:" << colorInfo.colorID();
    bool finding = false;
    auto iterator = m_colorEntityMap.begin();
    while (iterator != m_colorEntityMap.end()) {
        if (iterator.value()->colorID() == colorInfo.colorID()) {
            QAbstractButton *btn = m_colorGroup->button(iterator.key());
            if (btn) {
                qCDebug(ClientLogger) << "Found matching color, clicking button with key:" << iterator.key();
                btn->click();
                finding = true;
            }
            break;
        }
        iterator++;
    }
    if (!finding) {
        qCDebug(ClientLogger) << "Color not found, creating user color";
        DTypeColor::Ptr ptr;
        ptr.reset(new DTypeColor(colorInfo));
        setUserColor(ptr);
    }
}

void ColorSeletorWidget::initView()
{
    qCDebug(ClientLogger) << "ColorSeletorWidget::initView - Initializing view";
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
    qCDebug(ClientLogger) << "View initialization completed";
}

void ColorSeletorWidget::slotButtonClicked(int butId)
{
    qCDebug(ClientLogger) << "ColorSeletorWidget::slotButtonClicked - Button clicked with ID:" << butId;
    auto it = m_colorEntityMap.find(butId);
    if (m_colorEntityMap.end() == it) {
        qCDebug(ClientLogger) << "Button ID not found in color map";
        return;
    }
    DTypeColor::Ptr info = it.value();
    if (info->colorCode() != m_colorInfo->colorCode()) {
        qCDebug(ClientLogger) << "Color changed from" << m_colorInfo->colorCode() << "to" << info->colorCode();
        m_colorInfo = info;
        emit signalColorChange(info);
    } else {
        qCDebug(ClientLogger) << "Color unchanged";
    }
}

void ColorSeletorWidget::slotAddColorButClicked()
{
    qCDebug(ClientLogger) << "ColorSeletorWidget::slotAddColorButClicked - Add color button clicked";
    CColorPickerWidget colorPicker;

    if (colorPicker.exec()) {
        qCDebug(ClientLogger) << "Color picker returned color:" << colorPicker.getSelectedColor().name();
        DTypeColor::Ptr typeColor;
        typeColor.reset(new DTypeColor());
        typeColor->setColorCode(colorPicker.getSelectedColor().name());
        typeColor->setPrivilege(DTypeColor::PriUser);
        setUserColor(typeColor);
        m_userColorBtn->click();
    } else {
        qCDebug(ClientLogger) << "Color picker cancelled";
    }
}

void ColorSeletorWidget::setUserColor(const DTypeColor::Ptr &colorInfo)
{
    // qCDebug(ClientLogger) << "ColorSeletorWidget::setUserColor - Setting user color:" << colorInfo->colorCode();
    if (nullptr == m_userColorBtn || DTypeColor::PriUser != colorInfo->privilege()) {
        qCDebug(ClientLogger) << "Invalid user color button or privilege";
        return;
    }
    if (!m_userColorBtn->isVisible()) {
        qCDebug(ClientLogger) << "Showing user color button";
        m_userColorBtn->show();
    }
    m_userColorBtn->setColor(colorInfo->colorCode());
    m_colorEntityMap[m_userColorBtnId] = colorInfo;
    qCDebug(ClientLogger) << "Clicking user color button";
    m_userColorBtn->click();
}
