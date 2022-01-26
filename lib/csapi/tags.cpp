/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#include "tags.h"

#include <QtCore/QStringBuilder>

using namespace Quotient;

QUrl GetRoomTagsJob::makeRequestUrl(QUrl baseUrl, const QString& userId,
                                    const QString& roomId)
{
    return BaseJob::makeRequestUrl(std::move(baseUrl),
                                   QStringLiteral("/_matrix/client/r0") % "/user/"
                                       % userId % "/rooms/" % roomId % "/tags");
}

GetRoomTagsJob::GetRoomTagsJob(const QString& userId, const QString& roomId)
    : BaseJob(HttpVerb::Get, QStringLiteral("GetRoomTagsJob"),
              QStringLiteral("/_matrix/client/r0") % "/user/" % userId
                  % "/rooms/" % roomId % "/tags")
{}

SetRoomTagJob::SetRoomTagJob(const QString& userId, const QString& roomId,
                             const QString& tag, Omittable<float> order,
                             const QVariantHash& additionalProperties)
    : BaseJob(HttpVerb::Put, QStringLiteral("SetRoomTagJob"),
              QStringLiteral("/_matrix/client/r0") % "/user/" % userId
                  % "/rooms/" % roomId % "/tags/" % tag)
{
    QJsonObject _data;
    fillJson(_data, additionalProperties);
    addParam<IfNotEmpty>(_data, QStringLiteral("order"), order);
    setRequestData(std::move(_data));
}

QUrl DeleteRoomTagJob::makeRequestUrl(QUrl baseUrl, const QString& userId,
                                      const QString& roomId, const QString& tag)
{
    return BaseJob::makeRequestUrl(std::move(baseUrl),
                                   QStringLiteral("/_matrix/client/r0")
                                       % "/user/" % userId % "/rooms/" % roomId
                                       % "/tags/" % tag);
}

DeleteRoomTagJob::DeleteRoomTagJob(const QString& userId, const QString& roomId,
                                   const QString& tag)
    : BaseJob(HttpVerb::Delete, QStringLiteral("DeleteRoomTagJob"),
              QStringLiteral("/_matrix/client/r0") % "/user/" % userId
                  % "/rooms/" % roomId % "/tags/" % tag)
{}
