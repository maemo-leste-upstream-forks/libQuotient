/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#include "appservice_room_directory.h"

#include <QtCore/QStringBuilder>

using namespace Quotient;

UpdateAppserviceRoomDirectoryVsibilityJob::UpdateAppserviceRoomDirectoryVsibilityJob(
    const QString& networkId, const QString& roomId, const QString& visibility)
    : BaseJob(HttpVerb::Put,
              QStringLiteral("UpdateAppserviceRoomDirectoryVsibilityJob"),
              QStringLiteral("/_matrix/client/r0")
                  % "/directory/list/appservice/" % networkId % "/" % roomId)
{
    QJsonObject _data;
    addParam<>(_data, QStringLiteral("visibility"), visibility);
    setRequestData(std::move(_data));
}
