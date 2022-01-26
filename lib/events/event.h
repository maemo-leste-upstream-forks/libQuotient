/******************************************************************************
 * Copyright (C) 2015 Felix Rohrbach <kde@fxrh.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#pragma once

#include "converters.h"
#include "logging.h"

#ifdef ENABLE_EVENTTYPE_ALIAS
#    define USE_EVENTTYPE_ALIAS 1
#endif

namespace Quotient {
// === event_ptr_tt<> and type casting facilities ===

template <typename EventT>
using event_ptr_tt = std::unique_ptr<EventT>;

/// Unwrap a plain pointer from a smart pointer
template <typename EventT>
inline EventT* rawPtr(const event_ptr_tt<EventT>& ptr)
{
    return ptr.get();
}

/// Unwrap a plain pointer and downcast it to the specified type
template <typename TargetEventT, typename EventT>
inline TargetEventT* weakPtrCast(const event_ptr_tt<EventT>& ptr)
{
    return static_cast<TargetEventT*>(rawPtr(ptr));
}

/// Re-wrap a smart pointer to base into a smart pointer to derived
template <typename TargetT, typename SourceT>
[[deprecated("Consider using eventCast() or visit() instead")]]
inline event_ptr_tt<TargetT> ptrCast(event_ptr_tt<SourceT>&& ptr)
{
    return std::unique_ptr<TargetT>(static_cast<TargetT*>(ptr.release()));
}

// === Standard Matrix key names and basicEventJson() ===

static const auto TypeKey = QStringLiteral("type");
static const auto BodyKey = QStringLiteral("body");
static const auto ContentKey = QStringLiteral("content");
static const auto EventIdKey = QStringLiteral("event_id");
static const auto UnsignedKey = QStringLiteral("unsigned");
static const auto StateKeyKey = QStringLiteral("state_key");
static const auto TypeKeyL = "type"_ls;
static const auto BodyKeyL = "body"_ls;
static const auto ContentKeyL = "content"_ls;
static const auto EventIdKeyL = "event_id"_ls;
static const auto UnsignedKeyL = "unsigned"_ls;
static const auto RedactedCauseKeyL = "redacted_because"_ls;
static const auto PrevContentKeyL = "prev_content"_ls;
static const auto StateKeyKeyL = "state_key"_ls;

/// Make a minimal correct Matrix event JSON
template <typename StrT>
inline QJsonObject basicEventJson(StrT matrixType, const QJsonObject& content)
{
    return { { TypeKey, std::forward<StrT>(matrixType) },
             { ContentKey, content } };
}

// === Event types and event types registry ===

using event_type_t = size_t;
using event_mtype_t = const char*;

class EventTypeRegistry {
public:
    ~EventTypeRegistry() = default;

    static event_type_t initializeTypeId(event_mtype_t matrixTypeId);

    template <typename EventT>
    static inline event_type_t initializeTypeId()
    {
        return initializeTypeId(EventT::matrixTypeId());
    }

    static QString getMatrixType(event_type_t typeId);

private:
    EventTypeRegistry() = default;
    Q_DISABLE_COPY(EventTypeRegistry)
    DISABLE_MOVE(EventTypeRegistry)

    static EventTypeRegistry& get()
    {
        static EventTypeRegistry etr;
        return etr;
    }

    std::vector<event_mtype_t> eventTypes;
};

template <>
inline event_type_t EventTypeRegistry::initializeTypeId<void>()
{
    return initializeTypeId("");
}

template <typename EventT>
struct EventTypeTraits {
    static event_type_t id()
    {
        static const auto id = EventTypeRegistry::initializeTypeId<EventT>();
        return id;
    }
};

template <typename EventT>
inline event_type_t typeId()
{
    return EventTypeTraits<std::decay_t<EventT>>::id();
}

inline event_type_t unknownEventTypeId() { return typeId<void>(); }

// === EventFactory ===

/** Create an event of arbitrary type from its arguments */
template <typename EventT, typename... ArgTs>
inline event_ptr_tt<EventT> makeEvent(ArgTs&&... args)
{
    return std::make_unique<EventT>(std::forward<ArgTs>(args)...);
}

template <typename BaseEventT>
class EventFactory {
public:
    template <typename FnT>
    static auto addMethod(FnT&& method)
    {
        factories().emplace_back(std::forward<FnT>(method));
        return 0;
    }

    /** Chain two type factories
     * Adds the factory class of EventT2 (EventT2::factory_t) to
     * the list in factory class of EventT1 (EventT1::factory_t) so
     * that when EventT1::factory_t::make() is invoked, types of
     * EventT2 factory are looked through as well. This is used
     * to include RoomEvent types into the more general Event factory,
     * and state event types into the RoomEvent factory.
     */
    template <typename EventT>
    static auto chainFactory()
    {
        return addMethod(&EventT::factory_t::make);
    }

    static event_ptr_tt<BaseEventT> make(const QJsonObject& json,
                                         const QString& matrixType)
    {
        for (const auto& f : factories())
            if (auto e = f(json, matrixType))
                return e;
        return nullptr;
    }

private:
    static auto& factories()
    {
        using inner_factory_tt = std::function<event_ptr_tt<BaseEventT>(
            const QJsonObject&, const QString&)>;
        static std::vector<inner_factory_tt> _factories {};
        return _factories;
    }
};

/** Add a type to its default factory
 * Adds a standard factory method (via makeEvent<>) for a given
 * type to EventT::factory_t factory class so that it can be
 * created dynamically from loadEvent<>().
 *
 * \tparam EventT the type to enable dynamic creation of
 * \return the registered type id
 * \sa loadEvent, Event::type
 */
template <typename EventT>
inline auto setupFactory()
{
    qDebug(EVENTS) << "Adding factory method for" << EventT::matrixTypeId();
    return EventT::factory_t::addMethod([](const QJsonObject& json,
                                           const QString& jsonMatrixType) {
        return EventT::matrixTypeId() == jsonMatrixType ? makeEvent<EventT>(json)
                                                        : nullptr;
    });
}

template <typename EventT>
inline auto registerEventType()
{
    // Initialise exactly once, even if this function is called twice for
    // the same type (for whatever reason - you never know the ways of
    // static initialisation is done).
    static const auto _ = setupFactory<EventT>();
    return _; // Only to facilitate usage in static initialisation
}

// === Event ===

class Event {
    Q_GADGET
    Q_PROPERTY(Type type READ type CONSTANT)
    Q_PROPERTY(QJsonObject contentJson READ contentJson CONSTANT)
public:
    using Type = event_type_t;
    using factory_t = EventFactory<Event>;

    explicit Event(Type type, const QJsonObject& json);
    explicit Event(Type type, event_mtype_t matrixType,
                   const QJsonObject& contentJson = {});
    Q_DISABLE_COPY(Event)
    Event(Event&&) = default;
    Event& operator=(Event&&) = delete;
    virtual ~Event();

    Type type() const { return _type; }
    QString matrixType() const;
    QByteArray originalJson() const;
    QJsonObject originalJsonObject() const { return fullJson(); }

    const QJsonObject& fullJson() const { return _json; }

    // According to the CS API spec, every event also has
    // a "content" object; but since its structure is different for
    // different types, we're implementing it per-event type.

    const QJsonObject contentJson() const;
    const QJsonObject unsignedJson() const;

    template <typename T>
    T content(const QString& key) const
    {
        return fromJson<T>(contentJson()[key]);
    }

    template <typename T>
    T content(QLatin1String key) const
    {
        return fromJson<T>(contentJson()[key]);
    }

    friend QDebug operator<<(QDebug dbg, const Event& e)
    {
        QDebugStateSaver _dss { dbg };
        dbg.noquote().nospace() << e.matrixType() << '(' << e.type() << "): ";
        e.dumpTo(dbg);
        return dbg;
    }

    virtual bool isStateEvent() const { return false; }
    virtual bool isCallEvent() const { return false; }
    virtual void dumpTo(QDebug dbg) const;

protected:
    QJsonObject& editJson() { return _json; }

private:
    Type _type;
    QJsonObject _json;
};
using EventPtr = event_ptr_tt<Event>;

template <typename EventT>
using EventsArray = std::vector<event_ptr_tt<EventT>>;
using Events = EventsArray<Event>;

// === Macros used with event class definitions ===

// This macro should be used in a public section of an event class to
// provide matrixTypeId() and typeId().
#define DEFINE_EVENT_TYPEID(_Id, _Type)                           \
    static constexpr event_mtype_t matrixTypeId() { return _Id; } \
    static auto typeId() { return Quotient::typeId<_Type>(); }    \
    // End of macro

// This macro should be put after an event class definition (in .h or .cpp)
// to enable its deserialisation from a /sync and other
// polymorphic event arrays
#define REGISTER_EVENT_TYPE(_Type)                                \
    namespace {                                                   \
        [[maybe_unused]] static const auto _factoryAdded##_Type = \
            registerEventType<_Type>();                           \
    }                                                             \
    // End of macro

// === is<>(), eventCast<>() and visit<>() ===

template <typename EventT>
inline bool is(const Event& e)
{
    return e.type() == typeId<EventT>();
}

inline bool isUnknown(const Event& e)
{
    return e.type() == unknownEventTypeId();
}

template <typename EventT, typename BasePtrT>
inline auto eventCast(const BasePtrT& eptr)
    -> decltype(static_cast<EventT*>(&*eptr))
{
    Q_ASSERT(eptr);
    return is<std::decay_t<EventT>>(*eptr) ? static_cast<EventT*>(&*eptr)
                                           : nullptr;
}

// A single generic catch-all visitor
template <typename BaseEventT, typename FnT>
inline auto visit(const BaseEventT& event, FnT&& visitor)
    -> decltype(visitor(event))
{
    return visitor(event);
}

namespace _impl {
    template <typename T, typename FnT>
    constexpr auto needs_downcast()
    {
        return !std::is_convertible_v<T, fn_arg_t<FnT>>;
    }
}

// A single type-specific void visitor
template <typename BaseEventT, typename FnT>
inline std::enable_if_t<_impl::needs_downcast<BaseEventT, FnT>()
                        && std::is_void_v<fn_return_t<FnT>>>
visit(const BaseEventT& event, FnT&& visitor)
{
    using event_type = fn_arg_t<FnT>;
    if (is<std::decay_t<event_type>>(event))
        visitor(static_cast<event_type>(event));
}

// A single type-specific non-void visitor with an optional default value
// non-voidness is guarded by defaultValue type
template <typename BaseEventT, typename FnT>
inline std::enable_if_t<_impl::needs_downcast<BaseEventT, FnT>(), fn_return_t<FnT>>
visit(const BaseEventT& event, FnT&& visitor,
      fn_return_t<FnT>&& defaultValue = {})
{
    using event_type = fn_arg_t<FnT>;
    if (is<std::decay_t<event_type>>(event))
        return visitor(static_cast<event_type>(event));
    return std::forward<fn_return_t<FnT>>(defaultValue);
}

// A chain of 2 or more visitors
template <typename BaseEventT, typename FnT1, typename FnT2, typename... FnTs>
inline fn_return_t<FnT1> visit(const BaseEventT& event, FnT1&& visitor1,
                               FnT2&& visitor2, FnTs&&... visitors)
{
    using event_type1 = fn_arg_t<FnT1>;
    if (is<std::decay_t<event_type1>>(event))
        return visitor1(static_cast<event_type1&>(event));
    return visit(event, std::forward<FnT2>(visitor2),
                 std::forward<FnTs>(visitors)...);
}

// A facility overload that calls void-returning visit() on each event
// over a range of event pointers
template <typename RangeT, typename... FnTs>
inline auto visitEach(RangeT&& events, FnTs&&... visitors)
    -> std::enable_if_t<std::is_convertible_v<
                            std::decay_t<decltype(**events.begin())>, Event>>
{
    for (auto&& evtPtr: events)
        visit(*evtPtr, std::forward<FnTs>(visitors)...);
}
} // namespace Quotient
Q_DECLARE_METATYPE(Quotient::Event*)
Q_DECLARE_METATYPE(const Quotient::Event*)
