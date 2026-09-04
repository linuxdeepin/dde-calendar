// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVPROFILE_H
#define DCALDAVPROFILE_H

#include <QCoreApplication>
#include <QString>
#include <QUrl>

class DCalDavProviderProfile
{
public:
    enum ProviderType {
        Provider_DingTalk,
        Provider_WeCom,
        Provider_TencentMeeting,
        Provider_QQMail,
        Provider_Feishu,
        Provider_Other
    };

    DCalDavProviderProfile()
        : providerType(Provider_Other)
        , serverUrlEditable(true)
        , requiresUsername(true)
        , requiresPassword(true)
        , supportsWrite(true)
    {
    }

    static QString providerName(ProviderType type)
    {
        switch (type) {
        case Provider_DingTalk:
            return QCoreApplication::translate("DCalDavProviderProfile", "DingTalk");
        case Provider_WeCom:
            return QCoreApplication::translate("DCalDavProviderProfile", "WeCom");
        case Provider_TencentMeeting:
            return QCoreApplication::translate("DCalDavProviderProfile", "Tencent Meeting");
        case Provider_QQMail:
            return QCoreApplication::translate("DCalDavProviderProfile", "QQ Mail");
        case Provider_Feishu:
            return QCoreApplication::translate("DCalDavProviderProfile", "Feishu");
        case Provider_Other:
        default:
            return QCoreApplication::translate("DCalDavProviderProfile", "Other CalDAV");
        }
    }

    static QString accountDisplayName(ProviderType type, const QString &name)
    {
        return name.isEmpty() ? name : providerName(type) + QStringLiteral("-") + name;
    }

    static QUrl normalizeServerUrl(const QString &serverUrl)
    {
        QString value = serverUrl.trimmed();
        if (value.isEmpty()) {
            return QUrl();
        }
        if (!value.contains(QStringLiteral("://"))) {
            value.prepend(QStringLiteral("https://"));
        }
        QUrl url(value);
        if (!url.isValid() || url.host().isEmpty() || !url.userName().isEmpty()
            || !url.password().isEmpty()
            || url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0) {
            return QUrl();
        }
        url.setScheme(QStringLiteral("https"));
        url.setFragment(QString());
        return url;
    }

    static ProviderType providerTypeForServerUrl(const QString &serverUrl, ProviderType fallback)
    {
        const QString host = QUrl(serverUrl).host().toLower();
        if (host == QStringLiteral("caldav.wecom.work")) {
            return Provider_WeCom;
        }
        if (host == QStringLiteral("cal.meeting.tencent.com")) {
            return Provider_TencentMeeting;
        }
        if (host == QStringLiteral("calendar.dingtalk.com")) {
            return Provider_DingTalk;
        }
        if (host == QStringLiteral("dav.qq.com")) {
            return Provider_QQMail;
        }
        if (host == QStringLiteral("caldav.feishu.cn")) {
            return Provider_Feishu;
        }
        return fallback;
    }

    static DCalDavProviderProfile forProvider(ProviderType type)
    {
        DCalDavProviderProfile profile;
        profile.providerType = type;

        switch (type) {
        case Provider_DingTalk:
            profile.displayName = QStringLiteral("DingTalk");
            profile.serverUrl = QStringLiteral("https://calendar.dingtalk.com");
            break;
        case Provider_WeCom:
            profile.displayName = QStringLiteral("WeCom");
            profile.serverUrl = QStringLiteral("https://caldav.wecom.work");
            break;
        case Provider_TencentMeeting:
            profile.displayName = QStringLiteral("Tencent Meeting");
            profile.serverUrl = QStringLiteral("https://cal.meeting.tencent.com/caldav/");
            break;
        case Provider_QQMail:
            profile.displayName = QStringLiteral("QQ Mail");
            profile.serverUrl = QStringLiteral("https://dav.qq.com");
            break;
        case Provider_Feishu:
            profile.displayName = QStringLiteral("Feishu");
            profile.serverUrl = QStringLiteral("https://caldav.feishu.cn");
            break;
        case Provider_Other:
            profile.displayName = QStringLiteral("Other CalDAV");
            break;
        }

        return profile;
    }

    ProviderType providerType;
    QString displayName;
    QString serverUrl;
    bool serverUrlEditable;
    bool requiresUsername;
    bool requiresPassword;
    bool supportsWrite;
};

#endif // DCALDAVPROFILE_H
