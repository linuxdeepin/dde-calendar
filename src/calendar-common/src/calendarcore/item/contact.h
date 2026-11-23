// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QString>

namespace CalendarCore {

class Contact
{
public:
    Contact() = default;
    Contact(const QString &name, const QString &email) : m_name(name), m_email(email) {}
    QString name() const { return m_name; }
    QString email() const { return m_email; }
    bool isEmpty() const { return m_name.isEmpty() && m_email.isEmpty(); }

private:
    QString m_name;
    QString m_email;
};

} // namespace CalendarCore
