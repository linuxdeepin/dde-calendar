// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QMap>
#include <QString>

namespace CalendarCore {

class PropertyBag
{
public:
    struct Property {
        QString value;
        QByteArray source;
    };

    QString nonKDECustomProperty(const QByteArray &name) const { return m_properties.value(name.toUpper()).value; }
    void setNonKDECustomProperty(const QByteArray &name, const QString &value, const QByteArray &source = {})
    {
        if (value.isEmpty()) {
            m_properties.remove(name.toUpper());
        } else {
            m_properties.insert(name.toUpper(), Property{value, source});
        }
    }
    const QMap<QByteArray, Property> &customProperties() const { return m_properties; }

private:
    QMap<QByteArray, Property> m_properties;
};

} // namespace CalendarCore
