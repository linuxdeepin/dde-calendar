// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingwidgets.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

using namespace SettingWidget;

// ===== SettingWidgets =====

TEST(SettingWidgetsTest, Constructor)
{
    SettingWidgets *sw = new SettingWidgets();
    ASSERT_NE(nullptr, sw);
    delete sw;
}

// ===== SyncTagRadioButton =====

TEST(SyncTagRadioButtonTest, ConstructorAndDefaultChecked)
{
    SyncTagRadioButton btn;
    EXPECT_TRUE(btn.isChecked());
}

TEST(SyncTagRadioButtonTest, IsChecked)
{
    SyncTagRadioButton btn;
    btn.setChecked(false);
    EXPECT_FALSE(btn.isChecked());
    btn.setChecked(true);
    EXPECT_TRUE(btn.isChecked());
}

TEST(SyncTagRadioButtonTest, SetCheckedSameValueReturnsEarly)
{
    SyncTagRadioButton btn;
    EXPECT_TRUE(btn.isChecked());
    btn.setChecked(true);
    EXPECT_TRUE(btn.isChecked());
}

TEST(SyncTagRadioButtonTest, SetCheckedToggle)
{
    SyncTagRadioButton btn;
    btn.setChecked(false);
    EXPECT_FALSE(btn.isChecked());
    btn.setChecked(true);
    EXPECT_TRUE(btn.isChecked());
}

TEST(SyncTagRadioButtonTest, PaintEventChecked)
{
    SyncTagRadioButton btn;
    btn.resize(32, 32);
    btn.show();
    QCoreApplication::processEvents();
    btn.repaint();
    EXPECT_TRUE(btn.isChecked());
}

TEST(SyncTagRadioButtonTest, PaintEventUnchecked)
{
    SyncTagRadioButton btn;
    btn.setChecked(false);
    btn.resize(32, 32);
    btn.show();
    QCoreApplication::processEvents();
    btn.repaint();
    EXPECT_FALSE(btn.isChecked());
}

TEST(SyncTagRadioButtonTest, MouseReleaseEventTogglesAndEmits)
{
    SyncTagRadioButton btn;
    bool signalReceived = false;
    bool signalValue = false;
    QObject::connect(&btn, &SyncTagRadioButton::clicked,
                     [&] (bool checked) { signalReceived = true; signalValue = checked; });

    EXPECT_TRUE(btn.isChecked());
    QMouseEvent event(QMouseEvent::MouseButtonRelease, QPointF(5, 5),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&btn, &event);

    EXPECT_TRUE(signalReceived);
    EXPECT_FALSE(signalValue);
    EXPECT_FALSE(btn.isChecked());
}

// ===== CalendarSettingSettings =====

TEST(CalendarSettingSettingsTest, IndexOfGroupsFound)
{
    CalendarSettingSettings settings;
    CalendarSettingSetting s;
    s._key = "group1";
    CalendarSettingGroup g;
    g._key = "sub1";
    s._groups.append(g);
    settings.append(s);

    EXPECT_EQ(0, settings.indexOf(settings[0]._groups, "sub1"));
}

TEST(CalendarSettingSettingsTest, IndexOfGroupsNotFound)
{
    CalendarSettingSettings settings;
    CalendarSettingGroups groups;
    EXPECT_EQ(-1, settings.indexOf(groups, "nonexistent"));
}

TEST(CalendarSettingSettingsTest, IndexOfSettingsFound)
{
    CalendarSettingSettings settings;
    CalendarSettingSetting s1;
    s1._key = "group1";
    CalendarSettingSetting s2;
    s2._key = "group2";
    settings.append(s1);
    settings.append(s2);

    EXPECT_EQ(0, settings.indexOf(settings, "group1"));
    EXPECT_EQ(1, settings.indexOf(settings, "group2"));
}

TEST(CalendarSettingSettingsTest, IndexOfSettingsNotFound)
{
    CalendarSettingSettings settings;
    EXPECT_EQ(-1, settings.indexOf(settings, "nonexistent"));
}

TEST(CalendarSettingSettingsTest, RemoveGroupByName)
{
    CalendarSettingSettings settings;
    CalendarSettingSetting s1;
    s1._key = "group1";
    CalendarSettingSetting s2;
    s2._key = "group2";
    settings.append(s1);
    settings.append(s2);

    settings.removeGroup("group1");
    ASSERT_EQ(1, settings.count());
    EXPECT_EQ("group2", settings[0]._key);
}

TEST(CalendarSettingSettingsTest, RemoveGroupByNameNotFound)
{
    CalendarSettingSettings settings;
    CalendarSettingSetting s1;
    s1._key = "group1";
    settings.append(s1);

    settings.removeGroup("nonexistent");
    EXPECT_EQ(1, settings.count());
}

TEST(CalendarSettingSettingsTest, RemoveGroupWithSubgroup)
{
    CalendarSettingSettings settings;
    CalendarSettingSetting s1;
    s1._key = "group1";
    CalendarSettingGroup g1;
    g1._key = "sub1";
    CalendarSettingGroup g2;
    g2._key = "sub2";
    s1._groups.append(g1);
    s1._groups.append(g2);
    settings.append(s1);

    settings.removeGroup("group1", "sub1");
    ASSERT_EQ(1, settings.count());
    ASSERT_EQ(1, settings[0]._groups.count());
    EXPECT_EQ("sub2", settings[0]._groups[0]._key);
}

TEST(CalendarSettingSettingsTest, RemoveGroupWithSubgroupNotFound)
{
    CalendarSettingSettings settings;
    CalendarSettingSetting s1;
    s1._key = "group1";
    CalendarSettingGroup g1;
    g1._key = "sub1";
    s1._groups.append(g1);
    settings.append(s1);

    settings.removeGroup("group1", "nonexistent");
    ASSERT_EQ(1, settings.count());
    EXPECT_EQ(1, settings[0]._groups.count());
}

TEST(CalendarSettingSettingsTest, RemoveGroupWithGroupNotFound)
{
    CalendarSettingSettings settings;
    CalendarSettingSetting s1;
    s1._key = "group1";
    settings.append(s1);

    settings.removeGroup("nonexistent", "sub1");
    EXPECT_EQ(1, settings.count());
}

TEST(CalendarSettingSettingsTest, RemoveGroupEmptiesParent)
{
    CalendarSettingSettings settings;
    CalendarSettingSetting s1;
    s1._key = "group1";
    CalendarSettingGroup g1;
    g1._key = "sub1";
    s1._groups.append(g1);
    settings.append(s1);

    settings.removeGroup("group1", "sub1");
    EXPECT_EQ(0, settings.count());
}
