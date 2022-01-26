/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#include "room_send.h"

#include <QtCore/QStringBuilder>

using namespace Quotient;

SendMessageJob::SendMessageJob(const QString& roomId, const QString& eventType,
                               const QString& txnId, const QJsonObject& body)
    : BaseJob(HttpVerb::Put, QStringLiteral("SendMessageJob"),
              QStringLiteral("/_matrix/client/r0") % "/rooms/" % roomId
                  % "/send/" % eventType % "/" % txnId)
{
    setRequestData(Data(toJson(body)));
    addExpectedKey("event_id");
}
