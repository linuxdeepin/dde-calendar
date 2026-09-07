// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "caldavaccountlistwidget.h"

#include "accountitem.h"
#include "accountmanager.h"
#include "dcaldavaccountstatus.h"
#include "dcaldavprofile.h"
#include "caldavprovidericon.h"

#include <QCoreApplication>
#include <QIcon>
#include <DLabel>
#include <QHBoxLayout>
#include <QPalette>
#include <QTimer>
#include <QVBoxLayout>

#include <DDialog>
#include <DToolButton>

#include <DBackgroundGroup>
#include <DPushButton>




namespace {


QString accountTitle(const DAccount::Ptr &account, const DCalDavAccountStatus &status)
{
    QString title = account->displayName();
    title = DCalDavProviderProfile::accountDisplayName(
        static_cast<DCalDavProviderProfile::ProviderType>(status.providerType), title);
    return title;
}

QString syncTimeText(const DCalDavAccountStatus &status)
{
    if (!status.lastSuccessfulSync.isValid()) {
        return QCoreApplication::translate("CalDavAccountListWidget", "Last sync time");
    }

    const QString time = status.lastSuccessfulSync.toLocalTime().toString(
        QStringLiteral("yyyy/MM/dd HH:mm"));
    return QCoreApplication::translate("CalDavAccountListWidget", "Last sync time (%1)").arg(time);
}

QIcon syncStatusIcon(bool failed)
{
    return QIcon(failed
        ? QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_warning_light_32px.svg")
        : QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_spinner_32px.svg"));
}


} // namespace

CalDavAccountListWidget::CalDavAccountListWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QWidget *cards = new QWidget(this);
    m_cardsLayout = new QVBoxLayout(cards);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(10);
    layout->addWidget(cards);

    connect(gAccountManager, &AccountManager::signalAccountUpdate,
            this, &CalDavAccountListWidget::slotAccountUpdate);
    connect(gAccountManager, &AccountManager::signalCalDavAccountStatusChanged,
            this, &CalDavAccountListWidget::slotStatusChanged);
    connect(gAccountManager, &AccountManager::signalCalDavAccountStatusReady,
            this, &CalDavAccountListWidget::rebuildCards);
    rebuildCards();
    for (const AccountItem::Ptr &item : gAccountManager->getAccountList()) {
        if (!item.isNull() && item->getAccount()->accountType() == DAccount::Account_CalDav) {
            showConflictNotice(item->getAccount()->accountID());
        }
    }
}

void CalDavAccountListWidget::rebuildCards()
{
    while (QLayoutItem *item = m_cardsLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    QList<AccountItem::Ptr> calDavAccounts;
    for (const AccountItem::Ptr &item : gAccountManager->getAccountList()) {
        if (!item.isNull() && item->getAccount()->accountType() == DAccount::Account_CalDav) {
            calDavAccounts.append(item);
        }
    }

    if (calDavAccounts.isEmpty()) {
        QVBoxLayout *emptyLayout = new QVBoxLayout;
        Dtk::Widget::DBackgroundGroup *emptyGroup = new Dtk::Widget::DBackgroundGroup(
            emptyLayout, m_cardsLayout->parentWidget());
        emptyGroup->setItemSpacing(0);
        emptyGroup->setItemMargins(QMargins(0, 0, 0, 0));
        emptyGroup->setBackgroundRole(QPalette::Window);
        emptyGroup->setUseWidgetBackground(false);

        QWidget *emptyRow = new QWidget(emptyGroup);
        emptyRow->setFixedHeight(48);
        QHBoxLayout *emptyRowLayout = new QHBoxLayout(emptyRow);
        emptyRowLayout->setContentsMargins(10, 0, 10, 0);
        emptyRowLayout->setSpacing(6);

        Dtk::Widget::DLabel *iconLabel = new Dtk::Widget::DLabel(emptyRow);
        iconLabel->setFixedSize(36, 36);
        iconLabel->setPixmap(QIcon(QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_third_party_36px.svg"))
                                 .pixmap(36, 36));
        emptyRowLayout->addWidget(iconLabel);

        Dtk::Widget::DLabel *emptyLabel = new Dtk::Widget::DLabel(tr("No third-party accounts added"), emptyRow);
        emptyLabel->setTextFormat(Qt::PlainText);
        emptyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        emptyRowLayout->addWidget(emptyLabel);
        emptyRowLayout->addStretch();

        Dtk::Widget::DPushButton *addButton = new Dtk::Widget::DPushButton(tr("Add"), emptyRow);
        addButton->setFixedSize(98, 36);
        connect(addButton, &Dtk::Widget::DPushButton::clicked,
                this, &CalDavAccountListWidget::addAccountRequested);
        emptyRowLayout->addWidget(addButton);

        emptyLayout->addWidget(emptyRow);
        m_cardsLayout->addWidget(emptyGroup);
        return;
    }

    for (const AccountItem::Ptr &item : calDavAccounts) {
        const DAccount::Ptr account = item->getAccount();
        const DCalDavAccountStatus status = gAccountManager->getCalDavAccountStatus(
            account->accountID());
        const QString accountID = account->accountID();

        QVBoxLayout *accountLayout = new QVBoxLayout;
        Dtk::Widget::DBackgroundGroup *accountGroup = new Dtk::Widget::DBackgroundGroup(
            accountLayout, m_cardsLayout->parentWidget());
        accountGroup->setItemSpacing(1);
        accountGroup->setItemMargins(QMargins(0, 0, 0, 0));
        accountGroup->setBackgroundRole(QPalette::Window);
        accountGroup->setUseWidgetBackground(false);

        QWidget *accountRow = new QWidget(accountGroup);
        accountRow->setFixedHeight(36);
        QHBoxLayout *accountRowLayout = new QHBoxLayout(accountRow);
        accountRowLayout->setContentsMargins(10, 0, 10, 0);
        accountRowLayout->setSpacing(8);

        Dtk::Widget::DLabel *iconLabel = new Dtk::Widget::DLabel(accountRow);
        iconLabel->setFixedSize(24, 24);
        iconLabel->setPixmap(CalDavAccountUi::providerIcon(static_cast<DCalDavProviderProfile::ProviderType>(status.providerType))
                                 .pixmap(24, 24));
        accountRowLayout->addWidget(iconLabel);

        Dtk::Widget::DLabel *nameLabel = new Dtk::Widget::DLabel(accountTitle(account, status), accountRow);
        nameLabel->setTextFormat(Qt::PlainText);
        nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        accountRowLayout->addWidget(nameLabel);
        accountRowLayout->addStretch();

        Dtk::Widget::DToolButton *deleteButton = new Dtk::Widget::DToolButton(accountRow);
        deleteButton->setAutoRaise(true);
        deleteButton->setAttribute(Qt::WA_Hover, true);
        deleteButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        deleteButton->setFocusPolicy(Qt::NoFocus);
        deleteButton->setFixedSize(28, 28);
        deleteButton->setIcon(QIcon(QStringLiteral(
            ":/icons/deepin/builtin/icons/dde_calendar_delete_16px.svg")));
        deleteButton->setIconSize(QSize(16, 16));
        const bool pendingOperations = status.pendingOperationCount > 0;
        deleteButton->setToolTip(pendingOperations ? tr("Pending synchronization") : tr("Delete"));
        deleteButton->setEnabled(true);
        connect(deleteButton, &Dtk::Widget::DToolButton::clicked, this, [this, accountID]() {
            emit deleteAccountRequested(accountID);
        });
        accountRowLayout->addWidget(deleteButton);
        accountLayout->addWidget(accountRow);

        QWidget *statusRow = new QWidget(accountGroup);
        statusRow->setFixedHeight(48);
        QHBoxLayout *statusLayout = new QHBoxLayout(statusRow);
        statusLayout->setContentsMargins(10, 0, 10, 0);
        statusLayout->setSpacing(6);

        const bool running = status.syncStatus == DCalDavSyncStatus::Running;
        const bool pendingDelete = status.pendingDeleteCount > 0;
        const bool failed = status.syncStatus == DCalDavSyncStatus::Failed
            || status.syncStatus == DCalDavSyncStatus::AuthenticationRequired
            || status.syncStatus == DCalDavSyncStatus::PermissionDenied
            || status.syncStatus == DCalDavSyncStatus::RetryScheduled;

        // The last successful sync time remains visible while a sync operation
        // is running or has failed. The current state is shown alongside it.
        Dtk::Widget::DLabel *timeLabel = new Dtk::Widget::DLabel(syncTimeText(status), statusRow);
        timeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        statusLayout->addWidget(timeLabel);

        if (status.conflictCount > 0) {
            Dtk::Widget::DLabel *stateIconLabel = new Dtk::Widget::DLabel(statusRow);
            stateIconLabel->setFixedSize(16, 16);
            stateIconLabel->setPixmap(syncStatusIcon(true).pixmap(16, 16));
            statusLayout->addWidget(stateIconLabel);
            Dtk::Widget::DLabel *stateLabel = new Dtk::Widget::DLabel(tr("Sync conflict"), statusRow);
            stateLabel->setToolTip(tr("A synchronization conflict was detected. The server version will replace the local changes."));
            statusLayout->addWidget(stateLabel);
        } else if (running || pendingDelete || failed) {
            Dtk::Widget::DLabel *stateIconLabel = new Dtk::Widget::DLabel(statusRow);
            stateIconLabel->setFixedSize(16, 16);
            stateIconLabel->setPixmap(syncStatusIcon(failed).pixmap(16, 16));
            statusLayout->addWidget(stateIconLabel);

            const QString stateText = failed
                ? tr("Sync Failed: %1").arg(
                      DCalDavSyncStatus::localizedFailureReason(
                    static_cast<DCalDavErrorCode>(status.failureCode)))
                : (pendingDelete ? tr("Deleting...") : tr("Syncing..."));
            Dtk::Widget::DLabel *stateLabel = new Dtk::Widget::DLabel(stateText, statusRow);
            stateLabel->setToolTip(failed
                ? DCalDavSyncStatus::localizedFailureReason(
                    static_cast<DCalDavErrorCode>(status.failureCode))
                : QString());
            stateLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
            stateLabel->setElideMode(Qt::ElideRight);
            stateLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            statusLayout->addWidget(stateLabel);
        }

        statusLayout->addStretch();
        Dtk::Widget::DPushButton *syncButton = new Dtk::Widget::DPushButton(tr("Sync Now"), statusRow);
        syncButton->setFixedSize(98, 36);
        syncButton->setEnabled(!running);
        connect(syncButton, &Dtk::Widget::DPushButton::clicked, this, [accountID]() {
            gAccountManager->downloadByAccountID(accountID);
        });
        statusLayout->addWidget(syncButton);
        accountLayout->addWidget(statusRow);

        m_cardsLayout->addWidget(accountGroup);
    }
}

void CalDavAccountListWidget::slotAccountUpdate()
{
    rebuildCards();
}

void CalDavAccountListWidget::slotStatusChanged(const QString &accountID)
{
    const DCalDavAccountStatus status = gAccountManager->getCalDavAccountStatus(accountID);
    if (status.conflictCount <= 0) {
        m_conflictNotices.remove(accountID);
    }

    rebuildCards();
    showConflictNotice(accountID);
}

void CalDavAccountListWidget::showConflictNotice(const QString &accountID)
{
    if (accountID.isEmpty() || m_conflictNotices.contains(accountID)) {
        return;
    }
    const DCalDavAccountStatus status = gAccountManager->getCalDavAccountStatus(accountID);
    if (status.conflictCount <= 0) {
        return;
    }
    QTimer::singleShot(0, this, [this, accountID]() {
        Dtk::Widget::DDialog dialog(this);
        dialog.setWindowTitle(tr("Calendar synchronization conflict"));
        Dtk::Widget::DLabel *message = new Dtk::Widget::DLabel(
            tr("A synchronization conflict was detected. The server version will replace the local changes."),
            &dialog);
        message->setWordWrap(true);
        dialog.addContent(message, Qt::AlignCenter);
        dialog.addButton(tr("Use server version"), false, Dtk::Widget::DDialog::ButtonWarning);
        QObject::connect(dialog.getButton(0), &QAbstractButton::clicked,
                         &dialog, [&dialog]() { dialog.done(1); });
        if (dialog.exec() == 1) {
            m_conflictNotices.insert(accountID);
            gAccountManager->resolveAllCalDavConflicts(accountID, false);
        }
    });
}
