// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVACCOUNTSTATUS_H
#define DCALDAVACCOUNTSTATUS_H

#include "dcaldaverrorcode.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QString>
#include <QVector>

class DCalDavSyncStatus
{
public:
    enum Value {
        Idle = 0,
        Running = 1,
        Succeeded = 2,
        Failed = 3,
        AuthenticationRequired = 4,
        PermissionDenied = 5,
        Conflict = 6,
        RetryScheduled = 7,
    };

    static Value fromErrorCode(DCalDavErrorCode errorCode)
    {
        switch (errorCode) {
        case DCalDavErrorCode::AuthenticationFailed:
            return AuthenticationRequired;
        case DCalDavErrorCode::PermissionDenied:
            return PermissionDenied;
        case DCalDavErrorCode::Conflict:
            return Conflict;
        default:
            return Failed;
        }
    }

    static QString localizedFailureReason(DCalDavErrorCode errorCode)
    {
        switch (errorCode) {
        case DCalDavErrorCode::CertificateInvalid:
            return QCoreApplication::translate(
                "DCalDavSyncStatus", "The server certificate is invalid.");
        case DCalDavErrorCode::AuthenticationFailed:
            return QCoreApplication::translate(
                "DCalDavSyncStatus", "Incorrect username or password. Please try again.");
        case DCalDavErrorCode::UnsupportedCalDav:
            return QCoreApplication::translate(
                "DCalDavSyncStatus",
                "This server does not support CalDAV.");
        case DCalDavErrorCode::ParseError:
            return QCoreApplication::translate(
                "DCalDavSyncStatus",
                "Unable to parse the data returned by the server. Please verify the server address or try again later.");
        case DCalDavErrorCode::PermissionDenied:
            return QCoreApplication::translate(
                "DCalDavSyncStatus", "The server denied access.");
        case DCalDavErrorCode::RequestTimedOut:
            return QCoreApplication::translate(
                "DCalDavSyncStatus", "The server request timed out.");
        case DCalDavErrorCode::NetworkUnavailable:
        case DCalDavErrorCode::NetworkError:
            return QCoreApplication::translate(
                "DCalDavSyncStatus",
                "Unable to connect to the server. Please check your network connection and server address.");
        default:
            return QCoreApplication::translate("DCalDavSyncStatus", "Synchronization failed.");
        }
    }

};

class DCalDavCredentialReference
{
public:
    static bool isValid(const QString &reference)
    {
        if (reference.isEmpty()) {
            return true;
        }
        static const QRegularExpression pattern(
            QStringLiteral("^secret-service:(/(?:[A-Za-z0-9_]+))+$"));
        return reference.size() <= 512 && pattern.match(reference).hasMatch();
    }
};

class DCalDavAccountStatus
{
public:
    typedef QVector<DCalDavAccountStatus> List;

    QString accountId;
    QString displayName;
    int providerType = 0;
    int syncStatus = 0;
    QDateTime lastSuccessfulSync;
    QString failureReason;
    int failureCode = static_cast<int>(DCalDavErrorCode::NoError);
    QString accountColor;
    bool supportsWrite = true;
    int pendingOperationCount = 0;
    int pendingDeleteCount = 0;
    int conflictCount = 0;
    QDateTime nextRetryAt;

    static QString toJsonListString(const List &statusList)
    {
        QJsonArray array;
        for (const DCalDavAccountStatus &status : statusList) {
            QJsonObject object;
            object.insert(QStringLiteral("accountId"), status.accountId);
            object.insert(QStringLiteral("displayName"), status.displayName);
            object.insert(QStringLiteral("providerType"), status.providerType);
            object.insert(QStringLiteral("syncStatus"), status.syncStatus);
            object.insert(QStringLiteral("lastSuccessfulSync"), status.lastSuccessfulSync.toString(Qt::ISODate));
            object.insert(QStringLiteral("failureReason"), status.failureReason);
            object.insert(QStringLiteral("failureCode"), status.failureCode);
            object.insert(QStringLiteral("accountColor"), status.accountColor);
            object.insert(QStringLiteral("supportsWrite"), status.supportsWrite);
            object.insert(QStringLiteral("pendingOperationCount"), status.pendingOperationCount);
            object.insert(QStringLiteral("pendingDeleteCount"), status.pendingDeleteCount);
            object.insert(QStringLiteral("conflictCount"), status.conflictCount);
            object.insert(QStringLiteral("nextRetryAt"), status.nextRetryAt.toString(Qt::ISODate));
            array.append(object);
        }
        return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }

    static bool fromJsonListString(List &statusList, const QString &json)
    {
        const QJsonDocument document = QJsonDocument::fromJson(json.toUtf8());
        if (!document.isArray()) {
            return false;
        }

        List parsed;
        for (const QJsonValue &value : document.array()) {
            if (!value.isObject()) {
                return false;
            }
            const QJsonObject object = value.toObject();
            DCalDavAccountStatus status;
            status.accountId = object.value(QStringLiteral("accountId")).toString();
            status.displayName = object.value(QStringLiteral("displayName")).toString();
            status.providerType = object.value(QStringLiteral("providerType")).toInt();
            status.syncStatus = object.value(QStringLiteral("syncStatus")).toInt();
            status.lastSuccessfulSync = QDateTime::fromString(object.value(QStringLiteral("lastSuccessfulSync")).toString(), Qt::ISODate);
            status.failureReason = object.value(QStringLiteral("failureReason")).toString();
            status.failureCode = object.contains(QStringLiteral("failureCode"))
                ? object.value(QStringLiteral("failureCode")).toInt()
                : static_cast<int>(DCalDavErrorCode::NoError);
            status.accountColor = object.value(QStringLiteral("accountColor")).toString();
            status.supportsWrite = object.value(QStringLiteral("supportsWrite")).toBool(true);
            status.pendingOperationCount = object.value(QStringLiteral("pendingOperationCount")).toInt();
            status.pendingDeleteCount = object.value(QStringLiteral("pendingDeleteCount")).toInt();
            status.conflictCount = object.value(QStringLiteral("conflictCount")).toInt();
            status.nextRetryAt = QDateTime::fromString(
                object.value(QStringLiteral("nextRetryAt")).toString(), Qt::ISODate);
            parsed.append(status);
        }
        statusList = parsed;
        return true;
    }
};

#endif // DCALDAVACCOUNTSTATUS_H
