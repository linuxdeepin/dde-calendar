// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "visitor.h"

using namespace CalendarKernel;

Visitor::Visitor()
{
}

Visitor::~Visitor()
{
}

bool Visitor::visit(const Event::Ptr &event)
{
    Q_UNUSED(event);
    return false;
}

bool Visitor::visit(const Todo::Ptr &todo)
{
    Q_UNUSED(todo);
    return false;
}

bool Visitor::visit(const Journal::Ptr &journal)
{
    Q_UNUSED(journal);
    return false;
}

bool Visitor::visit(const FreeBusy::Ptr &freebusy)
{
    Q_UNUSED(freebusy);
    return false;
}
