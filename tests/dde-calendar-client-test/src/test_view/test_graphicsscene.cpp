// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QDate>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QGraphicsRectItem>

#include "cgraphicsscene.h"
#include "view/graphicsItem/cfocusitem.h"
#include "keypress/ckeypressprxy.h"

class test_graphicsscene : public ::testing::Test
{
protected:
    CGraphicsScene *scene = nullptr;
    void SetUp() override
    {
        scene = new CGraphicsScene();
    }
    void TearDown() override
    {
        delete scene;
        scene = nullptr;
    }
};

TEST_F(test_graphicsscene, Constructor)
{
    EXPECT_NE(scene, nullptr);
}

TEST_F(test_graphicsscene, setFirstFocusItem_getFirstFocusItem)
{
    QGraphicsRectItem *item = new QGraphicsRectItem();
    scene->setFirstFocusItem(item);
    EXPECT_EQ(scene->getFirstFocusItem(), item);
    scene->removeItem(item);
    delete item;
}

TEST_F(test_graphicsscene, setCurrentFocusItem_getCurrentFocusItem)
{
    QGraphicsRectItem *item = new QGraphicsRectItem();
    scene->setCurrentFocusItem(item);
    EXPECT_EQ(scene->getCurrentFocusItem(), item);
    scene->removeItem(item);
    delete item;
}

TEST_F(test_graphicsscene, setKeyPressPrxy)
{
    CKeyPressPrxy *prxy = new CKeyPressPrxy();
    scene->setKeyPressPrxy(prxy);
    scene->setKeyPressPrxy(nullptr);
    delete prxy;
}

TEST_F(test_graphicsscene, currentFocusItemUpdate_nullptr)
{
    scene->currentFocusItemUpdate();
}

TEST_F(test_graphicsscene, currentFocusItemUpdate_withItem)
{
    CFocusItem *item = new CFocusItem();
    scene->setCurrentFocusItem(item);
    scene->addItem(item);
    scene->currentFocusItemUpdate();
    scene->clear();
}

TEST_F(test_graphicsscene, setPrePage)
{
    scene->setPrePage(QDate::currentDate(), false);
    scene->setPrePage(QDate::currentDate(), true);
}

TEST_F(test_graphicsscene, setNextPage)
{
    scene->setNextPage(QDate::currentDate(), false);
    scene->setNextPage(QDate::currentDate(), true);
}

TEST_F(test_graphicsscene, getActiveSwitching_setActiveSwitching)
{
    scene->setActiveSwitching(true);
    EXPECT_TRUE(scene->getActiveSwitching());
    scene->setActiveSwitching(false);
    EXPECT_FALSE(scene->getActiveSwitching());
}

TEST_F(test_graphicsscene, currentItemInit_nullptr)
{
    scene->currentItemInit();
}

TEST_F(test_graphicsscene, setIsShowCurrentItem)
{
    scene->setIsShowCurrentItem(true);
    scene->setIsShowCurrentItem(false);
}

TEST_F(test_graphicsscene, setIsContextMenu)
{
    scene->setIsContextMenu(true);
    scene->setIsContextMenu(false);
}

TEST_F(test_graphicsscene, event_FocusIn)
{
    QFocusEvent event(QEvent::FocusIn, Qt::TabFocusReason);
    scene->event(&event);
}

TEST_F(test_graphicsscene, event_FocusOut)
{
    QFocusEvent event(QEvent::FocusOut, Qt::TabFocusReason);
    scene->event(&event);
}

TEST_F(test_graphicsscene, event_KeyPress)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    scene->event(&event);
}

TEST_F(test_graphicsscene, event_Other)
{
    QEvent event(QEvent::None);
    scene->event(&event);
}

TEST_F(test_graphicsscene, focusInDeal_withItem)
{
    CFocusItem *item = new CFocusItem();
    scene->setCurrentFocusItem(item);
    scene->setFirstFocusItem(item);
    scene->addItem(item);
    scene->setIsShowCurrentItem(true);
    QFocusEvent event(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
    scene->event(&event);
    scene->clear();
}

TEST_F(test_graphicsscene, focusOutDeal_ActiveSwitching)
{
    scene->setActiveSwitching(true);
    QFocusEvent event(QEvent::FocusOut, Qt::ActiveWindowFocusReason);
    scene->event(&event);
}

TEST_F(test_graphicsscene, focusOutDeal_ContextMenu)
{
    scene->setIsContextMenu(true);
    QFocusEvent event(QEvent::FocusOut, Qt::OtherFocusReason);
    scene->event(&event);
}
