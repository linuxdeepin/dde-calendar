// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTROLCENTERADAPTOR_H
#define CONTROLCENTERADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.deepin.dde.ControlCenter1
 */
class ControlCenterAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.ControlCenter1")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.dde.ControlCenter1\">\n"
"    <method name=\"ShowPage\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"url\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    ControlCenterAdaptor(QObject *parent);
    virtual ~ControlCenterAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void ShowPage(const QString &url);
Q_SIGNALS: // SIGNALS
};

#endif
