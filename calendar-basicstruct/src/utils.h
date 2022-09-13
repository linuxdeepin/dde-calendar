// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H
#include "commondatastruct.h"

#include <QDateTime>
#include <QJsonObject>

class Utils
{
public:
    Utils();
    static QString toconvertData(QDateTime date);
    static QDateTime fromconvertData(QString str);
    static QDateTime fromconvertiIGData(QString str);
    static QJsonObject JobToObject(const Job &job);
    static bool StringToObject(const QString &jsonStr, QJsonArray &);
};

#endif // UTILS_H
