// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALENDARKERNEL_VISITOR_H
#define CALENDARKERNEL_VISITOR_H

#include "event.h"
#include "freebusy.h"
#include "journal.h"
#include "todo.h"

namespace CalendarKernel {
/**
  This class provides the interface for a visitor of calendar components.
  It serves as base class for concrete visitors, which implement certain
  actions on calendar components. It allows to add functions, which operate
  on the concrete types of calendar components, without changing the
  calendar component classes.
*/
class Q_CORE_EXPORT Visitor // krazy:exclude=dpointer
{
public:
    /** Destruct Incidence::Visitor */
    virtual ~Visitor();

    /**
      Reimplement this function in your concrete subclass of
      IncidenceBase::Visitor to perform actions on an Event object.
      @param event is a pointer to a valid Event object.
    */
    virtual bool visit(const Event::Ptr &event);

    /**
      Reimplement this function in your concrete subclass of
      IncidenceBase::Visitor to perform actions on a Todo object.
      @param todo is a pointer to a valid Todo object.
    */
    virtual bool visit(const Todo::Ptr &todo);

    /**
      Reimplement this function in your concrete subclass of
      IncidenceBase::Visitor to perform actions on an Journal object.
      @param journal is a pointer to a valid Journal object.
    */
    virtual bool visit(const Journal::Ptr &journal);

    /**
      Reimplement this function in your concrete subclass of
      IncidenceBase::Visitor to perform actions on a FreeBusy object.
      @param freebusy is a pointer to a valid FreeBusy object.
    */
    virtual bool visit(const FreeBusy::Ptr &freebusy);

protected:
    /**
      Constructor is protected to prevent direct creation of visitor
      base class.
    */
    Visitor();
};

} // namespace CalendarKernel

#endif
