/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the FileStorage class.

  @brief
  This class provides a calendar storage as a local file.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/
#include "filestorage.h"
#include "exceptions.h"
#include "icalformat.h"
#include "memorycalendar.h"
#include "vcalformat.h"

#include <QDebug>

using namespace KCalendarCore;

Q_LOGGING_CATEGORY(fileStorageLog, "calendar.storage.file")

/*
  Private class that helps to provide binary compatibility between releases.
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::FileStorage::Private
{
public:
    Private(const QString &fileName, CalFormat *format)
        : mFileName(fileName)
        , mSaveFormat(format)
    {
    }
    ~Private()
    {
        delete mSaveFormat;
    }

    QString mFileName;
    CalFormat *mSaveFormat = nullptr;
};
//@endcond

FileStorage::FileStorage(const Calendar::Ptr &cal, const QString &fileName, CalFormat *format)
    : CalStorage(cal)
    , d(new Private(fileName, format))
{
}

FileStorage::~FileStorage()
{
    delete d;
}

void FileStorage::setFileName(const QString &fileName)
{
    d->mFileName = fileName;
}

QString FileStorage::fileName() const
{
    return d->mFileName;
}

void FileStorage::setSaveFormat(CalFormat *format)
{
    delete d->mSaveFormat;
    d->mSaveFormat = format;
}

CalFormat *FileStorage::saveFormat() const
{
    return d->mSaveFormat;
}

bool FileStorage::open()
{
    return true;
}

bool FileStorage::load()
{
    if (d->mFileName.isEmpty()) {
        qCWarning(fileStorageLog) << "Empty filename while trying to load";
        return false;
    }

    // Always try to load with iCalendar. It will detect, if it is actually a
    // vCalendar file.
    bool success;
    QString productId;
    // First try the supplied format. Otherwise fall through to iCalendar, then
    // to vCalendar
    qCDebug(fileStorageLog) << "Attempting to load calendar from file:" << d->mFileName;
    success = saveFormat() && saveFormat()->load(calendar(), d->mFileName);
    if (success) {
        productId = saveFormat()->loadedProductId();
        qCInfo(fileStorageLog) << "Successfully loaded calendar with format:" << productId;
    } else {
        ICalFormat iCal;
        qCDebug(fileStorageLog) << "Trying to load as iCalendar format";
        success = iCal.load(calendar(), d->mFileName);

        if (success) {
            productId = iCal.loadedProductId();
            qCInfo(fileStorageLog) << "Successfully loaded calendar as iCalendar format:" << productId;
        } else {
            if (iCal.exception()) {
                if ((iCal.exception()->code() == Exception::ParseErrorIcal) || (iCal.exception()->code() == Exception::CalVersion1)) {
                    // Possible vCalendar or invalid iCalendar encountered
                    qCWarning(fileStorageLog) << d->mFileName << "is an invalid iCalendar or possibly a vCalendar, trying vCalendar format";
                    VCalFormat vCal;
                    success = vCal.load(calendar(), d->mFileName);
                    productId = vCal.loadedProductId();
                    if (!success) {
                        if (vCal.exception()) {
                            qCCritical(fileStorageLog) << d->mFileName << "is not a valid vCalendar file. Exception code:" << vCal.exception()->code();
                        }
                        return false;
                    }
                    qCInfo(fileStorageLog) << "Successfully loaded calendar as vCalendar format:" << productId;
                } else {
                    qCCritical(fileStorageLog) << "Failed to load calendar. Exception code:" << iCal.exception()->code();
                    return false;
                }
            } else {
                qCCritical(fileStorageLog) << "Failed to load calendar. No exception set.";
                return false;
            }
        }
    }

    calendar()->setProductId(productId);
    calendar()->setModified(false);
    qCDebug(fileStorageLog) << "Calendar loading completed successfully";

    return true;
}

bool FileStorage::save()
{
    if (d->mFileName.isEmpty()) {
        qCWarning(fileStorageLog) << "Empty filename while trying to save";
        return false;
    }

    CalFormat *format = d->mSaveFormat ? d->mSaveFormat : new ICalFormat;
    qCDebug(fileStorageLog) << "Saving calendar to file:" << d->mFileName;

    bool success = format->save(calendar(), d->mFileName);

    if (success) {
        calendar()->setModified(false);
        qCInfo(fileStorageLog) << "Calendar saved successfully";
    } else {
        if (!format->exception()) {
            qCCritical(fileStorageLog) << "Failed to save calendar. No exception set.";
        } else {
            qCCritical(fileStorageLog) << "Failed to save calendar. Exception code:" << format->exception()->code();
        }
    }

    if (!d->mSaveFormat) {
        delete format;
    }

    return success;
}

bool FileStorage::close()
{
    return true;
}
