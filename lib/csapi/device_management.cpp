/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#include "device_management.h"

#include <QtCore/QStringBuilder>

using namespace Quotient;

QUrl GetDevicesJob::makeRequestUrl(QUrl baseUrl)
{
    return BaseJob::makeRequestUrl(std::move(baseUrl),
                                   QStringLiteral("/_matrix/client/r0")
                                       % "/devices");
}

GetDevicesJob::GetDevicesJob()
    : BaseJob(HttpVerb::Get, QStringLiteral("GetDevicesJob"),
              QStringLiteral("/_matrix/client/r0") % "/devices")
{}

QUrl GetDeviceJob::makeRequestUrl(QUrl baseUrl, const QString& deviceId)
{
    return BaseJob::makeRequestUrl(std::move(baseUrl),
                                   QStringLiteral("/_matrix/client/r0")
                                       % "/devices/" % deviceId);
}

GetDeviceJob::GetDeviceJob(const QString& deviceId)
    : BaseJob(HttpVerb::Get, QStringLiteral("GetDeviceJob"),
              QStringLiteral("/_matrix/client/r0") % "/devices/" % deviceId)
{}

UpdateDeviceJob::UpdateDeviceJob(const QString& deviceId,
                                 const QString& displayName)
    : BaseJob(HttpVerb::Put, QStringLiteral("UpdateDeviceJob"),
              QStringLiteral("/_matrix/client/r0") % "/devices/" % deviceId)
{
    QJsonObject _data;
    addParam<IfNotEmpty>(_data, QStringLiteral("display_name"), displayName);
    setRequestData(std::move(_data));
}

DeleteDeviceJob::DeleteDeviceJob(const QString& deviceId,
                                 const Omittable<AuthenticationData>& auth)
    : BaseJob(HttpVerb::Delete, QStringLiteral("DeleteDeviceJob"),
              QStringLiteral("/_matrix/client/r0") % "/devices/" % deviceId)
{
    QJsonObject _data;
    addParam<IfNotEmpty>(_data, QStringLiteral("auth"), auth);
    setRequestData(std::move(_data));
}

DeleteDevicesJob::DeleteDevicesJob(const QStringList& devices,
                                   const Omittable<AuthenticationData>& auth)
    : BaseJob(HttpVerb::Post, QStringLiteral("DeleteDevicesJob"),
              QStringLiteral("/_matrix/client/r0") % "/delete_devices")
{
    QJsonObject _data;
    addParam<>(_data, QStringLiteral("devices"), devices);
    addParam<IfNotEmpty>(_data, QStringLiteral("auth"), auth);
    setRequestData(std::move(_data));
}
