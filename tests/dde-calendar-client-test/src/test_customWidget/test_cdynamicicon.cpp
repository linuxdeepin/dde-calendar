// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cdynamicicon.h"
#include "../dialog_stub.h"

#include <DApplication>
#include <DAboutDialog>

test_cdynamicicon::test_cdynamicicon()
{
    mDynamicicon = new CDynamicIcon(8, 8);
}

test_cdynamicicon::~test_cdynamicicon()
{
    delete mDynamicicon;
    mDynamicicon = nullptr;
}

// Stubs for DApplication methods used by setIcon()
void stub_setProductIcon(void *obj, const QIcon &icon)
{
    Q_UNUSED(obj)
    Q_UNUSED(icon)
}

DAboutDialog *stub_aboutDialog(void *obj)
{
    Q_UNUSED(obj)
    return nullptr;
}

// CDynamicIcon::CDynamicIcon(int width, int height)
TEST_F(test_cdynamicicon, Constructor)
{
    CDynamicIcon *icon = new CDynamicIcon(16, 16);
    EXPECT_NE(icon->m_pixmap, nullptr);
    EXPECT_EQ(icon->m_pixmap->width(), 16);
    EXPECT_EQ(icon->m_pixmap->height(), 16);
    EXPECT_NE(icon->m_Dayrenderer, nullptr);
    EXPECT_NE(icon->m_Weekrenderer, nullptr);
    EXPECT_NE(icon->m_Monthrenderer, nullptr);
    EXPECT_NE(icon->m_backgroundrenderer, nullptr);
    delete icon;
}

// CDynamicIcon::~CDynamicIcon()
TEST_F(test_cdynamicicon, Destructor)
{
    CDynamicIcon *icon = new CDynamicIcon(8, 8);
    delete icon;
    SUCCEED();
}

// CDynamicIcon *CDynamicIcon::getInstance()
TEST_F(test_cdynamicicon, getInstance)
{
    CDynamicIcon::releaseInstance();
    CDynamicIcon *icon1 = CDynamicIcon::getInstance();
    EXPECT_NE(icon1, nullptr);
    CDynamicIcon *icon2 = CDynamicIcon::getInstance();
    EXPECT_EQ(icon1, icon2);
    CDynamicIcon::releaseInstance();
}

// void CDynamicIcon::releaseInstance()
TEST_F(test_cdynamicicon, releaseInstance)
{
    CDynamicIcon::releaseInstance();
    CDynamicIcon *icon = CDynamicIcon::getInstance();
    EXPECT_NE(icon, nullptr);
    CDynamicIcon::releaseInstance();
    CDynamicIcon *icon2 = CDynamicIcon::getInstance();
    EXPECT_NE(icon2, nullptr);
    EXPECT_NE(icon, icon2);
    CDynamicIcon::releaseInstance();
}

// void CDynamicIcon::setDate(const QDate &date)
TEST_F(test_cdynamicicon, setDate)
{
    mDynamicicon->setDate(QDate::currentDate());
    EXPECT_EQ(mDynamicicon->m_Date, QDate::currentDate());
}

// void CDynamicIcon::setIcon()
TEST_F(test_cdynamicicon, setIcon)
{
    Stub stub;

    typedef void (*fptr_setProductIcon)(DApplication*, const QIcon&);
    fptr_setProductIcon f_setProductIcon = (fptr_setProductIcon)(&DApplication::setProductIcon);
    stub.set(f_setProductIcon, stub_setProductIcon);

    typedef DAboutDialog* (*fptr_aboutDialog)(const DApplication*);
    fptr_aboutDialog f_aboutDialog = (fptr_aboutDialog)(&DApplication::aboutDialog);
    stub.set(f_aboutDialog, stub_aboutDialog);

    mDynamicicon->setIcon();
    SUCCEED();
}

// void CDynamicIcon::setTitlebar(DTitlebar *titlebar)
TEST_F(test_cdynamicicon, setTitlebar)
{
    DTitlebar *titlebar = new DTitlebar();
    mDynamicicon->setTitlebar(titlebar);
    EXPECT_EQ(mDynamicicon->m_Titlebar, titlebar);
    delete titlebar;
}

// void CDynamicIcon::paintPixmap(QPixmap *pixmap)
TEST_F(test_cdynamicicon, paintPixmap)
{
    QPixmap *pixmap = new QPixmap(64, 64);
    pixmap->fill(Qt::transparent);
    mDynamicicon->paintPixmap(pixmap);
    EXPECT_FALSE(pixmap->isNull());
    delete pixmap;
}
