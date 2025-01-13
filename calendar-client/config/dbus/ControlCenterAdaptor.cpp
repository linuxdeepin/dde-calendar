// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ControlCenterAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class ControlCenterAdaptor
 */

ControlCenterAdaptor::ControlCenterAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ControlCenterAdaptor::~ControlCenterAdaptor()
{
    // destructor
}

void ControlCenterAdaptor::ShowPage(const QString &url)
{
    // handle method call org.deepin.dde.ControlCenter1.ShowPage
    QMetaObject::invokeMethod(parent(), "ShowPage", Q_ARG(QString, url));
}

