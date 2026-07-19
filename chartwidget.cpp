#include "chartwidget.h"

#include <QPainter>
#include <QtMath>

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(260);
}

void ChartWidget::setRecords(const QList<MonitorRecord> &records, const QString &title)
{
    m_records = records;
    m_title = title;
    update();
}

void ChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(250, 252, 255));

    const QRect plotRect = rect().adjusted(64, 42, -24, -48);
    painter.setPen(QColor(35, 45, 62));
    QFont titleFont = painter.font();
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(12, 8, width() - 24, 24), Qt::AlignLeft | Qt::AlignVCenter,
                     m_title.isEmpty() ? QStringLiteral("监测曲线") : m_title);

    painter.setPen(QPen(QColor(190, 198, 210), 1));
    painter.drawRect(plotRect);

    if (m_records.isEmpty()) {
        painter.setPen(QColor(100, 110, 125));
        painter.drawText(plotRect, Qt::AlignCenter, QStringLiteral("请选择测点、装置和时间范围后查询"));
        return;
    }

    double minValue = m_records.first().value1;
    double maxValue = m_records.first().value1;
    for (const MonitorRecord &record : m_records) {
        minValue = qMin(minValue, record.value1);
        maxValue = qMax(maxValue, record.value1);
    }
    if (qFuzzyCompare(minValue, maxValue)) {
        minValue -= 1.0;
        maxValue += 1.0;
    }
    const double padding = (maxValue - minValue) * 0.12;
    minValue -= padding;
    maxValue += padding;

    QFont axisFont = painter.font();
    axisFont.setPointSize(8);
    axisFont.setBold(false);
    painter.setFont(axisFont);

    for (int i = 0; i <= 4; ++i) {
        const int y = plotRect.bottom() - (plotRect.height() * i / 4);
        painter.setPen(QPen(QColor(225, 231, 239), 1));
        painter.drawLine(plotRect.left(), y, plotRect.right(), y);
        painter.setPen(QColor(85, 95, 110));
        const double value = minValue + (maxValue - minValue) * i / 4.0;
        painter.drawText(QRect(4, y - 10, 56, 20), Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(value, 'f', 2));
    }

    QPolygonF polyline;
    for (int i = 0; i < m_records.size(); ++i) {
        const double xRatio = m_records.size() == 1 ? 0.0 : double(i) / double(m_records.size() - 1);
        const double yRatio = (m_records.at(i).value1 - minValue) / (maxValue - minValue);
        const QPointF point(plotRect.left() + xRatio * plotRect.width(),
                            plotRect.bottom() - yRatio * plotRect.height());
        polyline << point;
    }

    painter.setPen(QPen(QColor(0, 122, 204), 2.2));
    painter.drawPolyline(polyline);
    painter.setBrush(QColor(0, 122, 204));
    painter.setPen(Qt::NoPen);
    for (const QPointF &point : polyline) {
        painter.drawEllipse(point, 3.2, 3.2);
    }

    painter.setPen(QColor(85, 95, 110));
    painter.drawText(QRect(plotRect.left(), plotRect.bottom() + 8, 160, 20),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     m_records.first().time.toString("MM-dd HH:mm"));
    painter.drawText(QRect(plotRect.right() - 160, plotRect.bottom() + 8, 160, 20),
                     Qt::AlignRight | Qt::AlignVCenter,
                     m_records.last().time.toString("MM-dd HH:mm"));
}
