/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CalFormat base class.

  @brief
  Base class providing an interface to various calendar formats.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "calformat.h"
#include "exceptions.h"
#include <QLoggingCategory>
#include <QUuid>

Q_LOGGING_CATEGORY(calFormatLog, "calendar.format")

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::CalFormat::Private
{
public:
    Private()
    {
    }
    ~Private()
    {
        delete mException;
    }
    static QString mApplication; // Name of application, for creating unique ID strings
    static QString mProductId; // PRODID string to write to calendar files
    QString mLoadedProductId; // PRODID string loaded from calendar file 从日历文件加载的PRODID字符串
    Exception *mException = nullptr;
};

QString CalFormat::Private::mApplication = QStringLiteral("libkcal");
QString CalFormat::Private::mProductId = QStringLiteral("-//K Desktop Environment//NONSGML libkcal 4.3//EN");
//@endcond

CalFormat::CalFormat()
    : d(new KCalendarCore::CalFormat::Private)
{
}

CalFormat::~CalFormat()
{
    clearException();
    delete d;
}

void CalFormat::clearException()
{
    qCDebug(calFormatLog) << "Clearing exception";
    delete d->mException;
    d->mException = nullptr;
}

void CalFormat::setException(Exception *exception)
{
    qCWarning(calFormatLog) << "Setting exception:" << (exception ? exception->message() : "null");
    delete d->mException;
    d->mException = exception;
}

Exception *CalFormat::exception() const
{
    return d->mException;
}

void CalFormat::setApplication(const QString &application, const QString &productID)
{
    qCInfo(calFormatLog) << "Setting application:" << application << "with product ID:" << productID;
    Private::mApplication = application;
    Private::mProductId = productID;
}

const QString &CalFormat::application()
{
    return Private::mApplication;
}

const QString &CalFormat::productId()
{
    return Private::mProductId;
}

QString CalFormat::loadedProductId()
{
    qCDebug(calFormatLog) << "Getting loaded product ID:" << d->mLoadedProductId;
    return d->mLoadedProductId;
}

void CalFormat::setLoadedProductId(const QString &id)
{
    qCDebug(calFormatLog) << "Setting loaded product ID:" << id;
    d->mLoadedProductId = id;
}

QString CalFormat::createUniqueId()
{
    QString id = QUuid::createUuid().toString().mid(1, 36);
    qCDebug(calFormatLog) << "Created unique ID:" << id;
    return id;
}

void CalFormat::virtual_hook(int id, void *data)
{
    qCWarning(calFormatLog) << "Virtual hook called with id:" << id << "- Not implemented";
    Q_UNUSED(id);
    Q_UNUSED(data);
    Q_ASSERT(false);
}
