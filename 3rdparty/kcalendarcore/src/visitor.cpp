/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  SPDX-FileContributor: Alvaro Manera <alvaro.manera@nokia.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "visitor.h"

using namespace KCalendarCore;

Visitor::Visitor()
{
}

Visitor::~Visitor()
{
}

bool Visitor::visit(const Event::Ptr &event)
{
    qCDebug(moduleLog) << "Visiting Event object:" << event->uid();
    Q_UNUSED(event);
    return false;
}

bool Visitor::visit(const Todo::Ptr &todo)
{
    qCDebug(moduleLog) << "Visiting Todo object:" << todo->uid();
    Q_UNUSED(todo);
    return false;
}

bool Visitor::visit(const Journal::Ptr &journal)
{
    qCDebug(moduleLog) << "Visiting Journal object:" << journal->uid();
    Q_UNUSED(journal);
    return false;
}

bool Visitor::visit(const FreeBusy::Ptr &freebusy)
{
    qCDebug(moduleLog) << "Visiting FreeBusy object:" << freebusy->uid();
    Q_UNUSED(freebusy);
    return false;
}
