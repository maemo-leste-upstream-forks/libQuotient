/******************************************************************************
 * Copyright (C) 2019 QMatrixClient project
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

#include "stateevent.h"

namespace Quotient {
class RoomTombstoneEvent : public StateEventBase {
public:
    DEFINE_EVENT_TYPEID("m.room.tombstone", RoomTombstoneEvent)

    explicit RoomTombstoneEvent() : StateEventBase(typeId(), matrixTypeId()) {}
    explicit RoomTombstoneEvent(const QJsonObject& obj)
        : StateEventBase(typeId(), obj)
    {}

    QString serverMessage() const;
    QString successorRoomId() const;
};
REGISTER_EVENT_TYPE(RoomTombstoneEvent)
} // namespace Quotient
