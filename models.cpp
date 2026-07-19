#include "models.h"

#include <QJsonArray>

QString sensorTypeName(SensorType type)
{
    switch (type) {
    case SensorType::Wind: return QStringLiteral("风速风向");
    case SensorType::Deflection: return QStringLiteral("挠度");
    case SensorType::ExpansionJoint: return QStringLiteral("伸缩缝");
    case SensorType::CableForce: return QStringLiteral("索力");
    case SensorType::TemperatureHumidity: return QStringLiteral("温湿度");
    case SensorType::Vibration: return QStringLiteral("振动");
    case SensorType::BearingDisplacement: return QStringLiteral("支座位移");
    }
    return QStringLiteral("未知");
}

QString sensorTypeUnit(SensorType type)
{
    switch (type) {
    case SensorType::Wind: return QStringLiteral("m/s");
    case SensorType::Deflection: return QStringLiteral("mm");
    case SensorType::ExpansionJoint: return QStringLiteral("mm");
    case SensorType::CableForce: return QStringLiteral("kN");
    case SensorType::TemperatureHumidity: return QStringLiteral("℃");
    case SensorType::Vibration: return QStringLiteral("Hz");
    case SensorType::BearingDisplacement: return QStringLiteral("mm");
    }
    return QString();
}

QString sensorTypeSecondUnit(SensorType type)
{
    if (type == SensorType::Wind) {
        return QStringLiteral("°");
    }
    if (type == SensorType::TemperatureHumidity) {
        return QStringLiteral("%");
    }
    return QString();
}

QStringList sensorTypeNames()
{
    QStringList names;
    for (int i = 0; i <= static_cast<int>(SensorType::BearingDisplacement); ++i) {
        names << sensorTypeName(sensorTypeFromIndex(i));
    }
    return names;
}

SensorType sensorTypeFromIndex(int index)
{
    if (index < 0 || index > static_cast<int>(SensorType::BearingDisplacement)) {
        return SensorType::Wind;
    }
    return static_cast<SensorType>(index);
}

QJsonObject Device::toJson() const
{
    QJsonObject object;
    object["id"] = id;
    object["name"] = name;
    object["type"] = static_cast<int>(type);
    object["specification"] = specification;
    object["model"] = model;
    object["manufacturer"] = manufacturer;
    object["productionDate"] = productionDate.toString(Qt::ISODate);
    return object;
}

Device Device::fromJson(const QJsonObject &object)
{
    Device device;
    device.id = object["id"].toString();
    device.name = object["name"].toString();
    device.type = sensorTypeFromIndex(object["type"].toInt());
    device.specification = object["specification"].toString();
    device.model = object["model"].toString();
    device.manufacturer = object["manufacturer"].toString();
    device.productionDate = QDate::fromString(object["productionDate"].toString(), Qt::ISODate);
    return device;
}

QJsonObject MonitorPoint::toJson() const
{
    QJsonObject object;
    object["id"] = id;
    object["name"] = name;
    object["location"] = location;
    object["description"] = description;
    return object;
}

MonitorPoint MonitorPoint::fromJson(const QJsonObject &object)
{
    MonitorPoint point;
    point.id = object["id"].toString();
    point.name = object["name"].toString();
    point.location = object["location"].toString();
    point.description = object["description"].toString();
    return point;
}

QJsonObject Binding::toJson() const
{
    QJsonObject object;
    object["pointId"] = pointId;
    object["deviceId"] = deviceId;
    return object;
}

Binding Binding::fromJson(const QJsonObject &object)
{
    Binding binding;
    binding.pointId = object["pointId"].toString();
    binding.deviceId = object["deviceId"].toString();
    return binding;
}

QJsonObject MonitorRecord::toJson() const
{
    QJsonObject object;
    object["deviceId"] = deviceId;
    object["pointId"] = pointId;
    object["type"] = static_cast<int>(type);
    object["time"] = time.toString(Qt::ISODate);
    object["value1"] = value1;
    object["value2"] = value2;
    return object;
}

MonitorRecord MonitorRecord::fromJson(const QJsonObject &object)
{
    MonitorRecord record;
    record.deviceId = object["deviceId"].toString();
    record.pointId = object["pointId"].toString();
    record.type = sensorTypeFromIndex(object["type"].toInt());
    record.time = QDateTime::fromString(object["time"].toString(), Qt::ISODate);
    record.value1 = object["value1"].toDouble();
    record.value2 = object["value2"].toDouble();
    return record;
}
