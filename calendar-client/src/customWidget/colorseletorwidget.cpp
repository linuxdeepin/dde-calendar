// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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
    //默认选中第一个
    initColorButton(0);
}

void ColorSeletorWidget::initColorButton(int index)
{
    reset();
    QList<JobTypeColorInfo> lstColorInfo = JobTypeInfoManager::instance()->getJobTypeColorList();
    for (JobTypeColorInfo &var : lstColorInfo) {
        if (TypeSystem == var.getAuthority()) {
            addColor(var);
        }
    }

    if (index >= 0 && index < m_colorGroup->buttons().size()) {
        m_colorGroup->buttons().at(index)->click();
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
        delete m_userColorBtn;
    }
    m_userColorBtn = nullptr;
    m_userColorBtnId = -1;
}

void ColorSeletorWidget::addColor(const JobTypeColorInfo &cInfo)
{
    static int count = 0;   //静态变量，充当色彩控件id
    count++;
    m_colorEntityMap.insert(count, cInfo);      //映射id与控件
    CRadioButton *radio = new CRadioButton(this);
    radio->setColor(QColor(cInfo.getColorHex()));         //设置控件颜色
    radio->setFixedSize(18, 18);
    m_colorGroup->addButton(radio, count);
    m_colorLayout->addWidget(radio);
    if (TypeUser == cInfo.getAuthority()) {
        m_userColorBtn = radio;             //记录用户色彩指针
        m_userColorBtnId = count;
    }
}

JobTypeColorInfo ColorSeletorWidget::getSelectedColorInfo()
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
        if (iterator.value().getTypeNo() == colorId) {
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

void ColorSeletorWidget::setSelectedColor(const JobTypeColorInfo &colorInfo)
{
    if (TypeUser == colorInfo.getAuthority()) {
        setUserColor(colorInfo);
    }

    bool isFind = false;
    //遍历所有控件
    for (QAbstractButton *but : m_colorGroup->buttons()) {
        if (nullptr != but && qobject_cast<CRadioButton *>(but)->getColor().name() == colorInfo.getColorHex()) {
            but->click();
            isFind = true;
            break;
        }
    }
    if (!isFind) {
        setUserColor(JobTypeColorInfo(0, colorInfo.getColorHex(), TypeUser));
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
    JobTypeColorInfo info = it.value();
    if (info.getColorHex() != m_colorInfo.getColorHex()) {
        m_colorInfo = info;
        emit signalColorChange(info);
    }
}

void ColorSeletorWidget::slotAddColorButClicked()
{
    CColorPickerWidget colorPicker;

    if (colorPicker.exec()) {
        //设置用户自定义控件颜色
        setUserColor(JobTypeColorInfo(0, colorPicker.getSelectedColor().name(), TypeUser));
        m_userColorBtn->click();
    }
}

void ColorSeletorWidget::setUserColor(const JobTypeColorInfo &colorInfo)
{
    if (TypeUser != colorInfo.getAuthority()) {
        return;
    }
    if (nullptr == m_userColorBtn) {
        addColor(colorInfo);
    }
    m_userColorBtn->setColor(colorInfo.getColorHex());
    m_colorEntityMap[m_userColorBtnId] = colorInfo;
    m_userColorBtn->click();
}
