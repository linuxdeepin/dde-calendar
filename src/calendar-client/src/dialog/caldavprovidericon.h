// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALDAVPROVIDERICON_H
#define CALDAVPROVIDERICON_H

#include "dcaldavprofile.h"

#include <QIcon>
#include <QString>

namespace CalDavAccountUi {

inline QIcon providerIcon(DCalDavProviderProfile::ProviderType provider)
{
    QString resourcePath;
    switch (provider) {
    case DCalDavProviderProfile::Provider_DingTalk:
        resourcePath = QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_caldav_dingtalk_24px.svg");
        break;
    case DCalDavProviderProfile::Provider_WeCom:
        resourcePath = QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_caldav_wecom_24px.svg");
        break;
    case DCalDavProviderProfile::Provider_TencentMeeting:
        resourcePath = QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_caldav_tencent_meeting_24px.svg");
        break;
    case DCalDavProviderProfile::Provider_QQMail:
        resourcePath = QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_caldav_qq_mail_24px.svg");
        break;
    case DCalDavProviderProfile::Provider_Feishu:
        resourcePath = QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_caldav_feishu_24px.svg");
        break;
    case DCalDavProviderProfile::Provider_Other:
    default:
        resourcePath = QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_caldav_other_24px.svg");
        break;
    }
    return QIcon(resourcePath);
}

} // namespace CalDavAccountUi

#endif // CALDAVPROVIDERICON_H
