#ifndef MODELS_H
#define MODELS_H

#include <QDate>
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QStringList>

enum class SensorType {
    Wind = 0,
    Deflection,
    ExpansionJoint,
    CableForce,
    TemperatureHumidity,
    Vibration,
    BearingDisplacement
};

QString sensorTypeName(SensorType type); // 把传感器类型转换成界面上显示的中文名称
QString sensorTypeUnit(SensorType type); // 获取该传感器主要数值的单位
QString sensorTypeSecondUnit(SensorType type); // 获取该传感器第二个数值的单位，没有则返回空
QStringList sensorTypeNames(); // 获取所有传感器类型名称，用于填充下拉框
SensorType sensorTypeFromIndex(int index); // 把下拉框索引转换成对应的传感器类型

struct Device {
    QString id;
    QString name;
    SensorType type = SensorType::Wind;
    QString specification;
    QString model;
    QString manufacturer;
    QDate productionDate;

    QJsonObject toJson() const; // 把设备对象转换成 JSON，方便保存到文件
    static Device fromJson(const QJsonObject &object); // 从 JSON 还原出一个设备对象
};

struct MonitorPoint {
    QString id;
    QString name;
    QString location;
    QString description;

    QJsonObject toJson() const; // 把监测点对象转换成 JSON，方便保存到文件
    static MonitorPoint fromJson(const QJsonObject &object); // 从 JSON 还原出一个监测点对象
};

struct Binding {
    QString pointId;
    QString deviceId;

    QJsonObject toJson() const; // 把绑定关系转换成 JSON，方便保存到文件
    static Binding fromJson(const QJsonObject &object); // 从 JSON 还原出一个绑定关系
};

struct MonitorRecord {
    QString deviceId;
    QString pointId;
    SensorType type = SensorType::Wind;
    QDateTime time;
    double value1 = 0.0;
    double value2 = 0.0;

    QJsonObject toJson() const; // 把监测记录转换成 JSON，方便保存到文件
    static MonitorRecord fromJson(const QJsonObject &object); // 从 JSON 还原出一条监测记录
};

#endif
