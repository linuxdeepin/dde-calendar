// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_colorseletorwidget.h"
#include "../dialog_stub.h"
#include "../third-party_stub/stub.h"
#include <QSignalSpy>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QHBoxLayout>
#include <QPushButton>

test_colorseletorwidget::test_colorseletorwidget(QObject *parent) : QObject(parent)
{
}

// Helper: create a DTypeColor::Ptr with given code and privilege
static DTypeColor::Ptr makeColor(const QString &code, DTypeColor::Privilege pri, const QString &id = QString())
{
    DTypeColor::Ptr c = DTypeColor::Ptr::create();
    c->setColorCode(code);
    c->setPrivilege(pri);
    c->setColorID(id);
    return c;
}

// ColorSeletorWidget::ColorSeletorWidget(QWidget *parent) — constructor calls init()
TEST_F(test_colorseletorwidget, Constructor)
{
    EXPECT_NE(mWidget->m_colorGroup, nullptr);
    EXPECT_NE(mWidget->m_colorLayout, nullptr);
    EXPECT_NE(mWidget->m_addColorButton, nullptr);
    EXPECT_NE(mWidget->m_colorInfo, nullptr);
    EXPECT_TRUE(mWidget->m_colorGroup->exclusive());
}

// void ColorSeletorWidget::init() — private, called by constructor
TEST_F(test_colorseletorwidget, init)
{
    mWidget->m_colorInfo.reset();
    mWidget->init();
    EXPECT_NE(mWidget->m_colorGroup, nullptr);
    EXPECT_NE(mWidget->m_colorInfo, nullptr);
    EXPECT_TRUE(mWidget->m_colorGroup->exclusive());
}

// void ColorSeletorWidget::initView() — private, called by init()
TEST_F(test_colorseletorwidget, initView)
{
    mWidget->m_colorLayout = nullptr;
    mWidget->m_addColorButton = nullptr;
    mWidget->initView();
    EXPECT_NE(mWidget->m_colorLayout, nullptr);
    EXPECT_NE(mWidget->m_addColorButton, nullptr);
    EXPECT_EQ(mWidget->m_addColorButton->objectName(), QString("AddColorButton"));
}

// void ColorSeletorWidget::reset()
TEST_F(test_colorseletorwidget, reset)
{
    mWidget->reset();
    EXPECT_TRUE(mWidget->m_colorEntityMap.isEmpty());
    EXPECT_NE(mWidget->m_userColorBtn, nullptr);
    EXPECT_TRUE(mWidget->m_userColorBtn->isHidden());
}

// void ColorSeletorWidget::reset() — second call reuses existing m_userColorBtn
TEST_F(test_colorseletorwidget, reset_SecondCall)
{
    mWidget->reset();
    CRadioButton *firstBtn = mWidget->m_userColorBtn;
    mWidget->reset();
    EXPECT_EQ(mWidget->m_userColorBtn, firstBtn);
    EXPECT_TRUE(mWidget->m_userColorBtn->isHidden());
}

// void ColorSeletorWidget::addColor(const DTypeColor::Ptr &cInfo)
TEST_F(test_colorseletorwidget, addColor)
{
    mWidget->reset();
    int beforeSize = mWidget->m_colorEntityMap.size();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem));
    EXPECT_EQ(mWidget->m_colorEntityMap.size(), beforeSize + 1);
    EXPECT_EQ(mWidget->m_colorGroup->buttons().size(), beforeSize + 1);
}

// DTypeColor::Ptr ColorSeletorWidget::getSelectedColorInfo() — system color
TEST_F(test_colorseletorwidget, getSelectedColorInfo_System)
{
    mWidget->m_colorInfo = makeColor("#FF0000", DTypeColor::PriSystem, "1");
    DTypeColor::Ptr result = mWidget->getSelectedColorInfo();
    EXPECT_EQ(result->privilege(), DTypeColor::PriSystem);
    EXPECT_EQ(result->colorCode(), QString("#FF0000"));
}

// DTypeColor::Ptr ColorSeletorWidget::getSelectedColorInfo() — user color with non-empty code
TEST_F(test_colorseletorwidget, getSelectedColorInfo_User)
{
    mWidget->m_colorInfo = makeColor("#00FF00", DTypeColor::PriUser);
    DTypeColor::Ptr result = mWidget->getSelectedColorInfo();
    EXPECT_EQ(result->privilege(), DTypeColor::PriUser);
    EXPECT_EQ(result->colorCode(), QString("#00FF00"));
}

// DTypeColor::Ptr ColorSeletorWidget::getSelectedColorInfo() — user color with empty code (no save)
TEST_F(test_colorseletorwidget, getSelectedColorInfo_EmptyCode)
{
    mWidget->m_colorInfo = makeColor("", DTypeColor::PriUser);
    DTypeColor::Ptr result = mWidget->getSelectedColorInfo();
    EXPECT_EQ(result->privilege(), DTypeColor::PriUser);
    EXPECT_TRUE(result->colorCode().isEmpty());
}

// void ColorSeletorWidget::setSelectedColorByIndex(int index) — valid index
TEST_F(test_colorseletorwidget, setSelectedColorByIndex_Valid)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem));
    mWidget->addColor(makeColor("#00FF00", DTypeColor::PriSystem));
    mWidget->setSelectedColorByIndex(0);
    EXPECT_TRUE(mWidget->m_colorGroup->buttons().at(0)->isChecked());
    EXPECT_FALSE(mWidget->m_colorGroup->buttons().at(1)->isChecked());
}

// void ColorSeletorWidget::setSelectedColorByIndex(int index) — invalid index
TEST_F(test_colorseletorwidget, setSelectedColorByIndex_Invalid)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem));
    mWidget->setSelectedColorByIndex(-1);
    mWidget->setSelectedColorByIndex(99);
    EXPECT_EQ(mWidget->m_colorGroup->buttons().size(), 1);
    SUCCEED();
}

// void ColorSeletorWidget::setSelectedColorById(int colorId) — local path, colorId < 0 → first button
TEST_F(test_colorseletorwidget, setSelectedColorById_LocalNegative)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem));
    mWidget->addColor(makeColor("#00FF00", DTypeColor::PriSystem));
    mWidget->setSelectedColorById(-1);
    EXPECT_TRUE(mWidget->m_colorGroup->buttons().at(0)->isChecked());
}

// void ColorSeletorWidget::setSelectedColorById(int colorId) — local path, colorId == 9 → user button
TEST_F(test_colorseletorwidget, setSelectedColorById_LocalUserColor)
{
    mWidget->reset();
    mWidget->m_colorGroup->addButton(mWidget->m_userColorBtn, mWidget->m_userColorBtnId);
    mWidget->setSelectedColorById(9);
    EXPECT_TRUE(mWidget->m_userColorBtn->isChecked());
}

// void ColorSeletorWidget::setSelectedColorById(int colorId) — local path, colorId == 8 → index 0
TEST_F(test_colorseletorwidget, setSelectedColorById_LocalColor8)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem));
    mWidget->addColor(makeColor("#00FF00", DTypeColor::PriSystem));
    mWidget->m_colorGroup->addButton(mWidget->m_userColorBtn, mWidget->m_userColorBtnId);
    mWidget->setSelectedColorById(8);
    EXPECT_TRUE(mWidget->m_colorGroup->buttons().at(0)->isChecked());
}

// void ColorSeletorWidget::setSelectedColorById(int colorId) — local path, normal colorId
TEST_F(test_colorseletorwidget, setSelectedColorById_LocalNormal)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem));
    mWidget->addColor(makeColor("#00FF00", DTypeColor::PriSystem));
    mWidget->addColor(makeColor("#0000FF", DTypeColor::PriSystem));
    mWidget->m_colorGroup->addButton(mWidget->m_userColorBtn, mWidget->m_userColorBtnId);
    // colorId=1 → ++colorId=2 → buttons.at(2) (third system button, index 0-based)
    mWidget->setSelectedColorById(1);
    EXPECT_TRUE(mWidget->m_colorGroup->buttons().at(2)->isChecked());
}

// void ColorSeletorWidget::setSelectedColorById(int colorId) — CalDAV path
TEST_F(test_colorseletorwidget, setSelectedColorById_CalDav)
{
    mWidget->reset();
    mWidget->m_allowCustomColor = false;
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem));
    mWidget->addColor(makeColor("#00FF00", DTypeColor::PriSystem));
    mWidget->setSelectedColorById(0);
    EXPECT_TRUE(mWidget->m_colorGroup->buttons().at(0)->isChecked());
}

// void ColorSeletorWidget::setSelectedColorById(int colorId) — CalDAV path, invalid id → first button
TEST_F(test_colorseletorwidget, setSelectedColorById_CalDavInvalid)
{
    mWidget->reset();
    mWidget->m_allowCustomColor = false;
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem));
    mWidget->setSelectedColorById(99);
    EXPECT_TRUE(mWidget->m_colorGroup->buttons().at(0)->isChecked());
}

// void ColorSeletorWidget::setSelectedColor(const DTypeColor &colorInfo) — found by colorCode
TEST_F(test_colorseletorwidget, setSelectedColor_Found)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem, "1"));
    mWidget->addColor(makeColor("#00FF00", DTypeColor::PriSystem, "2"));

    DTypeColor searchColor;
    searchColor.setColorCode("#00FF00");
    mWidget->setSelectedColor(searchColor);
    EXPECT_FALSE(mWidget->m_colorGroup->buttons().at(0)->isChecked());
    EXPECT_TRUE(mWidget->m_colorGroup->buttons().at(1)->isChecked());
}

// void ColorSeletorWidget::setSelectedColor(const DTypeColor &colorInfo) — found by colorID
TEST_F(test_colorseletorwidget, setSelectedColor_FoundById)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem, "1"));
    mWidget->addColor(makeColor("#00FF00", DTypeColor::PriSystem, "2"));

    DTypeColor searchColor;
    searchColor.setColorID("1");
    mWidget->setSelectedColor(searchColor);
    EXPECT_TRUE(mWidget->m_colorGroup->buttons().at(0)->isChecked());
}

// void ColorSeletorWidget::setSelectedColor(const DTypeColor &colorInfo) — not found, creates user color
TEST_F(test_colorseletorwidget, setSelectedColor_NotFound)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem));

    DTypeColor searchColor;
    searchColor.setColorCode("#999999");
    searchColor.setPrivilege(DTypeColor::PriUser);
    mWidget->setSelectedColor(searchColor);
    EXPECT_TRUE(!mWidget->m_userColorBtn->isHidden());
    EXPECT_TRUE(mWidget->m_colorEntityMap.contains(mWidget->m_userColorBtnId));
}

// void ColorSeletorWidget::slotButtonClicked(int butId) — valid id, color changed
TEST_F(test_colorseletorwidget, slotButtonClicked_Valid)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem, "1"));

    QSignalSpy spy(mWidget, &ColorSeletorWidget::signalColorChange);
    // Get the key of the first (and only) color in the map
    int key = mWidget->m_colorEntityMap.firstKey();
    mWidget->slotButtonClicked(key);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(mWidget->m_colorInfo->colorCode(), QString("#FF0000"));
}

// void ColorSeletorWidget::slotButtonClicked(int butId) — invalid id
TEST_F(test_colorseletorwidget, slotButtonClicked_Invalid)
{
    mWidget->reset();
    QSignalSpy spy(mWidget, &ColorSeletorWidget::signalColorChange);
    mWidget->slotButtonClicked(99999);
    EXPECT_EQ(spy.count(), 0);
}

// void ColorSeletorWidget::slotButtonClicked(int butId) — same color, no signal
TEST_F(test_colorseletorwidget, slotButtonClicked_SameColor)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem, "1"));
    int key = mWidget->m_colorEntityMap.firstKey();
    // First click sets m_colorInfo
    mWidget->slotButtonClicked(key);
    // Second click with same color — should not emit signal
    QSignalSpy spy(mWidget, &ColorSeletorWidget::signalColorChange);
    mWidget->slotButtonClicked(key);
    EXPECT_EQ(spy.count(), 0);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
// void ColorSeletorWidget::slotButtonClickedCompat(QAbstractButton *button) — Qt5 compat
TEST_F(test_colorseletorwidget, slotButtonClickedCompat)
{
    mWidget->reset();
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem, "1"));
    int key = mWidget->m_colorEntityMap.firstKey();
    QAbstractButton *btn = mWidget->m_colorGroup->button(key);
    QSignalSpy spy(mWidget, &ColorSeletorWidget::signalColorChange);
    mWidget->slotButtonClickedCompat(btn);
    EXPECT_EQ(spy.count(), 1);
}
#endif

// void ColorSeletorWidget::slotAddColorButClicked() — exec returns true
TEST_F(test_colorseletorwidget, slotAddColorButClicked_Accept)
{
    mWidget->reset();
    calendarDDialogExecReturn = 1;
    Stub stub;
    calendarDDialogExecStub(stub);

    mWidget->slotAddColorButClicked();
    EXPECT_TRUE(!mWidget->m_userColorBtn->isHidden());
    EXPECT_TRUE(mWidget->m_colorEntityMap.contains(mWidget->m_userColorBtnId));
}

// void ColorSeletorWidget::slotAddColorButClicked() — exec returns false
TEST_F(test_colorseletorwidget, slotAddColorButClicked_Reject)
{
    mWidget->reset();
    calendarDDialogExecReturn = 0;
    Stub stub;
    calendarDDialogExecStub(stub);

    mWidget->slotAddColorButClicked();
    EXPECT_TRUE(mWidget->m_userColorBtn->isHidden());
}

// void ColorSeletorWidget::slotAddColorButClicked() — custom color disabled
TEST_F(test_colorseletorwidget, slotAddColorButClicked_Disabled)
{
    mWidget->reset();
    mWidget->m_allowCustomColor = false;
    mWidget->slotAddColorButClicked();
    EXPECT_TRUE(mWidget->m_userColorBtn->isHidden());
}

// void ColorSeletorWidget::setUserColor(const DTypeColor::Ptr &colorInfo) — normal case
TEST_F(test_colorseletorwidget, setUserColor_Normal)
{
    mWidget->reset();
    DTypeColor::Ptr color = makeColor("#AABBCC", DTypeColor::PriUser);
    mWidget->setUserColor(color);
    EXPECT_TRUE(!mWidget->m_userColorBtn->isHidden());
    EXPECT_TRUE(mWidget->m_colorEntityMap.contains(mWidget->m_userColorBtnId));
}

// void ColorSeletorWidget::setUserColor(const DTypeColor::Ptr &colorInfo) — custom color disabled
TEST_F(test_colorseletorwidget, setUserColor_Disallowed)
{
    mWidget->reset();
    mWidget->m_allowCustomColor = false;
    DTypeColor::Ptr color = makeColor("#AABBCC", DTypeColor::PriUser);
    mWidget->setUserColor(color);
    EXPECT_TRUE(mWidget->m_userColorBtn->isHidden());
}

// void ColorSeletorWidget::setUserColor(const DTypeColor::Ptr &colorInfo) — null user button
TEST_F(test_colorseletorwidget, setUserColor_NullButton)
{
    mWidget->m_userColorBtn = nullptr;
    DTypeColor::Ptr color = makeColor("#AABBCC", DTypeColor::PriUser);
    mWidget->setUserColor(color);
    SUCCEED();
}

// void ColorSeletorWidget::setUserColor(const DTypeColor::Ptr &colorInfo) — wrong privilege
TEST_F(test_colorseletorwidget, setUserColor_WrongPrivilege)
{
    mWidget->reset();
    DTypeColor::Ptr color = makeColor("#AABBCC", DTypeColor::PriSystem);
    mWidget->setUserColor(color);
    EXPECT_TRUE(mWidget->m_userColorBtn->isHidden());
}

// void ColorSeletorWidget::resetColorButton(const AccountItem::Ptr &account) — null account
TEST_F(test_colorseletorwidget, resetColorButton_NullAccount)
{
    mWidget->addColor(makeColor("#FF0000", DTypeColor::PriSystem));
    mWidget->resetColorButton(AccountItem::Ptr());
    EXPECT_TRUE(mWidget->m_colorEntityMap.isEmpty());
    EXPECT_NE(mWidget->m_userColorBtn, nullptr);
}
