#ifndef DATASTORE_H
#define DATASTORE_H

#include "models.h"

#include <QDateTime>
#include <QList>
#include <QString>

class DataStore
{
public:
    explicit DataStore(const QString &filePath);

    bool load();
    bool save() const;
    void ensureSampleData();

    QList<Device> devices;
    QList<MonitorPoint> points;
    QList<Binding> bindings;
    QList<MonitorRecord> records;

    QString nextDeviceId() const;
    QString nextPointId() const;
    Device *deviceById(const QString &id);
    MonitorPoint *pointById(const QString &id);
    const Device *deviceById(const QString &id) const;
    const MonitorPoint *pointById(const QString &id) const;
    bool isDeviceBound(const QString &deviceId) const;
    bool isPointBound(const QString &pointId) const;
    QList<MonitorRecord> queryRecords(const QString &pointId, const QString &deviceId,
                                      const QDateTime &from, const QDateTime &to) const;

private:
    bool loadRealMonitoringData();

    QString m_filePath;
};

#endif
