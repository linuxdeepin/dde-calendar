// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "icscodec.h"

#include <QDebug>
#include <QFile>
#include <QHash>
#include <QSaveFile>
#include <libical/ical.h>

#include <memory>
#include <vector>

using namespace CalendarCore;

namespace {

constexpr int MaxIcsBytes = 32 * 1024 * 1024;

constexpr char LunarProperty[] = "X-DDE-ICAL-LUNNAR";
constexpr char EnabledProperty[] = "X-KDE-KCALCORE-ENABLED";
constexpr char VersionProperty[] = "X-KDE-ICAL-IMPLEMENTATION-VERSION";
constexpr char SchedulingIdProperty[] = "X-KDE-LIBKCAL-ID";
using Component = std::unique_ptr<icalcomponent, decltype(&icalcomponent_free)>;
using TimeZone = std::unique_ptr<icaltimezone, void (*)(icaltimezone *)>;

QByteArray bytes(const QString &text) { return text.toUtf8(); }
QString text(const char *value) { return QString::fromUtf8(value); }

QByteArray serialized(icalproperty *property)
{
    char *buffer = icalproperty_as_ical_string_r(property);
    const QByteArray result(buffer);
    icalmemory_free_buffer(buffer);
    return result;
}

void appendRaw(icalcomponent *target, const QByteArray &source)
{
    if (auto property = icalproperty_new_from_string(source.constData())) {
        icalcomponent_add_property(target, property);
    } else {
        qWarning("Cannot restore an iCalendar property during serialization");
    }
}

void appendExtension(icalcomponent *target, const QByteArray &name, const QString &value)
{
    auto property = icalproperty_new_x(bytes(value).constData());
    icalproperty_set_x_name(property, name.constData());
    icalcomponent_add_property(target, property);
}

void appendExtensions(icalcomponent *target, const PropertyBag &bag)
{
    const auto &properties = bag.customProperties();
    for (auto entry = properties.cbegin(); entry != properties.cend(); ++entry) {
        if (entry.key().startsWith("X-KDE-VOLATILE")) {
            continue;
        }
        if (entry->source.isEmpty()) {
            appendExtension(target, entry.key(), entry->value);
        } else {
            appendRaw(target, entry->source);
        }
    }
}

void readExtension(PropertyBag &bag, icalproperty *property)
{
    bag.setNonKDECustomProperty(icalproperty_get_x_name(property),
                                text(icalproperty_get_value_as_string(property)), serialized(property));
}

QString parameter(icalproperty *property, const char *name)
{
    return text(icalproperty_get_parameter_as_string(property, name));
}

QByteArray timeZoneId(icalproperty *property)
{
    return bytes(parameter(property, "TZID").section(QLatin1Char(';'), 0, 0));
}

QString emailAddress(QString address)
{
    if (address.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
        address.remove(0, 7);
    }
    return address;
}

icaltimetype fieldsAt(const QDateTime &dateTime, bool dateOnly, bool utc)
{
    const auto converted = utc ? dateTime.toUTC() : dateTime;
    auto result = icaltime_null_time();
    result.year = converted.date().year();
    result.month = converted.date().month();
    result.day = converted.date().day();
    if (!dateOnly) {
        result.hour = converted.time().hour();
        result.minute = converted.time().minute();
        result.second = converted.time().second();
    }
    result.is_date = dateOnly;
    result.zone = utc ? icaltimezone_get_utc_timezone() : nullptr;
    return result;
}

TimeSpan fromDuration(icaldurationtype duration)
{
    const qint64 sign = duration.is_neg ? -1 : 1;
    const qint64 days = qint64(duration.weeks) * 7 + duration.days;
    const qint64 seconds = qint64(duration.hours) * 3600 + qint64(duration.minutes) * 60 + duration.seconds;
    if (seconds || !days) {
        return TimeSpan(sign * (days * 86400 + seconds));
    }
    return TimeSpan::days(sign * days);
}

icaldurationtype toDuration(TimeSpan duration)
{
    auto result = icaldurationtype_null_duration();
    const qint64 amount = duration.isDaily() ? duration.asDays() : duration.asSeconds();
    const auto magnitude = qAbs(amount);
    result.is_neg = amount < 0;
    if (duration.isDaily()) {
        result.days = magnitude;
    } else {
        result.days = magnitude / 86400;
        result.hours = magnitude / 3600 % 24;
        result.minutes = magnitude / 60 % 60;
        result.seconds = magnitude % 60;
    }
    return result;
}

} // namespace

struct IcsCodec::Reader {
    QMap<QByteArray, icaltimezone *> customZones;
    QMap<QByteArray, QByteArray> zoneComponents;
    mutable QHash<QByteArray, QTimeZone> namedZones;
    std::vector<TimeZone> zoneOwners;
    QString diagnostic;
    int duplicateEvents = 0;
    bool legacyKdeCalendar = false;
    bool legacyKdeTimestamps = false;
    QTimeZone floatingZone;
    ReadMode mode;

    QTimeZone namedTimeZone(const QByteArray &id) const
    {
        const auto cached = namedZones.constFind(id);
        if (cached != namedZones.cend()) {
            return cached.value();
        }
        const QTimeZone zone(id);
        namedZones.insert(id, zone);
        return zone;
    }

    QDateTime dateTime(icaltimetype value, icalproperty *property = nullptr) const
    {
        const QDate date(value.year, value.month, value.day);
        const QTime time = value.is_date ? QTime(0, 0) : QTime(value.hour, value.minute, value.second);
        if (!date.isValid() || !time.isValid()) {
            return {};
        }
        if (icaltime_is_utc(value)) {
            return QDateTime(date, time, Qt::UTC);
        }
        if (value.is_date) {
            return QDateTime(date, time);
        }
        if (!property) {
            return QDateTime(date, time, floatingZone);
        }
        const QByteArray id = timeZoneId(property);
        if (!id.isEmpty()) {
            const QTimeZone namedZone = namedTimeZone(id);
            if (namedZone.isValid()) {
                return QDateTime(date, time, namedZone);
            }
            if (auto custom = customZones.value(id)) {
                int daylight = 0;
                const int seconds = icaltimezone_get_utc_offset(custom, &value, &daylight);
                const QString name = QString::fromUtf8(id);
                const QTimeZone zone(id, seconds, name, name);
                return QDateTime(date, time, zone.isValid() ? zone : QTimeZone(seconds));
            }
        }
        return QDateTime(date, time, floatingZone);
    }

    void readAlarm(CalEvent &event, icalcomponent *component) const
    {
        Remind::Type type = Remind::Display;
        const auto action = icalcomponent_get_first_property(component, ICAL_ACTION_PROPERTY);
        switch (action ? icalproperty_get_action(action) : ICAL_ACTION_NONE) {
        case ICAL_ACTION_AUDIO: type = Remind::Audio; break;
        case ICAL_ACTION_EMAIL: type = Remind::Email; break;
        case ICAL_ACTION_PROCEDURE: type = Remind::Procedure; break;
        default: break;
        }
        const auto alarm = event.newAlarm();
        alarm->setType(type);
        alarm->setEnabled(true);
        for (auto property = icalcomponent_get_first_property(component, ICAL_ANY_PROPERTY); property;
             property = icalcomponent_get_next_property(component, ICAL_ANY_PROPERTY)) {
            switch (icalproperty_isa(property)) {
            case ICAL_ACTION_PROPERTY:
                break;
            case ICAL_DESCRIPTION_PROPERTY:
                alarm->setText(text(icalproperty_get_description(property)));
                break;
            case ICAL_TRIGGER_PROPERTY: {
                const auto trigger = icalproperty_get_trigger(property);
                if (icaltime_is_null_time(trigger.time)) {
                    auto related = icalproperty_get_first_parameter(property, ICAL_RELATED_PARAMETER);
                    alarm->setRelativeOffset(fromDuration(trigger.duration),
                                             related && icalparameter_get_related(related) == ICAL_RELATED_END);
                } else {
                    alarm->setTime(dateTime(trigger.time, property));
                }
                break;
            }
            case ICAL_X_PROPERTY:
                if (QByteArray(icalproperty_get_x_name(property)).toUpper() == EnabledProperty) {
                    alarm->setEnabled(qstricmp(icalproperty_get_x(property), "TRUE") == 0);
                } else {
                    readExtension(*alarm, property);
                }
                break;
            case ICAL_XLICERROR_PROPERTY:
                break;
            default:
                alarm->m_extraProperties.append(serialized(property));
            }
        }
    }

    CalEvent::Ptr readEvent(icalcomponent *component, bool deleted)
    {
        const auto event = CalEvent::Ptr::create();
        const QString uid = text(icalcomponent_get_uid(component));
        const auto invalidRule = [&](const QString &property) {
            diagnostic = QStringLiteral("Invalid %1 on event UID %2").arg(property, uid);
            return CalEvent::Ptr();
        };
        const auto start = icalcomponent_get_first_property(component, ICAL_DTSTART_PROPERTY);
        if (start) {
            const auto value = icalproperty_get_dtstart(start);
            event->setAllDay(value.is_date);
            event->setDtStart(dateTime(value, start));
            const QByteArray zoneId = timeZoneId(start);
            const auto definition = zoneComponents.constFind(zoneId);
            if (!value.is_date && definition != zoneComponents.cend()) {
                event->recurrence()->setCustomTimeZone(zoneId, definition.value());
            }
        }

        QDateTime stamp;
        QDateTime created;
        for (auto property = icalcomponent_get_first_property(component, ICAL_ANY_PROPERTY); property;
             property = icalcomponent_get_next_property(component, ICAL_ANY_PROPERTY)) {
            const auto tzid = icalproperty_get_first_parameter(property, ICAL_TZID_PARAMETER);
            const auto definition = zoneComponents.constFind(tzid ? timeZoneId(property) : QByteArray());
            if (definition != zoneComponents.cend()) {
                event->m_timeZoneComponents.insert(definition.key(), definition.value());
            }
            switch (icalproperty_isa(property)) {
            case ICAL_DTSTART_PROPERTY:
                break;
            case ICAL_DTSTAMP_PROPERTY:
                stamp = dateTime(icalproperty_get_dtstamp(property), property);
                break;
            case ICAL_XLICERROR_PROPERTY: {
                const auto error = text(icalproperty_get_xlicerror(property));
                if (mode == ReadMode::ExistingData) {
                    break;
                }
                if (!deleted && error.contains(QStringLiteral("RRULE"), Qt::CaseInsensitive)) {
                    return invalidRule(QStringLiteral("RRULE"));
                }
                if (!deleted && error.contains(QStringLiteral("EXRULE"), Qt::CaseInsensitive)) {
                    return invalidRule(QStringLiteral("EXRULE"));
                }
                break;
            }
            case ICAL_DTEND_PROPERTY: {
                const auto value = icalproperty_get_dtend(property);
                auto end = dateTime(value, property);
                if (value.is_date && end.isValid()) {
                    // The model stores inclusive dates. Legacy calendars can use a same-day DTEND.
                    end.setDate(qMax(event->dtStart().date(), end.date().addDays(-1)));
                }
                event->setDtEnd(end);
                event->setAllDay(value.is_date);
                break;
            }
            case ICAL_DURATION_PROPERTY:
                event->setDuration(fromDuration(icalproperty_get_duration(property)));
                break;
            case ICAL_UID_PROPERTY:
                event->setUid(text(icalproperty_get_uid(property)));
                break;
            case ICAL_CREATED_PROPERTY:
                created = dateTime(icalproperty_get_created(property), property);
                event->setCreated(created);
                break;
            case ICAL_LASTMODIFIED_PROPERTY:
                event->setLastModified(dateTime(icalproperty_get_lastmodified(property), property));
                break;
            case ICAL_CATEGORIES_PROPERTY: {
                const auto categories = text(icalproperty_get_categories(property)).split(QLatin1Char(','));
                for (const auto &category : categories) {
                    if (!category.isEmpty() && !event->m_categories.contains(category)) {
                        event->m_categories.append(category);
                    }
                }
                event->m_extraProperties.append(serialized(property));
                break;
            }
            case ICAL_SEQUENCE_PROPERTY:
                event->setRevision(icalproperty_get_sequence(property));
                break;
            case ICAL_SUMMARY_PROPERTY:
                event->setSummary(text(icalproperty_get_summary(property)));
                event->m_summaryRich = parameter(property, "X-KDE-TEXTFORMAT").compare(QStringLiteral("HTML"), Qt::CaseInsensitive) == 0;
                break;
            case ICAL_DESCRIPTION_PROPERTY:
                event->setDescription(text(icalproperty_get_description(property)));
                event->m_descriptionRich = parameter(property, "X-KDE-TEXTFORMAT").compare(QStringLiteral("HTML"), Qt::CaseInsensitive) == 0;
                break;
            case ICAL_LOCATION_PROPERTY:
                event->setLocation(text(icalproperty_get_location(property)));
                event->m_locationRich = parameter(property, "X-KDE-TEXTFORMAT").compare(QStringLiteral("HTML"), Qt::CaseInsensitive) == 0;
                break;
            case ICAL_STATUS_PROPERTY:
                if (icalproperty_get_status(property) == ICAL_STATUS_CANCELLED) {
                    event->setCancelled(true);
                } else {
                    event->m_extraProperties.append(serialized(property));
                }
                break;
            case ICAL_RECURRENCEID_PROPERTY: {
                const auto value = icalproperty_get_recurrenceid(property);
                event->setRecurrenceId(dateTime(value, property));
                event->m_recurrenceIdDateOnly = value.is_date;
                event->m_thisAndFuture = parameter(property, "RANGE").compare(QStringLiteral("THISANDFUTURE"), Qt::CaseInsensitive) == 0
                    || parameter(property, "TZID").contains(QStringLiteral("RANGE=THISANDFUTURE"), Qt::CaseInsensitive);
                break;
            }
            case ICAL_RRULE_PROPERTY:
            case ICAL_EXRULE_PROPERTY: {
                if (deleted) {
                    break;
                }
                auto rule = std::unique_ptr<RRule>(new RRule);
                const bool isRule = icalproperty_isa(property) == ICAL_RRULE_PROPERTY;
                const auto ruleText = text(icalproperty_get_value_as_string(property));
                if (!rule->parse(ruleText)) {
                    const auto name = isRule ? QStringLiteral("RRULE") : QStringLiteral("EXRULE");
                    if (mode == ReadMode::Strict) {
                        return invalidRule(name);
                    }
                    // Old stores may contain invalid end dates, which the old reader treated as open-ended.
                    if (!rule->parse(ruleText, true)) {
                        qWarning().noquote() << "Ignoring unreadable" << name << "on stored event UID" << uid;
                        break;
                    }
                    qWarning().noquote() << "Ignoring invalid UNTIL on stored event UID" << uid;
                }
                if (legacyKdeCalendar) {
                    // The old codec resolved a floating UNTIL in the system zone,
                    // even when DTSTART had an explicit time zone.
                    rule->bindFloatingUntil(QTimeZone::systemTimeZone());
                }
                if (isRule) {
                    event->recurrence()->addRRule(rule.release());
                } else {
                    event->recurrence()->addExRule(rule.release());
                }
                break;
            }
            case ICAL_RDATE_PROPERTY: {
                const auto value = icalproperty_get_rdate(property);
                if (icaltime_is_null_time(value.time)) {
                    event->m_extraProperties.append(serialized(property));
                } else if (value.time.is_date) {
                    event->recurrence()->addRDate(dateTime(value.time, property).date());
                } else {
                    event->recurrence()->addRDateTime(dateTime(value.time, property));
                }
                break;
            }
            case ICAL_EXDATE_PROPERTY: {
                const auto value = icalproperty_get_exdate(property);
                if (value.is_date) {
                    event->recurrence()->addExDate(dateTime(value, property).date());
                } else {
                    event->recurrence()->addExDateTime(dateTime(value, property));
                }
                break;
            }
            case ICAL_ORGANIZER_PROPERTY:
                event->setOrganizer(Contact(parameter(property, "CN"), emailAddress(text(icalproperty_get_organizer(property)))));
                event->m_organizerProperty = serialized(property);
                break;
            case ICAL_ATTENDEE_PROPERTY:
                event->addAttendee(Participant(parameter(property, "CN"), emailAddress(text(icalproperty_get_attendee(property)))));
                event->m_attendeeProperties.append(serialized(property));
                break;
            case ICAL_X_PROPERTY:
                readExtension(*event, property);
                break;
            default:
                // Retain calendar data that the UI does not edit, such as conferencing
                // details and URI or binary attachments.
                event->m_extraProperties.append(serialized(property));
            }
        }
        if (legacyKdeTimestamps && !created.isValid() && stamp.isValid()) {
            event->setCreated(stamp);
        }
        event->m_parsedAttendeeCount = event->attendees().size();
        const auto schedulingId = event->nonKDECustomProperty(SchedulingIdProperty);
        if (!schedulingId.isEmpty()) {
            event->setSchedulingID(event->uid(), schedulingId);
            event->setNonKDECustomProperty(SchedulingIdProperty, {});
        }
        const auto microsoftAllDay = event->nonKDECustomProperty("X-MICROSOFT-CDO-ALLDAYEVENT");
        if (!microsoftAllDay.isEmpty()) {
            event->setAllDay(microsoftAllDay.compare(QStringLiteral("TRUE"), Qt::CaseInsensitive) == 0);
        }
        event->setLunar(event->nonKDECustomProperty(LunarProperty).contains(QStringLiteral("TRUE")));
        if (event->summary().isEmpty() && !event->description().isEmpty()) {
            const auto description = event->description().trimmed();
            event->setSummary(description.section(QLatin1Char('\n'), 0, 0));
            if (event->summary() == description) {
                event->setDescription(QString());
            }
        }
        for (auto alarm = icalcomponent_get_first_component(component, ICAL_VALARM_COMPONENT); alarm;
             alarm = icalcomponent_get_next_component(component, ICAL_VALARM_COMPONENT)) {
            readAlarm(*event, alarm);
        }
        return event;
    }

    bool readCalendar(icalcomponent *component, CalStore &store, bool deleted)
    {
        auto version = icalcomponent_get_first_property(component, ICAL_VERSION_PROPERTY);
        if (!version || QByteArray(icalproperty_get_version(version)) != "2.0") {
            diagnostic = QStringLiteral("Expected iCalendar VERSION:2.0");
            return false;
        }
        for (auto zone = icalcomponent_get_first_component(component, ICAL_VTIMEZONE_COMPONENT); zone;
             zone = icalcomponent_get_next_component(component, ICAL_VTIMEZONE_COMPONENT)) {
            auto id = icalcomponent_get_first_property(zone, ICAL_TZID_PROPERTY);
            if (!id || QByteArray(icalproperty_get_tzid(id)).trimmed().isEmpty()) {
                qWarning("Ignoring VTIMEZONE without a TZID");
                continue;
            }
            // Date-times with a system TZID use its rules. Export those same rules.
            if (namedTimeZone(icalproperty_get_tzid(id)).isValid()) {
                continue;
            }
            TimeZone owner(icaltimezone_new(), [](icaltimezone *value) { icaltimezone_free(value, 1); });
            Component clone(icalcomponent_new_clone(zone), icalcomponent_free);
            if (icaltimezone_set_component(owner.get(), clone.get())) {
                clone.release();
                const QByteArray zoneId = icaltimezone_get_tzid(owner.get());
                char *raw = icalcomponent_as_ical_string_r(icaltimezone_get_component(owner.get()));
                zoneComponents.insert(zoneId, QByteArray(raw));
                icalmemory_free_buffer(raw);
                customZones.insert(zoneId, owner.get());
                zoneOwners.push_back(std::move(owner));
            }
        }
        const auto product = icalcomponent_get_first_property(component, ICAL_PRODID_PROPERTY);
        const auto productId = product ? QByteArray(icalproperty_get_prodid(product)) : QByteArray();
        QString implementationVersion;
        for (auto property = icalcomponent_get_first_property(component, ICAL_X_PROPERTY); property;
             property = icalcomponent_get_next_property(component, ICAL_X_PROPERTY)) {
            if (QByteArray(icalproperty_get_x_name(property)).toUpper() == VersionProperty) {
                implementationVersion = text(icalproperty_get_value_as_string(property));
            } else {
                readExtension(store, property);
            }
        }
        legacyKdeCalendar = productId.contains("libkcal") || productId.contains("KOrganizer") || productId.contains("KAlarm");
        legacyKdeTimestamps = legacyKdeCalendar && implementationVersion.isEmpty();
        for (auto entry = icalcomponent_get_first_component(component, ICAL_VEVENT_COMPONENT); entry;
             entry = icalcomponent_get_next_component(component, ICAL_VEVENT_COMPONENT)) {
            auto event = readEvent(entry, deleted);
            if (!event) {
                return false;
            }
            if (deleted) {
                store.removeEvent(event->uid(), event->recurrenceId());
            } else if (!store.addEvent(event) && !event->uid().isEmpty()) {
                ++duplicateEvents;
            }
        }
        return true;
    }
};

struct IcsCodec::Writer {
    QMap<QByteArray, QTimeZone> zones;
    QMap<QByteArray, QByteArray> customZones;

    icalproperty *timeProperty(icalproperty_kind kind, const QDateTime &time, bool dateOnly = false)
    {
        const auto zone = time.timeZone();
        const bool utc = !dateOnly && zone == QTimeZone::utc();
        const auto value = fieldsAt(time, dateOnly, utc);
        auto property = icalproperty_new(kind);
        icalproperty_set_value(property, dateOnly ? icalvalue_new_date(value) : icalvalue_new_datetime(value));
        if (!dateOnly && !utc && time.timeSpec() != Qt::LocalTime && zone.isValid()) {
            const auto id = zone.id();
            if (!customZones.contains(id)) {
                zones.insert(id, zone);
            }
            icalproperty_add_parameter(property, icalparameter_new_tzid(id.constData()));
        }
        return property;
    }

    void addText(icalcomponent *component, icalproperty_kind kind, const QString &value, bool rich)
    {
        if (value.isEmpty()) {
            return;
        }
        auto property = icalproperty_new(kind);
        icalproperty_set_value(property, icalvalue_new_text(bytes(value).constData()));
        if (rich) {
            auto format = icalparameter_new_x("HTML");
            icalparameter_set_xname(format, "X-KDE-TEXTFORMAT");
            icalproperty_add_parameter(property, format);
        }
        icalcomponent_add_property(component, property);
    }

    void addContact(icalcomponent *component, icalproperty_kind kind, const Contact &person,
                    const Participant *participant = nullptr)
    {
        if (person.email().isEmpty()) {
            return;
        }
        const auto address = QByteArray("MAILTO:") + bytes(person.email());
        auto property = kind == ICAL_ORGANIZER_PROPERTY ? icalproperty_new_organizer(address.constData())
                                                       : icalproperty_new_attendee(address.constData());
        if (!person.name().isEmpty()) {
            icalproperty_add_parameter(property, icalparameter_new_cn(bytes(person.name()).constData()));
        }
        if (participant) {
            const icalparameter_partstat statuses[] = { ICAL_PARTSTAT_NEEDSACTION, ICAL_PARTSTAT_ACCEPTED,
                ICAL_PARTSTAT_DECLINED, ICAL_PARTSTAT_TENTATIVE, ICAL_PARTSTAT_DELEGATED, ICAL_PARTSTAT_COMPLETED,
                ICAL_PARTSTAT_INPROCESS, ICAL_PARTSTAT_NONE };
            const icalparameter_role roles[] = { ICAL_ROLE_REQPARTICIPANT, ICAL_ROLE_OPTPARTICIPANT,
                                                ICAL_ROLE_NONPARTICIPANT, ICAL_ROLE_CHAIR };
            icalproperty_add_parameter(property, icalparameter_new_role(roles[participant->role()]));
            icalproperty_add_parameter(property, icalparameter_new_partstat(statuses[participant->status()]));
            icalproperty_add_parameter(property, icalparameter_new_rsvp(participant->RSVP() ? ICAL_RSVP_TRUE : ICAL_RSVP_FALSE));
        }
        icalcomponent_add_property(component, property);
    }

    icalcomponent *writeAlarm(const Remind &alarm)
    {
        auto component = icalcomponent_new_valarm();
        addText(component, ICAL_DESCRIPTION_PROPERTY, alarm.m_text, false);
        for (const auto &property : alarm.m_extraProperties) {
            appendRaw(component, property);
        }
        icalproperty_action action = ICAL_ACTION_DISPLAY;
        switch (alarm.type()) {
        case Remind::Audio: action = ICAL_ACTION_AUDIO; break;
        case Remind::Email: action = ICAL_ACTION_EMAIL; break;
        case Remind::Procedure: action = ICAL_ACTION_PROCEDURE; break;
        default: break;
        }
        icalcomponent_add_property(component, icalproperty_new_action(action));
        icaltriggertype trigger{};
        if (alarm.absoluteTime().isValid()) {
            trigger.time = fieldsAt(alarm.absoluteTime(), false, true);
        } else {
            trigger.duration = toDuration(alarm.m_offset);
        }
        auto property = icalproperty_new_trigger(trigger);
        if (alarm.m_offsetFromEnd) {
            icalproperty_add_parameter(property, icalparameter_new_related(ICAL_RELATED_END));
        }
        icalcomponent_add_property(component, property);
        appendExtensions(component, alarm);
        appendExtension(component, EnabledProperty, alarm.enabled() ? QStringLiteral("TRUE") : QStringLiteral("FALSE"));
        return component;
    }

    icalcomponent *writeEvent(const CalEvent &event)
    {
        for (auto zone = event.m_timeZoneComponents.cbegin(); zone != event.m_timeZoneComponents.cend(); ++zone) {
            customZones.insert(zone.key(), zone.value());
        }
        auto component = icalcomponent_new_vevent();
        if (!event.m_organizerProperty.isEmpty()) {
            appendRaw(component, event.m_organizerProperty);
        } else {
            addContact(component, ICAL_ORGANIZER_PROPERTY, event.organizer());
        }
        const auto stamp = event.lastModified().isValid() ? event.lastModified() : event.created();
        if (stamp.isValid()) {
            icalcomponent_add_property(component, icalproperty_new_dtstamp(fieldsAt(stamp, false, true)));
        }
        for (const auto &property : event.m_attendeeProperties) {
            appendRaw(component, property);
        }
        for (auto i = event.m_parsedAttendeeCount; i < event.attendees().size(); ++i) {
            const auto &person = event.attendees().at(i);
            addContact(component, ICAL_ATTENDEE_PROPERTY, person, &person);
        }
        appendExtensions(component, event);
        if (event.created().isValid()) {
            icalcomponent_add_property(component, icalproperty_new_created(fieldsAt(event.created(), false, true)));
        }
        const auto id = event.schedulingID();
        if (id != event.uid()) {
            appendExtension(component, SchedulingIdProperty, event.uid());
        }
        const QByteArray uid = bytes(id);
        icalcomponent_add_property(component, icalproperty_new_uid(uid.constData()));
        if (event.revision() != 0) {
            icalcomponent_add_property(component, icalproperty_new_sequence(event.revision()));
        }
        if (event.lastModified().isValid()) {
            icalcomponent_add_property(component, icalproperty_new_lastmodified(fieldsAt(event.lastModified(), false, true)));
        }
        addText(component, ICAL_DESCRIPTION_PROPERTY, event.description(), event.m_descriptionRich);
        addText(component, ICAL_SUMMARY_PROPERTY, event.summary(), event.m_summaryRich);
        addText(component, ICAL_LOCATION_PROPERTY, event.location(), event.m_locationRich);
        if (event.hasRecurrenceId()) {
            auto property = timeProperty(ICAL_RECURRENCEID_PROPERTY, event.recurrenceId(), event.m_recurrenceIdDateOnly);
            if (event.m_thisAndFuture) {
                icalproperty_add_parameter(property, icalparameter_new_range(ICAL_RANGE_THISANDFUTURE));
            }
            icalcomponent_add_property(component, property);
        }
        const auto recurrence = event.recurrence();
        for (const auto &rule : recurrence->m_rules) {
            appendRaw(component, QByteArray("RRULE:") + bytes(rule->rrule()));
        }
        for (const auto &rule : recurrence->m_exceptionRules) {
            appendRaw(component, QByteArray("EXRULE:") + bytes(rule->rrule()));
        }
        for (const auto &date : recurrence->exDates()) {
            icalcomponent_add_property(component, timeProperty(ICAL_EXDATE_PROPERTY, QDateTime(date, QTime(0, 0)), true));
        }
        for (const auto &time : recurrence->exDateTimes()) {
            icalcomponent_add_property(component, timeProperty(ICAL_EXDATE_PROPERTY, time, false));
        }
        for (const auto &date : recurrence->rDates()) {
            icalcomponent_add_property(component, timeProperty(ICAL_RDATE_PROPERTY, QDateTime(date, QTime(0, 0)), true));
        }
        for (const auto &time : recurrence->rDateTimes()) {
            icalcomponent_add_property(component, timeProperty(ICAL_RDATE_PROPERTY, time, false));
        }
        for (const auto &property : event.m_extraProperties) {
            appendRaw(component, property);
        }
        if (event.cancelled()) {
            icalcomponent_set_status(component, ICAL_STATUS_CANCELLED);
        }
        if (event.dtStart().isValid()) {
            icalcomponent_add_property(component, timeProperty(ICAL_DTSTART_PROPERTY, event.dtStart(), event.allDay()));
        }
        if (event.m_hasDuration) {
            icalcomponent_add_property(component, icalproperty_new_duration(toDuration(event.duration())));
        } else if (event.dtEnd().isValid() && (event.allDay() || event.dtEnd() != event.dtStart())) {
            const auto end = event.allDay() ? event.dtEnd().addDays(1) : event.dtEnd();
            icalcomponent_add_property(component, timeProperty(ICAL_DTEND_PROPERTY, end, event.allDay()));
        }
        if (!icalcomponent_get_first_property(component, ICAL_TRANSP_PROPERTY)) {
            icalcomponent_add_property(component, icalproperty_new_transp(ICAL_TRANSP_OPAQUE));
        }
        if (event.lunar()) {
            appendExtension(component, LunarProperty, QStringLiteral("TRUE"));
        }
        for (const auto &alarm : event.alarms()) {
            icalcomponent_add_component(component, writeAlarm(*alarm));
        }
        return component;
    }

    static icalcomponent *writeZone(const QByteArray &id, const QTimeZone &zone)
    {
        auto builtin = icaltimezone_get_builtin_timezone(id.constData());
        auto source = builtin ? icaltimezone_get_component(builtin) : nullptr;
        if (source) {
            auto clone = icalcomponent_new_clone(source);
            auto property = icalcomponent_get_first_property(clone, ICAL_TZID_PROPERTY);
            if (property) {
                icalproperty_set_tzid(property, id.constData());
            }
            return clone;
        }
        auto component = icalcomponent_new_vtimezone();
        icalcomponent_add_property(component, icalproperty_new_tzid(id.constData()));
        auto standard = icalcomponent_new_xstandard();
        const QDateTime epoch(QDate(1970, 1, 1), QTime(0, 0), Qt::UTC);
        const int offset = zone.offsetFromUtc(epoch);
        icalcomponent_add_property(standard, icalproperty_new_dtstart(fieldsAt(epoch, false, false)));
        icalcomponent_add_property(standard, icalproperty_new_tzoffsetfrom(offset));
        icalcomponent_add_property(standard, icalproperty_new_tzoffsetto(offset));
        icalcomponent_add_component(component, standard);
        return component;
    }
};

bool IcsCodec::fromString(const CalStore::Ptr &calendar, const QString &input, bool deleted,
                          QString *error, ReadMode mode) const
{
    const auto fail = [error](const QString &reason) {
        if (error) {
            *error = reason;
        }
        return false;
    };
    if (error) {
        error->clear();
    }
    if (!calendar) {
        return fail(QStringLiteral("Missing calendar store"));
    }
    if (input.size() > MaxIcsBytes) {
        return fail(QStringLiteral("iCalendar input exceeds 32 MiB"));
    }
    const QByteArray utf8 = bytes(input);
    if (utf8.size() > MaxIcsBytes || utf8.contains('\0')) {
        return fail(QStringLiteral("iCalendar input exceeds 32 MiB or contains NUL"));
    }
    if (input.trimmed().isEmpty()) {
        return fail(QStringLiteral("iCalendar input is empty"));
    }
    Component root(icalcomponent_new_from_string(utf8.constData()), icalcomponent_free);
    if (!root) {
        return fail(QStringLiteral("Unable to parse iCalendar input"));
    }
    CalStore parsed = *calendar;
    int duplicateEvents = 0;
    const auto read = [&](icalcomponent *component) {
        Reader reader;
        reader.mode = mode;
        const auto zone = calendar->timeZone();
        reader.floatingZone = zone.isValid() ? zone : QTimeZone::systemTimeZone();
        if (!reader.readCalendar(component, parsed, deleted)) {
            return fail(reader.diagnostic);
        }
        duplicateEvents += reader.duplicateEvents;
        return true;
    };
    if (icalcomponent_isa(root.get()) == ICAL_VCALENDAR_COMPONENT) {
        if (!read(root.get())) {
            return false;
        }
    } else if (icalcomponent_isa(root.get()) == ICAL_XROOT_COMPONENT) {
        bool found = false;
        for (auto component = icalcomponent_get_first_component(root.get(), ICAL_VCALENDAR_COMPONENT); component;
             component = icalcomponent_get_next_component(root.get(), ICAL_VCALENDAR_COMPONENT)) {
            if (!read(component)) {
                return false;
            }
            found = true;
        }
        if (!found) {
            return fail(QStringLiteral("Missing VCALENDAR component"));
        }
    } else {
        return fail(QStringLiteral("Missing VCALENDAR component"));
    }
    QStringList warnings;
    const int parseErrors = icalcomponent_count_errors(root.get());
    if (parseErrors > 0) {
        warnings.append(QStringLiteral("libical ignored %1 malformed iCalendar properties or components").arg(parseErrors));
    }
    if (duplicateEvents > 0) {
        warnings.append(QStringLiteral("Ignored %1 duplicate events with equal or older revisions").arg(duplicateEvents));
    }
    if (!warnings.isEmpty()) {
        const auto warning = warnings.join(QStringLiteral(". "));
        if (error) {
            *error = warning;
        }
        qWarning().noquote() << warning;
    }
    *calendar = std::move(parsed);
    return true;
}

QString IcsCodec::toString(const CalStore::Ptr &calendar) const
{
    if (!calendar) {
        return {};
    }
    Component output(icalcomponent_new_vcalendar(), icalcomponent_free);
    icalcomponent_add_property(output.get(), icalproperty_new_prodid("-//UnionTech Software Technology Co., Ltd.//NONSGML DDE Calendar 1.0//EN"));
    icalcomponent_add_property(output.get(), icalproperty_new_version("2.0"));
    appendExtension(output.get(), VersionProperty, QStringLiteral("1.0"));
    appendExtensions(output.get(), *calendar);

    Writer writer;
    QList<icalcomponent *> events;
    for (const auto &event : calendar->events()) {
        events.append(writer.writeEvent(*event));
    }
    for (auto entry = writer.customZones.cbegin(); entry != writer.customZones.cend(); ++entry) {
        Component zone(icalcomponent_new_from_string(entry.value().constData()), icalcomponent_free);
        if (zone) {
            icalcomponent_add_component(output.get(), zone.release());
        }
    }
    for (auto entry = writer.zones.cbegin(); entry != writer.zones.cend(); ++entry) {
        if (writer.customZones.contains(entry.key())) {
            continue;
        }
        icalcomponent_add_component(output.get(), Writer::writeZone(entry.key(), entry.value()));
    }
    for (auto event : events) {
        icalcomponent_add_component(output.get(), event);
    }
    char *buffer = icalcomponent_as_ical_string_r(output.get());
    const auto result = QString::fromUtf8(buffer);
    icalmemory_free_buffer(buffer);
    return result;
}

bool IcsCodec::load(const CalStore::Ptr &calendar, const QString &path, QString *error) const
{
    QFile input(path);
    if (!input.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = input.errorString();
        }
        return false;
    }
    QByteArray content = input.read(MaxIcsBytes + 1);
    if (input.error() != QFileDevice::NoError || content.size() > MaxIcsBytes) {
        if (error) {
            *error = input.error() != QFileDevice::NoError
                ? input.errorString() : QStringLiteral("iCalendar file exceeds 32 MiB");
        }
        return false;
    }
    if (content.startsWith("\xEF\xBB\xBF")) {
        content.remove(0, 3);
    }
    const QString decoded = QString::fromUtf8(content);
    if (decoded.toUtf8() != content) {
        if (error) {
            *error = QStringLiteral("iCalendar file is not valid UTF-8");
        }
        return false;
    }
    return fromString(calendar, decoded, false, error);
}

bool IcsCodec::save(const CalStore::Ptr &calendar, const QString &path) const
{
    const QByteArray content = bytes(toString(calendar));
    if (content.isEmpty()) {
        return false;
    }
    QSaveFile output(path);
    if (!output.open(QIODevice::WriteOnly) || output.write(content) != content.size() || !output.flush()) {
        return false;
    }
    if (QFile::exists(path)) {
        QFile previous(path);
        QSaveFile backup(path + QLatin1Char('~'));
        if (!previous.open(QIODevice::ReadOnly) || !backup.open(QIODevice::WriteOnly)) {
            qWarning("Cannot open the previous iCalendar file or its backup");
            return false;
        }
        while (!previous.atEnd()) {
            const QByteArray chunk = previous.read(64 * 1024);
            if (chunk.isEmpty() || backup.write(chunk) != chunk.size()) {
                qWarning("Cannot copy the previous iCalendar file to its backup");
                return false;
            }
        }
        if (!backup.commit()) {
            qWarning("Cannot commit the iCalendar backup");
            return false;
        }
    }
    return output.commit();
}

bool IcsCodec::fromString(RRule *rule, const QString &text) const
{
    if (!rule || !rule->parse(text)) {
        return false;
    }
    // Legacy database and JSON migrations also read standalone rules.
    rule->bindFloatingUntil(QTimeZone::systemTimeZone());
    return true;
}

QString IcsCodec::toString(const RRule *rule) const
{
    if (!rule || rule->rrule().isEmpty()) {
        return {};
    }
    return QStringLiteral("RRULE:") + rule->rrule() + QStringLiteral("\r\n");
}
