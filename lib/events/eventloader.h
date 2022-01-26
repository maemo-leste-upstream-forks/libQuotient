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

#pragma once

#include "stateevent.h"

namespace Quotient {
namespace _impl {
    template <typename BaseEventT>
    static inline auto loadEvent(const QJsonObject& json,
                                 const QString& matrixType)
    {
        if (auto e = EventFactory<BaseEventT>::make(json, matrixType))
            return e;
        return makeEvent<BaseEventT>(unknownEventTypeId(), json);
    }
} // namespace _impl

/*! Create an event with proper type from a JSON object
 *
 * Use this factory template to detect the type from the JSON object
 * contents (the detected event type should derive from the template
 * parameter type) and create an event object of that type.
 */
template <typename BaseEventT>
inline event_ptr_tt<BaseEventT> loadEvent(const QJsonObject& fullJson)
{
    return _impl::loadEvent<BaseEventT>(fullJson, fullJson[TypeKeyL].toString());
}

/*! Create an event from a type string and content JSON
 *
 * Use this factory template to resolve the C++ type from the Matrix
 * type string in \p matrixType and create an event of that type that has
 * its content part set to \p content.
 */
template <typename BaseEventT>
inline event_ptr_tt<BaseEventT> loadEvent(const QString& matrixType,
                                          const QJsonObject& content)
{
    return _impl::loadEvent<BaseEventT>(basicEventJson(matrixType, content),
                                        matrixType);
}

/*! Create a state event from a type string, content JSON and state key
 *
 * Use this factory to resolve the C++ type from the Matrix type string
 * in \p matrixType and create a state event of that type with content part
 * set to \p content and state key set to \p stateKey (empty by default).
 */
inline StateEventPtr loadStateEvent(const QString& matrixType,
                                    const QJsonObject& content,
                                    const QString& stateKey = {})
{
    return _impl::loadEvent<StateEventBase>(
        basicStateEventJson(matrixType, content, stateKey), matrixType);
}

template <typename EventT>
struct JsonConverter<event_ptr_tt<EventT>> {
    static auto load(const QJsonValue& jv)
    {
        return loadEvent<EventT>(jv.toObject());
    }
    static auto load(const QJsonDocument& jd)
    {
        return loadEvent<EventT>(jd.object());
    }
};
} // namespace Quotient
