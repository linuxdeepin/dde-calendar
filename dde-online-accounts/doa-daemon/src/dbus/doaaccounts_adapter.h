/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp com.dde.onlineaccount.accounts.xml -a doaaccounts_adapter -c DOAAccountsdapter
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef DOAACCOUNTS_ADAPTER_H
#define DOAACCOUNTS_ADAPTER_H

#include "doaprovider.h"

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

/*
 * Adaptor class for interface com.dde.onlineaccount.account
 */
class DOAAccountsadapter : public QObject
    , protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.dde.onlineaccount.account")
public:
    explicit DOAAccountsadapter(QObject *parent = nullptr);
    virtual ~DOAAccountsadapter();

public: // PROPERTIES
    //获取/修改同步状态
    Q_PROPERTY(bool CalendarDisabled READ calendarDisabled WRITE setCalendarDisabled)
    bool calendarDisabled() const;
    void setCalendarDisabled(bool value);

    //获取帐户ID
    Q_PROPERTY(QString Id READ id)
    QString id() const;

    //获取帐户状态
    Q_PROPERTY(QString Status READ status)
    QString status() const;

    //获取和修改用户名称
    Q_PROPERTY(QString UserName READ userName WRITE setUserName)
    QString userName() const;
    void setUserName(QString &userName);

    //获取服务商名称
    Q_PROPERTY(QString ProviderName READ providerName)
    QString providerName() const;

    //获取协议类型
    Q_PROPERTY(QString ProviderType READ providerType)
    QString providerType() const;

public Q_SLOTS: // METHODS
    Q_SCRIPTABLE bool Remove();
    Q_SCRIPTABLE void CheckAccountState();
    Q_SCRIPTABLE void loginCancle();

    void onNetWorkChange(bool active);

signals:
    void sign_changeProperty(const QString &propertyName, DOAProvider *doaProvider);
    void sign_remove(DOAAccountsadapter *doaAccountAdapter);
    //状态改变
    void sign_accountState(const QString &accountState);

public:
    DOAProvider *m_doaProvider {nullptr};

private:
    void sendPropertiesChanged(QVariantMap changed_properties);

    QTimer m_checkAccountCalendarTimer;
    bool networkerror = false;
};

#endif
