// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "userloginwidget.h"
#include "accountmanager.h"
#include "doanetworkdbus.h"

#include "../third-party_stub/stub.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QBuffer>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QTest>
#include <DSettingsOption>
#include <DGuiApplicationHelper>

// ===== Stub replacement functions =====

static void login_stub() {}
static void loginout_stub() {}
static DOANetWorkDBus::NetWorkState getNetWorkState_stub()
{
    return DOANetWorkDBus::Disconnect;
}

// ===== Mock QNetworkReply for slotReplyPixmapLoad =====

class FakeNetworkReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit FakeNetworkReply(QObject *parent = nullptr)
        : QNetworkReply(parent)
    {
        setOpenMode(QIODevice::ReadOnly);
    }

    void setRedirectUrl(const QUrl &url)
    {
        setHeader(QNetworkRequest::LocationHeader, url);
    }

    void setRawData(const QByteArray &data)
    {
        m_data = data;
        m_buffer.setData(m_data);
        m_buffer.open(QIODevice::ReadOnly);
    }

    void abort() override { close(); }

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        return m_buffer.read(data, maxSize);
    }

private:
    QByteArray m_data;
    QBuffer m_buffer;
};

// ===== UserloginWidget tests =====

class UserloginWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.set(ADDR(AccountManager, login), login_stub);
        stub.set(ADDR(AccountManager, loginout), loginout_stub);
        stub.set(ADDR(DOANetWorkDBus, getNetWorkState), getNetWorkState_stub);

        widget = new UserloginWidget();
    }

    void TearDown() override
    {
        delete widget;
    }

    Stub stub;
    UserloginWidget *widget = nullptr;
};

TEST_F(UserloginWidgetTest, Constructor)
{
    ASSERT_NE(nullptr, widget);
}

TEST_F(UserloginWidgetTest, Destructor)
{
    UserloginWidget *w = new UserloginWidget();
    delete w;
    SUCCEED();
}

TEST_F(UserloginWidgetTest, SlotNetworkStateChangeDisconnect)
{
    widget->slotNetworkStateChange(DOANetWorkDBus::Disconnect);
    QPushButton *loginBtn = widget->findChild<QPushButton *>("ButtonLogin");
    ASSERT_NE(nullptr, loginBtn);
    EXPECT_FALSE(loginBtn->isEnabled());
}

TEST_F(UserloginWidgetTest, SlotNetworkStateChangeActive)
{
    widget->slotNetworkStateChange(DOANetWorkDBus::Active);
    QPushButton *loginBtn = widget->findChild<QPushButton *>("ButtonLogin");
    ASSERT_NE(nullptr, loginBtn);
    EXPECT_TRUE(loginBtn->isEnabled());
}

TEST_F(UserloginWidgetTest, SlotLoginBtnClicked)
{
    widget->slotLoginBtnClicked();
    SUCCEED();
}

TEST_F(UserloginWidgetTest, SlotLogoutBtnClicked)
{
    widget->slotLogoutBtnClicked();
    SUCCEED();
}

TEST_F(UserloginWidgetTest, SlotAccountUpdateLoggedOut)
{
    widget->slotAccountUpdate();
    QPushButton *loginBtn = widget->findChild<QPushButton *>("ButtonLogin");
    ASSERT_NE(nullptr, loginBtn);
    EXPECT_FALSE(loginBtn->isHidden());
}

TEST_F(UserloginWidgetTest, SlotReplyPixmapLoadDirectData)
{
    // Create a small valid pixmap and convert to PNG data
    QPixmap src(4, 4);
    src.fill(Qt::red);
    QByteArray pngData;
    QBuffer buf(&pngData);
    buf.open(QIODevice::WriteOnly);
    src.save(&buf, "PNG");
    buf.close();

    FakeNetworkReply *reply = new FakeNetworkReply();
    reply->setRawData(pngData);

    widget->slotReplyPixmapLoad(reply);
    reply->deleteLater();
    SUCCEED();
}

TEST_F(UserloginWidgetTest, SlotReplyPixmapLoadRedirect)
{
    FakeNetworkReply *reply = new FakeNetworkReply();
    reply->setRedirectUrl(QUrl("file:///dev/null"));

    widget->slotReplyPixmapLoad(reply);
    reply->deleteLater();
    SUCCEED();
}

TEST_F(UserloginWidgetTest, CreateloginButton)
{
    DTK_CORE_NAMESPACE::DSettingsOption option;
    QPair<QWidget *, QWidget *> result = UserloginWidget::createloginButton(&option);
    SUCCEED();
}

TEST_F(UserloginWidgetTest, PixmapToRound)
{
    QPixmap src(64, 64);
    src.fill(Qt::blue);

    QPixmap result = widget->pixmapToRound(src, 32);
    EXPECT_EQ(64, result.width());
    EXPECT_EQ(64, result.height());
    EXPECT_FALSE(result.isNull());
}

TEST_F(UserloginWidgetTest, UpdateLoggedOutStyle)
{
    widget->updateLoggedOutStyle();
    DLabel *label = widget->findChild<DLabel *>();
    ASSERT_NE(nullptr, label);
    SUCCEED();
}

TEST_F(UserloginWidgetTest, SignOutButtonPaintEvent)
{
    // SignOutButton is in anonymous namespace — test indirectly via child widget.
    // Show the parent as a top-level window so the child actually renders in the
    // headless test environment (child show() alone won't trigger a paint unless
    // the enclosing window is exposed by the windowing system).
    widget->resize(200, 100);
    widget->show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(widget));
    QWidget *signOutBtn = widget->findChild<QWidget *>("UosSignOutButton");
    ASSERT_NE(nullptr, signOutBtn);
    // The logout button is hidden by default (logged-out state); make it
    // visible so repaint() actually dispatches the paint event.
    signOutBtn->show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(widget));
    signOutBtn->repaint();
    QTest::qWait(50);
    widget->hide();
    SUCCEED();
}

TEST_F(UserloginWidgetTest, ThemeTypeChangedLoggedOut)
{
    // The themeTypeChanged signal triggers a lambda (connected in initConnect)
    // that calls updateLoggedOutStyle() when logged out (gUosAccountItem is null).
    // DTK6 removed setThemeType, so emit the signal directly (allowed via
    // -fno-access-control which lifts the protected access on signals).
    auto *helper = Dtk::Gui::DGuiApplicationHelper::instance();
    auto current = helper->themeType();
    auto next = (current == Dtk::Gui::DGuiApplicationHelper::LightType)
        ? Dtk::Gui::DGuiApplicationHelper::DarkType
        : Dtk::Gui::DGuiApplicationHelper::LightType;
    emit helper->themeTypeChanged(next);
    QTest::qWait(50);
    emit helper->themeTypeChanged(current);
    QTest::qWait(50);
    SUCCEED();
}

#include "test_userloginwidget.moc"
