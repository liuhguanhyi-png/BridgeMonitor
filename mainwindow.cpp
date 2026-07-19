#include "mainwindow.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMap>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_store(QCoreApplication::applicationDirPath() + "/data/bridge_monitor_data.json")
{
    setWindowTitle(QStringLiteral("万州三桥桥梁监测数据分析与展示系统"));
    resize(1180, 760);

    m_store.load();

    QTabWidget *tabs = new QTabWidget(this);
    tabs->addTab(createOverviewTab(), QStringLiteral("系统概览"));//QStringLiteralz
    tabs->addTab(createDeviceTab(), QStringLiteral("监测装置管理"));
    tabs->addTab(createPointTab(), QStringLiteral("监测点管理"));
    tabs->addTab(createBindingTab(), QStringLiteral("绑定管理"));
    tabs->addTab(createHistoryTab(), QStringLiteral("历史数据分析"));
    setCentralWidget(tabs);

    refreshAll();
}

QWidget *MainWindow::createOverviewTab()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    
    // 标题标签：系统主标题
    QLabel *title = new QLabel(QStringLiteral("万州三桥桥梁健康监测"));
    QFont titleFont = title->font();// 获取标签默认字体
    titleFont.setPointSize(18);// 设置字体大小18号
    titleFont.setBold(true); // 字体加粗
    title->setFont(titleFont);// 将自定义字体应用到标题标签
    
    // 成员标签：用于展示汇总信息，开启自动换行
    m_summaryLabel = new QLabel;
    m_summaryLabel->setWordWrap(true);
    
    // 介绍文本标签：系统功能简介
    QLabel *intro = new QLabel(QStringLiteral(
        "系统支持风速风向、挠度、伸缩缝、索力、温湿度、振动、支座位移七类监测装置。"
        "可完成装置管理、监测点管理、装置与测点绑定、历史数据查询、表格展示和曲线分析，并使用本地文件保存数据。"));
    intro->setWordWrap(true);
    
    // 功能按钮：重新导入真实数据按钮
    QPushButton *resetButton = new QPushButton(QStringLiteral("重新导入真实监测数据"));
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetSampleData);

    layout->addWidget(title);
    layout->addWidget(intro);
    layout->addWidget(m_summaryLabel);
    layout->addStretch();
    layout->addWidget(resetButton, 0, Qt::AlignLeft);
    return page;
}

QWidget *MainWindow::createDeviceTab()
{
    QWidget *page = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(page);

    m_deviceTable = new QTableWidget;
    setTableHeaders(m_deviceTable, {QStringLiteral("编号"), QStringLiteral("名称"), QStringLiteral("类型"),
                                    QStringLiteral("规格"), QStringLiteral("型号"), QStringLiteral("厂家"),
                                    QStringLiteral("生产日期")});

    QGroupBox *formBox = new QGroupBox(QStringLiteral("新增监测装置"));
    QFormLayout *form = new QFormLayout(formBox);
    m_deviceNameEdit = new QLineEdit;
    m_deviceTypeCombo = new QComboBox;
    m_deviceTypeCombo->addItems(sensorTypeNames());
    m_deviceSpecEdit = new QLineEdit(QStringLiteral("工业级"));
    m_deviceModelEdit = new QLineEdit;
    m_deviceMakerEdit = new QLineEdit(QStringLiteral("重庆桥安智能设备有限公司"));
    m_deviceDateEdit = new QDateEdit(QDate::currentDate());
    m_deviceDateEdit->setCalendarPopup(true);
    QPushButton *addButton = new QPushButton(QStringLiteral("添加装置"));
    QPushButton *deleteButton = new QPushButton(QStringLiteral("删除选中装置"));
    form->addRow(QStringLiteral("名称"), m_deviceNameEdit);
    form->addRow(QStringLiteral("类型"), m_deviceTypeCombo);
    form->addRow(QStringLiteral("规格"), m_deviceSpecEdit);
    form->addRow(QStringLiteral("型号"), m_deviceModelEdit);
    form->addRow(QStringLiteral("厂家"), m_deviceMakerEdit);
    form->addRow(QStringLiteral("生产日期"), m_deviceDateEdit);
    form->addRow(addButton);
    form->addRow(deleteButton);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addDevice);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedDevice);

    layout->addWidget(m_deviceTable, 3);
    layout->addWidget(formBox, 1);
    return page;
}

QWidget *MainWindow::createPointTab()
{
    QWidget *page = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(page);

    m_pointTable = new QTableWidget;
    setTableHeaders(m_pointTable, {QStringLiteral("编号"), QStringLiteral("名称"),
                                  QStringLiteral("安装位置"), QStringLiteral("说明")});

    QGroupBox *formBox = new QGroupBox(QStringLiteral("新增监测点"));
    QFormLayout *form = new QFormLayout(formBox);
    m_pointNameEdit = new QLineEdit;
    m_pointLocationEdit = new QLineEdit;
    m_pointDescriptionEdit = new QTextEdit;
    m_pointDescriptionEdit->setMinimumHeight(120);
    QPushButton *addButton = new QPushButton(QStringLiteral("添加测点"));
    QPushButton *deleteButton = new QPushButton(QStringLiteral("删除选中测点"));
    form->addRow(QStringLiteral("名称"), m_pointNameEdit);
    form->addRow(QStringLiteral("安装位置"), m_pointLocationEdit);
    form->addRow(QStringLiteral("说明"), m_pointDescriptionEdit);
    form->addRow(addButton);
    form->addRow(deleteButton);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addPoint);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedPoint);

    layout->addWidget(m_pointTable, 3);
    layout->addWidget(formBox, 1);
    return page;
}

QWidget *MainWindow::createBindingTab()
{
    QWidget *page = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(page);

    m_bindingTable = new QTableWidget;
    setTableHeaders(m_bindingTable, {QStringLiteral("测点编号"), QStringLiteral("测点名称"),
                                    QStringLiteral("装置编号"), QStringLiteral("装置名称"),
                                    QStringLiteral("类型")});

    QGroupBox *formBox = new QGroupBox(QStringLiteral("装置与测点绑定"));
    QFormLayout *form = new QFormLayout(formBox);
    m_bindPointCombo = new QComboBox;
    m_bindDeviceCombo = new QComboBox;
    QPushButton *bindButton = new QPushButton(QStringLiteral("绑定"));
    QPushButton *unbindButton = new QPushButton(QStringLiteral("解绑选中关系"));
    form->addRow(QStringLiteral("监测点"), m_bindPointCombo);
    form->addRow(QStringLiteral("监测装置"), m_bindDeviceCombo);
    form->addRow(bindButton);
    form->addRow(unbindButton);

    connect(bindButton, &QPushButton::clicked, this, &MainWindow::bindSelected);
    connect(unbindButton, &QPushButton::clicked, this, &MainWindow::unbindSelected);

    layout->addWidget(m_bindingTable, 3);
    layout->addWidget(formBox, 1);
    return page;
}

QWidget *MainWindow::createHistoryTab()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    QGroupBox *queryBox = new QGroupBox(QStringLiteral("历史数据查询"));
    QHBoxLayout *queryLayout = new QHBoxLayout(queryBox);
    m_queryPointCombo = new QComboBox;
    m_queryDeviceCombo = new QComboBox;
    m_queryFromEdit = new QDateTimeEdit(QDateTime(QDate(2023, 1, 1), QTime(0, 0)));
    m_queryToEdit = new QDateTimeEdit(QDateTime(QDate(2023, 1, 7), QTime(23, 59)));
    m_queryFromEdit->setCalendarPopup(true);
    m_queryToEdit->setCalendarPopup(true);
    QPushButton *queryButton = new QPushButton(QStringLiteral("查询"));
    QPushButton *collectButton = new QPushButton(QStringLiteral("模拟采集一条数据"));
    queryLayout->addWidget(new QLabel(QStringLiteral("测点")));
    queryLayout->addWidget(m_queryPointCombo);
    queryLayout->addWidget(new QLabel(QStringLiteral("装置")));
    queryLayout->addWidget(m_queryDeviceCombo);
    queryLayout->addWidget(new QLabel(QStringLiteral("开始")));
    queryLayout->addWidget(m_queryFromEdit);
    queryLayout->addWidget(new QLabel(QStringLiteral("结束")));
    queryLayout->addWidget(m_queryToEdit);
    queryLayout->addWidget(queryButton);
    queryLayout->addWidget(collectButton);

    connect(queryButton, &QPushButton::clicked, this, &MainWindow::queryHistory);
    connect(collectButton, &QPushButton::clicked, this, &MainWindow::generateRecord);

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    m_recordTable = new QTableWidget;
    setTableHeaders(m_recordTable, {QStringLiteral("时间"), QStringLiteral("测点"), QStringLiteral("装置"),
                                   QStringLiteral("类型"), QStringLiteral("数值1"), QStringLiteral("数值2")});
    m_chart = new ChartWidget;
    splitter->addWidget(m_chart);
    splitter->addWidget(m_recordTable);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    layout->addWidget(queryBox);
    layout->addWidget(splitter, 1);
    return page;
}

void MainWindow::addDevice()
{
    if (m_deviceNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请输入装置名称。"));
        return;
    }
    Device device;
    device.id = m_store.nextDeviceId();
    device.name = m_deviceNameEdit->text().trimmed();
    device.type = sensorTypeFromIndex(m_deviceTypeCombo->currentIndex());
    device.specification = m_deviceSpecEdit->text().trimmed();
    device.model = m_deviceModelEdit->text().trimmed();
    device.manufacturer = m_deviceMakerEdit->text().trimmed();
    device.productionDate = m_deviceDateEdit->date();
    m_store.devices << device;
    m_store.save();
    m_deviceNameEdit->clear();
    m_deviceModelEdit->clear();
    refreshAll();
}

void MainWindow::deleteSelectedDevice()
{
    const QString id = selectedRowId(m_deviceTable);
    if (id.isEmpty()) {
        return;
    }
    for (const Binding &binding : m_store.bindings) {
        if (binding.deviceId == id) {
            QMessageBox::warning(this, QStringLiteral("无法删除"), QStringLiteral("该装置已绑定测点，请先解绑。"));
            return;
        }
    }
    for (int i = 0; i < m_store.devices.size(); ++i) {
        if (m_store.devices.at(i).id == id) {
            m_store.devices.removeAt(i);
            break;
        }
    }
    m_store.save();
    refreshAll();
}

void MainWindow::addPoint()
{
    if (m_pointNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请输入监测点名称。"));
        return;
    }
    MonitorPoint point;
    point.id = m_store.nextPointId();
    point.name = m_pointNameEdit->text().trimmed();
    point.location = m_pointLocationEdit->text().trimmed();
    point.description = m_pointDescriptionEdit->toPlainText().trimmed();
    m_store.points << point;
    m_store.save();
    m_pointNameEdit->clear();
    m_pointLocationEdit->clear();
    m_pointDescriptionEdit->clear();
    refreshAll();
}

void MainWindow::deleteSelectedPoint()
{
    const QString id = selectedRowId(m_pointTable);
    if (id.isEmpty()) {
        return;
    }
    for (const Binding &binding : m_store.bindings) {
        if (binding.pointId == id) {
            QMessageBox::warning(this, QStringLiteral("无法删除"), QStringLiteral("该测点已绑定装置，请先解绑。"));
            return;
        }
    }
    for (int i = 0; i < m_store.points.size(); ++i) {
        if (m_store.points.at(i).id == id) {
            m_store.points.removeAt(i);
            break;
        }
    }
    m_store.save();
    refreshAll();
}

void MainWindow::bindSelected()
{
    const QString pointId = m_bindPointCombo->currentData().toString();
    const QString deviceId = m_bindDeviceCombo->currentData().toString();
    if (pointId.isEmpty() || deviceId.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请选择未绑定的测点和装置。"));
        return;
    }
    if (m_store.isPointBound(pointId) || m_store.isDeviceBound(deviceId)) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("一个测点只能绑定一个装置，一个装置也只能绑定一个测点。"));
        return;
    }
    m_store.bindings << Binding{pointId, deviceId};
    m_store.save();
    refreshAll();
}

void MainWindow::unbindSelected()
{
    const int row = m_bindingTable->currentRow();
    if (row < 0) {
        return;
    }
    const QString pointId = m_bindingTable->item(row, 0)->text();
    const QString deviceId = m_bindingTable->item(row, 2)->text();
    for (int i = 0; i < m_store.bindings.size(); ++i) {
        const Binding &binding = m_store.bindings.at(i);
        if (binding.pointId == pointId && binding.deviceId == deviceId) {
            m_store.bindings.removeAt(i);
            break;
        }
    }
    m_store.save();
    refreshAll();
}

void MainWindow::queryHistory()
{
    const QString pointId = m_queryPointCombo->currentData().toString();
    const QString deviceId = m_queryDeviceCombo->currentData().toString();
    const QList<MonitorRecord> result = m_store.queryRecords(pointId, deviceId,
                                                             m_queryFromEdit->dateTime(),
                                                             m_queryToEdit->dateTime());
    m_recordTable->setRowCount(result.size());
    for (int row = 0; row < result.size(); ++row) {
        const MonitorRecord &record = result.at(row);
        const QString unit1 = sensorTypeUnit(record.type);
        const QString unit2 = sensorTypeSecondUnit(record.type);
        m_recordTable->setItem(row, 0, new QTableWidgetItem(record.time.toString("yyyy-MM-dd HH:mm:ss")));
        m_recordTable->setItem(row, 1, new QTableWidgetItem(displayNameForPoint(record.pointId)));
        m_recordTable->setItem(row, 2, new QTableWidgetItem(displayNameForDevice(record.deviceId)));
        m_recordTable->setItem(row, 3, new QTableWidgetItem(sensorTypeName(record.type)));
        m_recordTable->setItem(row, 4, new QTableWidgetItem(QString::number(record.value1, 'f', 2) + " " + unit1));
        m_recordTable->setItem(row, 5, new QTableWidgetItem(unit2.isEmpty() ? QStringLiteral("-") :
                                                            QString::number(record.value2, 'f', 2) + " " + unit2));
    }
    m_recordTable->resizeColumnsToContents();
    m_chart->setRecords(result, QStringLiteral("历史监测曲线：数值1"));
}

void MainWindow::generateRecord()
{
    if (m_store.bindings.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先绑定测点和装置。"));
        return;
    }
    const Binding binding = m_store.bindings.first();
    const Device *device = m_store.deviceById(binding.deviceId);
    if (!device) {
        return;
    }
    MonitorRecord record;
    record.pointId = binding.pointId;
    record.deviceId = binding.deviceId;
    record.type = device->type;
    record.time = QDateTime::currentDateTime();
    const double random = QRandomGenerator::global()->bounded(1000) / 1000.0;
    switch (device->type) {
    case SensorType::Wind:
        record.value1 = 4.0 + random * 8.0;
        record.value2 = QRandomGenerator::global()->bounded(360);
        break;
    case SensorType::CableForce:
        record.value1 = 780.0 + random * 120.0;
        break;
    case SensorType::TemperatureHumidity:
        record.value1 = 18.0 + random * 16.0;
        record.value2 = 45.0 + random * 40.0;
        break;
    case SensorType::Vibration:
        record.value1 = 2.8 + random * 1.2;
        break;
    default:
        record.value1 = 2.0 + random * 24.0;
        break;
    }
    m_store.records << record;
    m_store.save();
    refreshOverview();
    queryHistory();
}

void MainWindow::resetSampleData()
{
    if (QMessageBox::question(this, QStringLiteral("确认"), QStringLiteral("将重新导入真实监测数据并覆盖当前数据，是否继续？"))
        != QMessageBox::Yes) {
        return;
    }
    m_store.ensureSampleData();
    m_store.save();
    refreshAll();
}

void MainWindow::refreshAll()
{
    refreshOverview();
    refreshDevices();
    refreshPoints();
    refreshBindings();
    refreshCombos();
    queryHistory();
}

void MainWindow::refreshOverview()
{
    QMap<int, int> counts;
    for (const Device &device : m_store.devices) {
        counts[static_cast<int>(device.type)]++;
    }
    QStringList lines;
    for (int i = 0; i <= static_cast<int>(SensorType::BearingDisplacement); ++i) {
        lines << QString("%1：%2 台").arg(sensorTypeName(sensorTypeFromIndex(i))).arg(counts.value(i));
    }
    m_summaryLabel->setText(QStringLiteral("当前系统共有 %1 台监测装置、%2 个监测点、%3 条绑定关系、%4 条历史记录。\n%5")
                            .arg(m_store.devices.size())
                            .arg(m_store.points.size())
                            .arg(m_store.bindings.size())
                            .arg(m_store.records.size())
                            .arg(lines.join(QStringLiteral("；"))));
}

void MainWindow::refreshDevices()
{
    m_deviceTable->setRowCount(m_store.devices.size());
    for (int row = 0; row < m_store.devices.size(); ++row) {
        const Device &device = m_store.devices.at(row);
        m_deviceTable->setItem(row, 0, new QTableWidgetItem(device.id));
        m_deviceTable->setItem(row, 1, new QTableWidgetItem(device.name));
        m_deviceTable->setItem(row, 2, new QTableWidgetItem(sensorTypeName(device.type)));
        m_deviceTable->setItem(row, 3, new QTableWidgetItem(device.specification));
        m_deviceTable->setItem(row, 4, new QTableWidgetItem(device.model));
        m_deviceTable->setItem(row, 5, new QTableWidgetItem(device.manufacturer));
        m_deviceTable->setItem(row, 6, new QTableWidgetItem(device.productionDate.toString("yyyy-MM-dd")));
    }
    m_deviceTable->resizeColumnsToContents();
}

void MainWindow::refreshPoints()
{
    m_pointTable->setRowCount(m_store.points.size());
    for (int row = 0; row < m_store.points.size(); ++row) {
        const MonitorPoint &point = m_store.points.at(row);
        m_pointTable->setItem(row, 0, new QTableWidgetItem(point.id));
        m_pointTable->setItem(row, 1, new QTableWidgetItem(point.name));
        m_pointTable->setItem(row, 2, new QTableWidgetItem(point.location));
        m_pointTable->setItem(row, 3, new QTableWidgetItem(point.description));
    }
    m_pointTable->resizeColumnsToContents();
}

void MainWindow::refreshBindings()
{
    m_bindingTable->setRowCount(m_store.bindings.size());
    for (int row = 0; row < m_store.bindings.size(); ++row) {
        const Binding &binding = m_store.bindings.at(row);
        const Device *device = m_store.deviceById(binding.deviceId);
        m_bindingTable->setItem(row, 0, new QTableWidgetItem(binding.pointId));
        m_bindingTable->setItem(row, 1, new QTableWidgetItem(displayNameForPoint(binding.pointId)));
        m_bindingTable->setItem(row, 2, new QTableWidgetItem(binding.deviceId));
        m_bindingTable->setItem(row, 3, new QTableWidgetItem(displayNameForDevice(binding.deviceId)));
        m_bindingTable->setItem(row, 4, new QTableWidgetItem(device ? sensorTypeName(device->type) : QStringLiteral("-")));
    }
    m_bindingTable->resizeColumnsToContents();
}

void MainWindow::refreshCombos()
{
    auto fillPointCombo = [this](QComboBox *combo, bool onlyUnbound) {
        combo->clear();
        combo->addItem(QStringLiteral("全部/请选择"), QString());
        for (const MonitorPoint &point : m_store.points) {
            if (onlyUnbound && m_store.isPointBound(point.id)) {
                continue;
            }
            combo->addItem(point.id + " - " + point.name, point.id);
        }
    };
    auto fillDeviceCombo = [this](QComboBox *combo, bool onlyUnbound) {
        combo->clear();
        combo->addItem(QStringLiteral("全部/请选择"), QString());
        for (const Device &device : m_store.devices) {
            if (onlyUnbound && m_store.isDeviceBound(device.id)) {
                continue;
            }
            combo->addItem(device.id + " - " + device.name, device.id);
        }
    };
    fillPointCombo(m_bindPointCombo, true);
    fillDeviceCombo(m_bindDeviceCombo, true);
    fillPointCombo(m_queryPointCombo, false);
    fillDeviceCombo(m_queryDeviceCombo, false);
}

void MainWindow::setTableHeaders(QTableWidget *table, const QStringList &headers)
{
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
}

QString MainWindow::selectedRowId(QTableWidget *table, int column) const
{
    const int row = table->currentRow();
    if (row < 0 || !table->item(row, column)) {
        return QString();
    }
    return table->item(row, column)->text();
}

QString MainWindow::displayNameForDevice(const QString &id) const
{
    const Device *device = m_store.deviceById(id);
    return device ? device->name : id;
}

QString MainWindow::displayNameForPoint(const QString &id) const
{
    const MonitorPoint *point = m_store.pointById(id);
    return point ? point->name : id;
}
