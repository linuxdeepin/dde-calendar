// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "userloginwidget.h"
#include "accountmanager.h"
#include "doanetworkdbus.h"
#include "commondef.h"
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <QHBoxLayout>
#include <QPainterPath>
#include <QNetworkReply>

UserloginWidget::UserloginWidget(QWidget *parent)
    : QWidget(parent)
    , m_loginStatus(false)
{
    qCDebug(ClientLogger) << "UserloginWidget constructed";
    initView();
    initConnect();
    slotAccountUpdate();
}

UserloginWidget::~UserloginWidget()
{
    qCDebug(ClientLogger) << "UserloginWidget destroyed";
}

void UserloginWidget::initView()
{
    qCDebug(ClientLogger) << "Initializing UserloginWidget view";
    m_userNameLabel = new DLabel();
    m_userNameLabel->setElideMode(Qt::ElideMiddle);
    m_userNameLabel->setTextFormat(Qt::PlainText);
    m_buttonImg = new DIconButton(this);
    m_buttonLogin = new QPushButton(this);
    m_buttonLoginOut = new QPushButton(this);
    m_buttonLogin->setFixedSize(98, 36);
    m_buttonLoginOut->setFixedSize(98, 36);
    QHBoxLayout *layout = new QHBoxLayout(this);
    const QIcon &icon = QIcon::fromTheme("dde_calendar_account");
    m_buttonImg->setIcon(icon);
    m_buttonImg->setIconSize(QSize(36, 36));
    m_buttonImg->setStyleSheet("border:0px solid;");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_buttonImg);
    layout->addSpacing(5);
    layout->addWidget(m_userNameLabel);
    layout->addStretch();
    m_buttonLogin->setText(tr("Sign In", "button"));
    m_buttonLoginOut->setText(tr("Sign Out", "button"));
    m_buttonLoginOut->hide();
    layout->addWidget(m_buttonLogin);
    layout->addWidget(m_buttonLoginOut);
    this->layout()->setAlignment(Qt::AlignLeft);
    m_ptrDoaNetwork = new DOANetWorkDBus(this);
    if (m_ptrDoaNetwork->getNetWorkState() == DOANetWorkDBus::NetWorkState::Active) {
        qCDebug(ClientLogger) << "Network is active, enabling login/logout buttons";
        m_buttonLoginOut->setEnabled(true);
        m_buttonLogin->setEnabled(true);
    } else {
        qCDebug(ClientLogger) << "Network is not active, disabling login/logout buttons";
        m_buttonLogin->setEnabled(false);
        m_buttonLoginOut->setEnabled(false);
    }

    m_networkManager = new QNetworkAccessManager(this);
}

void UserloginWidget::initConnect()
{
    qCDebug(ClientLogger) << "Initializing UserloginWidget connections";
    connect(m_buttonLogin, &QPushButton::clicked, this, &UserloginWidget::slotLoginBtnClicked);
    connect(m_buttonLoginOut, &QPushButton::clicked, this, &UserloginWidget::slotLogoutBtnClicked);
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, &UserloginWidget::slotAccountUpdate);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &UserloginWidget::slotReplyPixmapLoad);
    connect(m_ptrDoaNetwork, &DOANetWorkDBus::sign_NetWorkChange, this, &UserloginWidget::slotNetworkStateChange);
}

QPixmap UserloginWidget::pixmapToRound(const QPixmap &src, int radius)
{
    qCDebug(ClientLogger) << "Converting pixmap to round with radius:" << radius;
    QSize size(2 * radius, 2 * radius);
    QPixmap pic(size);
    pic.fill(Qt::transparent);

    QPainterPath painterPath;
    painterPath.addEllipse(QRect(0, 0, pic.width(), pic.height()));

    QPainter painter(&pic);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipPath(painterPath);

    painter.drawPixmap(0, 0, src.scaled(size));
    pic.setDevicePixelRatio(devicePixelRatioF());

    return pic;
}

void UserloginWidget::slotNetworkStateChange(DOANetWorkDBus::NetWorkState state)
{
    qCDebug(ClientLogger) << "Network state changed to:" << static_cast<int>(state);
    if (DOANetWorkDBus::NetWorkState::Disconnect == state)  {
        qCDebug(ClientLogger) << "Network disconnected, disabling login/logout buttons";
        m_buttonLogin->setEnabled(false);
        m_buttonLoginOut->setEnabled(false);
    } else if (DOANetWorkDBus::NetWorkState::Active == state) {
        qCDebug(ClientLogger) << "Network active, enabling login/logout buttons";
        m_buttonLoginOut->setEnabled(true);
        m_buttonLogin->setEnabled(true);
    }
}

void UserloginWidget::slotLoginBtnClicked()
{
    qCDebug(ClientLogger) << "Login button clicked, initiating login";
    gAccountManager->login();
}

void UserloginWidget::slotLogoutBtnClicked()
{
    qCDebug(ClientLogger) << "Logout button clicked, initiating logout";
    gAccountManager->loginout();
}

void UserloginWidget::slotAccountUpdate()
{
    qCDebug(ClientLogger) << "Account update received";
    if (gUosAccountItem) {
        //账户为登录状态
        qCDebug(ClientLogger) << "Account is logged in";
        m_buttonLogin->hide();
        m_buttonLoginOut->show();
        DAccount::Ptr account = gUosAccountItem->getAccount();
        m_userNameLabel->setText(account->accountName());
        m_userNameLabel->setToolTip("<p style='white-space:pre;'>" + account->accountName().toHtmlEscaped());
        // 这里的url一定要带上http://头的， 跟在浏览器里输入其它链接不太一样，浏览器里面会自动转的，这里需要手动加上。
        m_networkManager->get(QNetworkRequest(account->avatar()));
    } else {
        //账户为未登录状态
        qCDebug(ClientLogger) << "Account is logged out";
        m_buttonLoginOut->hide();
        m_buttonLogin->show();
        m_userNameLabel->setText("");
        m_userNameLabel->setToolTip("");
        m_buttonImg->setIcon(QIcon::fromTheme("dde_calendar_account"));
    }
}

void UserloginWidget::slotReplyPixmapLoad(QNetworkReply *reply)
{
    qCDebug(ClientLogger) << "Network reply received for avatar";
    QPixmap pixmap;
    //因自定义头像路径拿到的不是真实路径，需要从请求头中拿取到真实路径再次发起请求
    QUrl url = reply->header(QNetworkRequest::LocationHeader).toUrl();
    if (url.url().isEmpty()) {
        qCDebug(ClientLogger) << "Using direct avatar data from reply";
        pixmap.loadFromData(reply->readAll());
    } else {
        qCDebug(ClientLogger) << "Found redirect URL for avatar:" << url.toString();
        m_networkManager->get(QNetworkRequest(url.url()));
    }

    if (!pixmap.isNull()) {
        qCDebug(ClientLogger) << "Avatar loaded successfully, updating UI";
        m_buttonImg->setIcon(pixmapToRound(pixmap, 32));
    }
}

QPair<QWidget *, QWidget *> UserloginWidget::createloginButton(QObject *obj)
{
    qCDebug(ClientLogger) << "Creating login button for settings";
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, new UserloginWidget());
    return optionWidget;
}
