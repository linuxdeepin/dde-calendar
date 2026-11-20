// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALENDARKERNEL_CALENDAR_P_H
#define CALENDARKERNEL_CALENDAR_P_H

#include "calendar.h"
#include "calfilter.h"

namespace CalendarKernel {
/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN Calendar::Private
{
public:
    Private()
        : mModified(false)
        , mNewObserver(false)
        , mObserversEnabled(true)
        , mDefaultFilter(new CalFilter)
        , batchAddingInProgress(false)
        , mDeletionTracking(true)
    {
        // Setup default filter, which does nothing
        mFilter = mDefaultFilter;
        mFilter->setEnabled(false);

        mOwner.setName(QStringLiteral("Unknown Name"));
        mOwner.setEmail(QStringLiteral("unknown@nowhere"));
    }

    ~Private()
    {
        if (mFilter != mDefaultFilter) {
            delete mFilter;
        }
        delete mDefaultFilter;
    }
    QTimeZone timeZoneIdSpec(const QByteArray &timeZoneId);

    QString mProductId;
    Person mOwner;
    QTimeZone mTimeZone;
    QVector<QTimeZone> mTimeZones;
    bool mModified = false;
    bool mNewObserver = false;
    bool mObserversEnabled = false;
    QList<CalendarObserver *> mObservers;

    CalFilter *mDefaultFilter = nullptr;
    CalFilter *mFilter = nullptr;

    // These lists are used to put together related To-dos
    QMultiHash<QString, Incidence::Ptr> mOrphans;
    QMultiHash<QString, Incidence::Ptr> mOrphanUids;

    // Lists for associating incidences to notebooks
    QMultiHash<QString, Incidence::Ptr> mNotebookIncidences;
    QHash<QString, QString> mUidToNotebook;
    QHash<QString, bool> mNotebooks; // name to visibility
    QHash<Incidence::Ptr, bool> mIncidenceVisibility; // incidence -> visibility
    QString mDefaultNotebook; // uid of default notebook
    QMap<QString, Incidence::List> mIncidenceRelations;
    bool batchAddingInProgress = false;
    bool mDeletionTracking = false;
    QString mId;
    QString mName;
    QString mIcon;
    AccessMode mAccessMode = ReadWrite;
};

} // namespace CalendarKernel

#endif
