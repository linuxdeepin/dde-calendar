// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <QTest>

// 无头（offscreen/minimal）平台下弹窗/焦点/动画/自绘类用例会崩溃或卡死，跳过；
// 真实图形会话（如 CI 桌面环境）下正常执行。
static bool headlessQpaPlatform()
{
    const QByteArray platform = qEnvironmentVariable("QT_QPA_PLATFORM").toUtf8();
    return platform == "offscreen" || platform == "minimal";
}

#include "test_dayhuangliview.h"

test_dayhuangliview::test_dayhuangliview()
{
    mDayHuangLiLabel = new CDayHuangLiLabel();
}

test_dayhuangliview::~test_dayhuangliview()
{
    delete mDayHuangLiLabel;
    mDayHuangLiLabel = nullptr;
}

//void CDayHuangLiLabel::CDayHuangLiLabel(QWidget *parent)
TEST_F(test_dayhuangliview, constructor)
{
    CDayHuangLiLabel *label = new CDayHuangLiLabel();
    ASSERT_NE(label, nullptr);
    delete label;
}

//void CDayHuangLiLabel::setbackgroundColor(QColor backgroundColor)
TEST_F(test_dayhuangliview, setbackgroundColor)
{
    mDayHuangLiLabel->setbackgroundColor(QColor(100, 100, 255));
}

//void CDayHuangLiLabel::setTextInfo(QColor tcolor, QFont font)
TEST_F(test_dayhuangliview, setTextInfo)
{
    QFont font;
    font.setPixelSize(14);
    mDayHuangLiLabel->setTextInfo(QColor(100, 100, 255), font);
}

//void CDayHuangLiLabel::setHuangLiText(QStringList vhuangli, int type)
TEST_F(test_dayhuangliview, setHuangLiText_nonEmpty)
{
    QStringList strlist;
    strlist << "嫁娶" << "入土";
    mDayHuangLiLabel->setHuangLiText(strlist, 0);
    mDayHuangLiLabel->setHuangLiText(strlist, 1);
}

TEST_F(test_dayhuangliview, setHuangLiText_empty)
{
    QStringList strlist;
    mDayHuangLiLabel->setHuangLiText(strlist, 0);
    mDayHuangLiLabel->setHuangLiText(strlist, 1);
}

//void CDayHuangLiLabel::paintEvent(QPaintEvent *e)
TEST_F(test_dayhuangliview, paintEvent)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (paint/interaction)";
    }
    mDayHuangLiLabel->setFixedSize(200, 50);
    mDayHuangLiLabel->setbackgroundColor(QColor(100, 100, 255));
    mDayHuangLiLabel->setTextInfo(QColor(0, 0, 0), QFont());
    QStringList strlist;
    strlist << "嫁娶" << "入土";
    mDayHuangLiLabel->setHuangLiText(strlist, 0);
    mDayHuangLiLabel->show();
    QTest::qWait(50);
    mDayHuangLiLabel->hide();
}

//void CDayHuangLiLabel::resizeEvent(QResizeEvent *event)
TEST_F(test_dayhuangliview, resizeEvent)
{
    mDayHuangLiLabel->setFixedSize(200, 50);
    mDayHuangLiLabel->setFixedSize(300, 60);
}
