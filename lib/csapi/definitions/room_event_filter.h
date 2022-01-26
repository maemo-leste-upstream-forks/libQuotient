/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#pragma once

#include "converters.h"

#include "csapi/definitions/event_filter.h"

namespace Quotient {

struct RoomEventFilter : EventFilter {
    /// If ``true``, enables lazy-loading of membership events. See
    /// `Lazy-loading room members <#lazy-loading-room-members>`_
    /// for more information. Defaults to ``false``.
    Omittable<bool> lazyLoadMembers;

    /// If ``true``, sends all membership events for all events, even if they
    /// have already been sent to the client. Does not apply unless
    /// ``lazy_load_members`` is ``true``. See `Lazy-loading room members
    /// <#lazy-loading-room-members>`_ for more information. Defaults to
    /// ``false``.
    Omittable<bool> includeRedundantMembers;

    /// A list of room IDs to exclude. If this list is absent then no rooms are
    /// excluded. A matching room will be excluded even if it is listed in the
    /// ``'rooms'`` filter.
    QStringList notRooms;

    /// A list of room IDs to include. If this list is absent then all rooms are
    /// included.
    QStringList rooms;

    /// If ``true``, includes only events with a ``url`` key in their content.
    /// If ``false``, excludes those events. If omitted, ``url`` key is not
    /// considered for filtering.
    Omittable<bool> containsUrl;
};

template <>
struct JsonObjectConverter<RoomEventFilter> {
    static void dumpTo(QJsonObject& jo, const RoomEventFilter& pod)
    {
        fillJson<EventFilter>(jo, pod);
        addParam<IfNotEmpty>(jo, QStringLiteral("lazy_load_members"),
                             pod.lazyLoadMembers);
        addParam<IfNotEmpty>(jo, QStringLiteral("include_redundant_members"),
                             pod.includeRedundantMembers);
        addParam<IfNotEmpty>(jo, QStringLiteral("not_rooms"), pod.notRooms);
        addParam<IfNotEmpty>(jo, QStringLiteral("rooms"), pod.rooms);
        addParam<IfNotEmpty>(jo, QStringLiteral("contains_url"),
                             pod.containsUrl);
    }
    static void fillFrom(const QJsonObject& jo, RoomEventFilter& pod)
    {
        fillFromJson<EventFilter>(jo, pod);
        fromJson(jo.value("lazy_load_members"_ls), pod.lazyLoadMembers);
        fromJson(jo.value("include_redundant_members"_ls),
                 pod.includeRedundantMembers);
        fromJson(jo.value("not_rooms"_ls), pod.notRooms);
        fromJson(jo.value("rooms"_ls), pod.rooms);
        fromJson(jo.value("contains_url"_ls), pod.containsUrl);
    }
};

} // namespace Quotient
