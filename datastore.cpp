#include "datastore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QRegularExpression>
#include <QtMath>

#include <algorithm>
#include <cmath>

namespace {

struct PointInfo {
    QString name;
    QString typeName;
};

struct SensorFileSpec {
    QString fileName;
    SensorType type = SensorType::Wind;
    bool pairedValues = false;
};

QString cleanCell(QString value)
{
    return value.remove(QChar(0xFEFF)).trimmed();
}

QString baseSensorCode(QString header, bool pairedValues)
{
    header = cleanCell(header);
    if (pairedValues && (header.endsWith(QStringLiteral("-A")) || header.endsWith(QStringLiteral("-B")))) {
        header.chop(2);
    }
    return header;
}

QDateTime parseMonitorTime(const QString &text)
{
    const QString value = cleanCell(text);
    const QStringList formats = {
        QStringLiteral("yyyy/M/d H:m"),
        QStringLiteral("yyyy/M/d H:mm"),
        QStringLiteral("yyyy/M/d HH:mm"),
        QStringLiteral("yyyy-MM-dd H:m"),
        QStringLiteral("yyyy-MM-dd HH:mm:ss")
    };

    for (const QString &format : formats) {
        const QDateTime dateTime = QDateTime::fromString(value, format);
        if (dateTime.isValid()) {
            return dateTime;
        }
    }
    return QDateTime();
}

QString findDataSourceDir()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString currentDir = QDir::currentPath();
    const QStringList candidates = {
        appDir + QStringLiteral("/data_sources"),
        appDir + QStringLiteral("/../data_sources"),
        appDir + QStringLiteral("/../../data_sources"),
        currentDir + QStringLiteral("/data_sources"),
        currentDir + QStringLiteral("/BridgeMonitor/data_sources"),
        currentDir + QStringLiteral("/../BridgeMonitor/data_sources"),
        currentDir + QStringLiteral("/../../BridgeMonitor/data_sources")
    };

    for (const QString &candidate : candidates) {
        QDir dir(candidate);
        if (dir.exists(QStringLiteral("monitor_points.tsv"))) {
            return dir.absolutePath();
        }
    }
    return QString();
}

QMap<QString, PointInfo> readPointInfoMap(const QString &sourceDir)
{
    QMap<QString, PointInfo> pointInfo;
    QFile file(QDir(sourceDir).filePath(QStringLiteral("monitor_points.tsv")));
    if (!file.open(QIODevice::ReadOnly)) {
        return pointInfo;
    }

    const QString text = QString::fromUtf8(file.readAll());
    const QStringList lines = text.split(QRegularExpression(QStringLiteral("\\r?\\n")), Qt::SkipEmptyParts);
    for (int i = 1; i < lines.size(); ++i) {
        const QStringList cells = lines.at(i).split(QLatin1Char('\t'));
        if (cells.size() < 3) {
            continue;
        }

        const QString code = cleanCell(cells.at(2));
        if (code.isEmpty()) {
            continue;
        }
        pointInfo[code] = PointInfo{cleanCell(cells.at(0)), cleanCell(cells.at(1))};
    }
    return pointInfo;
}

void addDevicePointAndBinding(DataStore *store, const QString &code, SensorType type,
                              const QMap<QString, PointInfo> &pointInfo,
                              const QString &sourceFileName)
{
    if (!store->deviceById(code)) {
        Device device;
        device.id = code;
        device.name = sensorTypeName(type) + QStringLiteral("装置-") + code;
        device.type = type;
        device.specification = sensorTypeUnit(type).isEmpty()
            ? QStringLiteral("桥梁监测")
            : QStringLiteral("量程单位：") + sensorTypeUnit(type);
        if (!sensorTypeSecondUnit(type).isEmpty()) {
            device.specification += QStringLiteral("/") + sensorTypeSecondUnit(type);
        }
        device.model = code;
        device.manufacturer = QStringLiteral("万州三桥监测系统");
        device.productionDate = QDate(2023, 4, 21);
        store->devices << device;
    }

    if (!store->pointById(code)) {
        MonitorPoint point;
        point.id = code;
        point.name = pointInfo.contains(code) && !pointInfo.value(code).name.isEmpty()
            ? pointInfo.value(code).name
            : sensorTypeName(type) + QStringLiteral("测点-") + code;
        point.location = QStringLiteral("万州三桥 ") + point.name;
        point.description = QStringLiteral("来源文件：") + sourceFileName
            + QStringLiteral("；监测类型：") + sensorTypeName(type);
        store->points << point;
    }

    for (const Binding &binding : store->bindings) {
        if (binding.pointId == code && binding.deviceId == code) {
            return;
        }
    }
    store->bindings << Binding{code, code};
}

bool importSensorFile(DataStore *store, const QString &sourceDir, const SensorFileSpec &spec,
                      const QMap<QString, PointInfo> &pointInfo)
{
    QFile file(QDir(sourceDir).filePath(spec.fileName));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QString text = QString::fromLocal8Bit(file.readAll());
    const QStringList lines = text.split(QRegularExpression(QStringLiteral("\\r?\\n")), Qt::SkipEmptyParts);
    if (lines.size() < 2) {
        return false;
    }

    const QStringList headers = lines.first().split(QLatin1Char('\t'));
    if (headers.size() < 2) {
        return false;
    }

    QMap<QString, int> value1Columns;
    QMap<QString, int> value2Columns;
    for (int col = 1; col < headers.size(); ++col) {
        const QString header = cleanCell(headers.at(col));
        const QString code = baseSensorCode(header, spec.pairedValues);
        if (code.isEmpty()) {
            continue;
        }

        addDevicePointAndBinding(store, code, spec.type, pointInfo, spec.fileName);
        if (spec.pairedValues && header.endsWith(QStringLiteral("-B"))) {
            value2Columns[code] = col;
        } else {
            value1Columns[code] = col;
        }
    }

    for (int row = 1; row < lines.size(); ++row) {
        const QStringList cells = lines.at(row).split(QLatin1Char('\t'));
        if (cells.isEmpty()) {
            continue;
        }

        const QDateTime time = parseMonitorTime(cells.first());
        if (!time.isValid()) {
            continue;
        }

        for (auto it = value1Columns.constBegin(); it != value1Columns.constEnd(); ++it) {
            const int col1 = it.value();
            if (col1 >= cells.size()) {
                continue;
            }

            bool ok1 = false;
            const double value1 = cleanCell(cells.at(col1)).toDouble(&ok1);
            if (!ok1) {
                continue;
            }

            MonitorRecord record;
            record.deviceId = it.key();
            record.pointId = it.key();
            record.type = spec.type;
            record.time = time;
            record.value1 = value1;
            record.value2 = 0.0;

            if (spec.pairedValues && value2Columns.contains(it.key())) {
                const int col2 = value2Columns.value(it.key());
                if (col2 < cells.size()) {
                    bool ok2 = false;
                    const double value2 = cleanCell(cells.at(col2)).toDouble(&ok2);
                    if (ok2) {
                        record.value2 = value2;
                    }
                }
            }
            store->records << record;
        }
    }
    return true;
}

} // namespace

DataStore::DataStore(const QString &filePath)
    : m_filePath(filePath)
{
}

bool DataStore::load()
{
    QFile file(m_filePath);
    if (!file.exists()) {
        ensureSampleData();
        return save();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    const QJsonObject root = document.object();

    if (root["source"].toString() != QStringLiteral("real-monitoring-files")
        && !findDataSourceDir().isEmpty()) {
        ensureSampleData();
        return save();
    }

    devices.clear();
    points.clear();
    bindings.clear();
    records.clear();

    for (const QJsonValue &value : root["devices"].toArray()) {
        devices << Device::fromJson(value.toObject());
    }
    for (const QJsonValue &value : root["points"].toArray()) {
        points << MonitorPoint::fromJson(value.toObject());
    }
    for (const QJsonValue &value : root["bindings"].toArray()) {
        bindings << Binding::fromJson(value.toObject());
    }
    for (const QJsonValue &value : root["records"].toArray()) {
        records << MonitorRecord::fromJson(value.toObject());
    }

    if (devices.isEmpty() || points.isEmpty() || records.isEmpty()) {
        ensureSampleData();
        return save();
    }
    return true;
}

bool DataStore::save() const
{
    QDir().mkpath(QFileInfo(m_filePath).absolutePath());
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    QJsonArray deviceArray;
    QJsonArray pointArray;
    QJsonArray bindingArray;
    QJsonArray recordArray;
    for (const Device &device : devices) {
        deviceArray.append(device.toJson());
    }
    for (const MonitorPoint &point : points) {
        pointArray.append(point.toJson());
    }
    for (const Binding &binding : bindings) {
        bindingArray.append(binding.toJson());
    }
    for (const MonitorRecord &record : records) {
        recordArray.append(record.toJson());
    }

    QJsonObject root;
    root["source"] = QStringLiteral("real-monitoring-files");
    root["devices"] = deviceArray;
    root["points"] = pointArray;
    root["bindings"] = bindingArray;
    root["records"] = recordArray;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

void DataStore::ensureSampleData()
{
    devices.clear();
    points.clear();
    bindings.clear();
    records.clear();

    if (loadRealMonitoringData()) {
        return;
    }

    const QList<SensorType> types = {
        SensorType::Wind, SensorType::Deflection, SensorType::ExpansionJoint,
        SensorType::CableForce, SensorType::TemperatureHumidity, SensorType::Vibration,
        SensorType::BearingDisplacement
    };

    int index = 1;
    for (SensorType type : types) {
        Device device;
        device.id = QString("D%1").arg(index, 3, 10, QLatin1Char('0'));
        device.name = sensorTypeName(type) + QStringLiteral("监测装置");
        device.type = type;
        device.specification = QStringLiteral("工业级");
        device.model = QString("WM-%1").arg(index);
        device.manufacturer = QStringLiteral("重庆桥安智能设备有限公司");
        device.productionDate = QDate(2025, 3, qMin(index + 3, 28));
        devices << device;

        MonitorPoint point;
        point.id = QString("P%1").arg(index, 3, 10, QLatin1Char('0'));
        point.name = sensorTypeName(type) + QStringLiteral("测点");
        point.location = QStringLiteral("万州三桥主梁/桥塔关键位置 %1").arg(index);
        point.description = QStringLiteral("用于采集") + sensorTypeName(type) + QStringLiteral("状态数据");
        points << point;

        bindings << Binding{point.id, device.id};

        const QDateTime start(QDate::currentDate().addDays(-6), QTime(8, 0));
        for (int day = 0; day < 7; ++day) {
            for (int hour = 0; hour < 24; hour += 2) {
                MonitorRecord record;
                record.deviceId = device.id;
                record.pointId = point.id;
                record.type = type;
                record.time = start.addDays(day).addSecs(hour * 3600);
                const double wave = qSin((day * 12 + hour) / 4.0);
                switch (type) {
                case SensorType::Wind:
                    record.value1 = 6.0 + wave * 2.4 + hour * 0.05;
                    record.value2 = std::fmod(90.0 + day * 18.0 + hour * 7.0, 360.0);
                    break;
                case SensorType::Deflection:
                    record.value1 = 12.0 + wave * 1.8;
                    break;
                case SensorType::ExpansionJoint:
                    record.value1 = 20.0 + day * 0.3 + wave * 2.0;
                    break;
                case SensorType::CableForce:
                    record.value1 = 820.0 + wave * 35.0 + day * 2.0;
                    break;
                case SensorType::TemperatureHumidity:
                    record.value1 = 24.0 + wave * 5.0;
                    record.value2 = 63.0 + qCos(hour / 5.0) * 9.0;
                    break;
                case SensorType::Vibration:
                    record.value1 = 3.2 + wave * 0.4;
                    break;
                case SensorType::BearingDisplacement:
                    record.value1 = 4.8 + wave * 0.9;
                    break;
                }
                records << record;
            }
        }
        ++index;
    }
}

bool DataStore::loadRealMonitoringData()
{
    const QString sourceDir = findDataSourceDir();
    if (sourceDir.isEmpty()) {
        return false;
    }

    const QMap<QString, PointInfo> pointInfo = readPointInfoMap(sourceDir);
    const QList<SensorFileSpec> files = {
        {QStringLiteral("风速风向2023-04-21_10-09-51.xls"), SensorType::Wind, true},
        {QStringLiteral("挠度2023-04-21_10-09-24.xls"), SensorType::Deflection, false},
        {QStringLiteral("伸缩缝2023-04-21_10-19-50.xls"), SensorType::ExpansionJoint, false},
        {QStringLiteral("索力2023-04-21_10-11-01.xls"), SensorType::CableForce, false},
        {QStringLiteral("温湿度2023-04-21_10-09-05.xls"), SensorType::TemperatureHumidity, true},
        {QStringLiteral("振动2023-04-21_10-12-43.xls"), SensorType::Vibration, false},
        {QStringLiteral("支座位移2023-04-21_10-10-17.xls"), SensorType::BearingDisplacement, false}
    };

    int importedFiles = 0;
    for (const SensorFileSpec &spec : files) {
        if (importSensorFile(this, sourceDir, spec, pointInfo)) {
            ++importedFiles;
        }
    }

    if (importedFiles == 0 || records.isEmpty()) {
        devices.clear();
        points.clear();
        bindings.clear();
        records.clear();
        return false;
    }
    return true;
}

QString DataStore::nextDeviceId() const
{
    return QString("D%1").arg(devices.size() + 1, 3, 10, QLatin1Char('0'));
}

QString DataStore::nextPointId() const
{
    return QString("P%1").arg(points.size() + 1, 3, 10, QLatin1Char('0'));
}

Device *DataStore::deviceById(const QString &id)
{
    for (Device &device : devices) {
        if (device.id == id) {
            return &device;
        }
    }
    return nullptr;
}

MonitorPoint *DataStore::pointById(const QString &id)
{
    for (MonitorPoint &point : points) {
        if (point.id == id) {
            return &point;
        }
    }
    return nullptr;
}

const Device *DataStore::deviceById(const QString &id) const
{
    for (const Device &device : devices) {
        if (device.id == id) {
            return &device;
        }
    }
    return nullptr;
}

const MonitorPoint *DataStore::pointById(const QString &id) const
{
    for (const MonitorPoint &point : points) {
        if (point.id == id) {
            return &point;
        }
    }
    return nullptr;
}

bool DataStore::isDeviceBound(const QString &deviceId) const
{
    for (const Binding &binding : bindings) {
        if (binding.deviceId == deviceId) {
            return true;
        }
    }
    return false;
}

bool DataStore::isPointBound(const QString &pointId) const
{
    for (const Binding &binding : bindings) {
        if (binding.pointId == pointId) {
            return true;
        }
    }
    return false;
}

QList<MonitorRecord> DataStore::queryRecords(const QString &pointId, const QString &deviceId,
                                             const QDateTime &from, const QDateTime &to) const
{
    QList<MonitorRecord> result;
    for (const MonitorRecord &record : records) {
        if (!pointId.isEmpty() && record.pointId != pointId) {
            continue;
        }
        if (!deviceId.isEmpty() && record.deviceId != deviceId) {
            continue;
        }
        if (record.time < from || record.time > to) {
            continue;
        }
        result << record;
    }
    std::sort(result.begin(), result.end(), [](const MonitorRecord &left, const MonitorRecord &right) {
        return left.time < right.time;
    });
    return result;
}
