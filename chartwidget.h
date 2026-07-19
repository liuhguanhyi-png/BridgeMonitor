#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include "models.h"

#include <QWidget>

class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = nullptr); // 构造曲线图控件，设置基本大小

    void setRecords(const QList<MonitorRecord> &records, const QString &title); // 设置要绘制的监测记录和图表标题

protected:
    void paintEvent(QPaintEvent *event) override; // Qt 需要重画控件时自动调用，在这里绘制曲线图

private:
    QList<MonitorRecord> m_records;
    QString m_title;
};

#endif
