// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "contact.h"

namespace CalendarCore {

class Participant : public Contact
{
public:
    enum PartStat : int { NeedsAction, Accepted, Declined, Tentative, Delegated, Completed, InProcess, None };
    enum Role : int { ReqParticipant, OptParticipant, NonParticipant, Chair };

    Participant(const QString &name, const QString &email, bool rsvp = false,
                PartStat status = NeedsAction, Role role = ReqParticipant)
        : Contact(name, email), m_status(status), m_role(role), m_rsvp(rsvp) {}
    PartStat status() const { return m_status; }
    Role role() const { return m_role; }
    bool RSVP() const { return m_rsvp; }

private:
    PartStat m_status;
    Role m_role;
    bool m_rsvp;
};

} // namespace CalendarCore
