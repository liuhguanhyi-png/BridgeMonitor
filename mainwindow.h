#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chartwidget.h"
#include "datastore.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QTableWidget>
#include <QTextEdit>

class MainWindow : public QMainWindow
{
    Q_OBJECT//宏:要使用信号与槽就必须包含该宏

public:
    explicit MainWindow(QWidget *parent = nullptr); // 构造主窗口，创建界面并加载数据
//explocit:防止隐式转换
private slots:
    void addDevice(); // 点击“添加装置”后，把输入框内容保存为一个新的监测装置
    void deleteSelectedDevice(); // 删除设备表格中当前选中的监测装置
    void addPoint(); // 点击“添加测点”后，把输入框内容保存为一个新的监测点
    void deleteSelectedPoint(); // 删除测点表格中当前选中的监测点
    void bindSelected(); // 将下拉框中选中的监测点和监测装置建立绑定关系
    void unbindSelected(); // 解除绑定表格中当前选中的绑定关系
    void queryHistory(); // 根据测点、装置和时间范围查询历史监测记录
    void generateRecord(); // 模拟采集一条新的监测数据并保存
    void resetSampleData(); // 恢复程序内置的演示数据

private:
    QWidget *createOverviewTab(); // 创建“系统概览”标签页界面
    QWidget *createDeviceTab(); // 创建“监测装置管理”标签页界面
    QWidget *createPointTab(); // 创建“监测点管理”标签页界面
    QWidget *createBindingTab(); // 创建“绑定管理”标签页界面
    QWidget *createHistoryTab(); // 创建“历史数据分析”标签页界面

    void refreshAll(); // 刷新所有页面上的数据显示
    void refreshOverview(); // 刷新系统概览中的统计信息
    void refreshDevices(); // 刷新监测装置表格
    void refreshPoints(); // 刷新监测点表格
    void refreshBindings(); // 刷新测点和装置的绑定关系表格
    void refreshCombos(); // 刷新所有下拉框中的可选项
    void setTableHeaders(QTableWidget *table, const QStringList &headers); // 给表格设置表头和通用显示规则
    QString selectedRowId(QTableWidget *table, int column = 0) const; // 取得某个表格当前选中行指定列的编号
    QString displayNameForDevice(const QString &id) const; // 根据装置编号找到用于显示的装置名称
    QString displayNameForPoint(const QString &id) const; // 根据测点编号找到用于显示的测点名称

    DataStore m_store; // 数据仓库，负责保存和读取设备、测点、绑定关系、历史记录

    QLabel *m_summaryLabel = nullptr; // 系统概览页中的统计文字标签
    QTableWidget *m_deviceTable = nullptr; // 监测装置管理页中的设备表格
    QTableWidget *m_pointTable = nullptr; // 监测点管理页中的测点表格
    QTableWidget *m_bindingTable = nullptr; // 绑定管理页中的绑定关系表格
    QTableWidget *m_recordTable = nullptr; // 历史数据分析页中的历史记录表格
    ChartWidget *m_chart = nullptr; // 历史数据分析页中的曲线图控件

    QLineEdit *m_deviceNameEdit = nullptr; // 新增设备表单中的“名称”输入框
    QComboBox *m_deviceTypeCombo = nullptr; // 新增设备表单中的“类型”下拉框
    QLineEdit *m_deviceSpecEdit = nullptr; // 新增设备表单中的“规格”输入框
    QLineEdit *m_deviceModelEdit = nullptr; // 新增设备表单中的“型号”输入框
    QLineEdit *m_deviceMakerEdit = nullptr; // 新增设备表单中的“厂家”输入框
    QDateEdit *m_deviceDateEdit = nullptr; // 新增设备表单中的“生产日期”日期选择框

    QLineEdit *m_pointNameEdit = nullptr; // 新增测点表单中的“名称”输入框
    QLineEdit *m_pointLocationEdit = nullptr; // 新增测点表单中的“安装位置”输入框
    QTextEdit *m_pointDescriptionEdit = nullptr; // 新增测点表单中的“说明”多行文本框

    QComboBox *m_bindPointCombo = nullptr; // 绑定管理页中选择待绑定测点的下拉框
    QComboBox *m_bindDeviceCombo = nullptr; // 绑定管理页中选择待绑定设备的下拉框
    QComboBox *m_queryPointCombo = nullptr; // 历史查询区域中选择测点筛选条件的下拉框
    QComboBox *m_queryDeviceCombo = nullptr; // 历史查询区域中选择设备筛选条件的下拉框
    QDateTimeEdit *m_queryFromEdit = nullptr; // 历史查询区域中的开始时间选择框
    QDateTimeEdit *m_queryToEdit = nullptr; // 历史查询区域中的结束时间选择框
};

#endif
