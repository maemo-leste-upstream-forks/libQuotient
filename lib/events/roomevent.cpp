/******************************************************************************
 * Copyright (C) 2018 Kitsune Ral <kitsune-ral@users.sf.net>
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

#include "roomevent.h"

#include "converters.h"
#include "logging.h"
#include "redactionevent.h"

using namespace Quotient;

[[maybe_unused]] static auto roomEventTypeInitialised =
    Event::factory_t::chainFactory<RoomEvent>();

RoomEvent::RoomEvent(Type type, event_mtype_t matrixType,
                     const QJsonObject& contentJson)
    : Event(type, matrixType, contentJson)
{}

RoomEvent::RoomEvent(Type type, const QJsonObject& json) : Event(type, json)
{
    const auto unsignedData = json[UnsignedKeyL].toObject();
    const auto redaction = unsignedData[RedactedCauseKeyL];
    if (redaction.isObject())
        _redactedBecause = makeEvent<RedactionEvent>(redaction.toObject());
}

RoomEvent::~RoomEvent() = default; // Let the smart pointer do its job

QString RoomEvent::id() const { return fullJson()[EventIdKeyL].toString(); }

QDateTime RoomEvent::originTimestamp() const
{
    return Quotient::fromJson<QDateTime>(fullJson()["origin_server_ts"_ls]);
}

QString RoomEvent::roomId() const
{
    return fullJson()["room_id"_ls].toString();
}

QString RoomEvent::senderId() const
{
    return fullJson()["sender"_ls].toString();
}

bool RoomEvent::isReplaced() const
{
    return unsignedJson()["m.relations"_ls].toObject().contains("m.replace");
}

QString RoomEvent::replacedBy() const
{
    // clang-format off
    return unsignedJson()["m.relations"_ls].toObject()
            .value("m.replace").toObject()
            .value(EventIdKeyL).toString();
    // clang-format on
}

QString RoomEvent::redactionReason() const
{
    return isRedacted() ? _redactedBecause->reason() : QString {};
}

QString RoomEvent::transactionId() const
{
    return unsignedJson()["transaction_id"_ls].toString();
}

QString RoomEvent::stateKey() const
{
    return fullJson()[StateKeyKeyL].toString();
}

void RoomEvent::setRoomId(const QString& roomId)
{
    editJson().insert(QStringLiteral("room_id"), roomId);
}

void RoomEvent::setSender(const QString& senderId)
{
    editJson().insert(QStringLiteral("sender"), senderId);
}

void RoomEvent::setTransactionId(const QString& txnId)
{
    auto unsignedData = fullJson()[UnsignedKeyL].toObject();
    unsignedData.insert(QStringLiteral("transaction_id"), txnId);
    editJson().insert(UnsignedKey, unsignedData);
    Q_ASSERT(transactionId() == txnId);
}

void RoomEvent::addId(const QString& newId)
{
    Q_ASSERT(id().isEmpty());
    Q_ASSERT(!newId.isEmpty());
    editJson().insert(EventIdKey, newId);
    qCDebug(EVENTS) << "Event txnId -> id:" << transactionId() << "->" << id();
    Q_ASSERT(id() == newId);
}

QJsonObject makeCallContentJson(const QString& callId, int version,
                                QJsonObject content)
{
    content.insert(QStringLiteral("call_id"), callId);
    content.insert(QStringLiteral("version"), version);
    return content;
}

CallEventBase::CallEventBase(Type type, event_mtype_t matrixType,
                             const QString& callId, int version,
                             const QJsonObject& contentJson)
    : RoomEvent(type, matrixType,
                makeCallContentJson(callId, version, contentJson))
{}

CallEventBase::CallEventBase(Event::Type type, const QJsonObject& json)
    : RoomEvent(type, json)
{
    if (callId().isEmpty())
        qCWarning(EVENTS) << id() << "is a call event with an empty call id";
}
