// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALENDAREVENTLOG_H
#define DCALENDAREVENTLOG_H

#include "dcaldavprofile.h"
#include "dcaldavvalidationerror.h"

#include <QJsonObject>
#include <QLibrary>
#include <QString>

#include <string>

class DCalendarEventLog
{
public:
    enum EventType {
        AddAccountClicked = 1002000000,
        LoginClicked = 1002000001,
        LoginValidationFinished = 1002000002,
        DeleteAccount = 1002000003,
    };

    static DCalendarEventLog &instance();

    void reportAddAccountClicked();
    void reportLoginClicked(DCalDavProviderProfile::ProviderType providerType);
    void reportLoginValidationFinished(bool success,
                                       DCalDavValidationError::Type validationError);
    void reportDeleteAccount(bool deleteData);

private:
    using InitializeFunction = bool (*)(const std::string &, bool);
    using WriteEventLogFunction = void (*)(const std::string &);

    DCalendarEventLog();

    void writeEvent(EventType eventType, const QJsonObject &fields = {});
    static QString accountType(DCalDavProviderProfile::ProviderType providerType);
    static QString validationFailureReason(DCalDavValidationError::Type validationError);

    QLibrary m_library;
    InitializeFunction m_initialize = nullptr;
    WriteEventLogFunction m_writeEventLog = nullptr;
    bool m_available = false;
};

#endif // DCALENDAREVENTLOG_H
