/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#pragma once

#include "converters.h"

namespace Quotient {

struct ThirdPartyUser {
    /// A Matrix User ID represting a third party user.
    QString userid;

    /// The protocol ID that the third party location is a part of.
    QString protocol;

    /// Information used to identify this third party location.
    QJsonObject fields;
};

template <>
struct JsonObjectConverter<ThirdPartyUser> {
    static void dumpTo(QJsonObject& jo, const ThirdPartyUser& pod)
    {
        addParam<>(jo, QStringLiteral("userid"), pod.userid);
        addParam<>(jo, QStringLiteral("protocol"), pod.protocol);
        addParam<>(jo, QStringLiteral("fields"), pod.fields);
    }
    static void fillFrom(const QJsonObject& jo, ThirdPartyUser& pod)
    {
        fromJson(jo.value("userid"_ls), pod.userid);
        fromJson(jo.value("protocol"_ls), pod.protocol);
        fromJson(jo.value("fields"_ls), pod.fields);
    }
};

} // namespace Quotient
