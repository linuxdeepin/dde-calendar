// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALDAVACCOUNTLISTWIDGET_H
#define CALDAVACCOUNTLISTWIDGET_H

#include <QWidget>
#include <QSet>

class QVBoxLayout;

class CalDavAccountListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CalDavAccountListWidget(QWidget *parent = nullptr);

signals:
    void addAccountRequested();
    void deleteAccountRequested(const QString &accountID);

private slots:
    void slotAccountUpdate();
    void slotStatusChanged(const QString &accountID);

private:
    void rebuildCards();
    void showConflictNotice(const QString &accountID);
    QVBoxLayout *m_cardsLayout = nullptr;
    QSet<QString> m_conflictNotices;
};

#endif // CALDAVACCOUNTLISTWIDGET_H
