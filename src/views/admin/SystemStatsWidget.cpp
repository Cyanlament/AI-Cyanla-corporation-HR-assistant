#include "SystemStatsWidget.h"
#include "../common/UIStyleManager.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QRandomGenerator>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>

SystemStatsWidget::SystemStatsWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_dateGroup(nullptr)
    , m_startDate(nullptr)
    , m_endDate(nullptr)
    , m_btnRefresh(nullptr)
    , m_btnExport(nullptr)
    , m_tabWidget(nullptr)
    , m_overviewTab(nullptr)
    , m_overviewLayout(nullptr)
    , m_totalUsers(nullptr)
    , m_activeUsers(nullptr)
    , m_totalChats(nullptr)
    , m_avgResponseTime(nullptr)
    , m_systemLoad(nullptr)
    , m_memoryUsage(nullptr)
    , m_userStatsTab(nullptr)
    , m_userStatsLayout(nullptr)
    , m_userPieChart(nullptr)
    , m_userStatsTable(nullptr)
    , m_systemStatsTab(nullptr)
    , m_systemStatsLayout(nullptr)
    , m_performanceChart(nullptr)
    , m_resourceGroup(nullptr)
    , m_reportsTab(nullptr)
    , m_reportsLayout(nullptr)
    , m_reportType(nullptr)
    , m_reportTable(nullptr)
{
    setupUI();
    updateOverviewStats();
}

void SystemStatsWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);
    
    // 标题
    QLabel* titleLabel = new QLabel("数据统计");
    UIStyleManager::applyLabelStyle(titleLabel, "title");
    m_mainLayout->addWidget(titleLabel);
    
    // 日期范围选择
    m_dateGroup = new QGroupBox("查询范围");
    UIStyleManager::applyGroupBoxStyle(m_dateGroup);
    QHBoxLayout* dateLayout = new QHBoxLayout(m_dateGroup);
    
    QLabel* startLabel = new QLabel("开始日期:");
    m_startDate = new QDateEdit;
    m_startDate->setDate(QDate::currentDate().addDays(-7));
    m_startDate->setCalendarPopup(true);
    
    QLabel* endLabel = new QLabel("结束日期:");
    m_endDate = new QDateEdit;
    m_endDate->setDate(QDate::currentDate());
    m_endDate->setCalendarPopup(true);
    
    m_btnRefresh = new QPushButton("🔄 刷新");
    m_btnExport = new QPushButton("📄 导出报表");
    
    UIStyleManager::applyButtonStyle(m_btnRefresh, "primary");
    UIStyleManager::applyButtonStyle(m_btnExport, "secondary");
    
    connect(m_btnRefresh, &QPushButton::clicked, this, &SystemStatsWidget::onRefreshStats);
    connect(m_btnExport, &QPushButton::clicked, this, &SystemStatsWidget::onExportReport);
    connect(m_startDate, &QDateEdit::dateChanged, this, &SystemStatsWidget::onDateRangeChanged);
    connect(m_endDate, &QDateEdit::dateChanged, this, &SystemStatsWidget::onDateRangeChanged);
    
    dateLayout->addWidget(startLabel);
    dateLayout->addWidget(m_startDate);
    dateLayout->addWidget(endLabel);
    dateLayout->addWidget(m_endDate);
    dateLayout->addWidget(m_btnRefresh);
    dateLayout->addWidget(m_btnExport);
    dateLayout->addStretch();
    
    m_mainLayout->addWidget(m_dateGroup);
    
    // 选项卡
    m_tabWidget = new QTabWidget;
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &SystemStatsWidget::onTabChanged);
    
    setupOverviewTab();
    setupUserStatsTab();
    setupSystemStatsTab();
    setupReportsTab();
    
    m_tabWidget->addTab(m_overviewTab, "📊 概览");
    m_tabWidget->addTab(m_userStatsTab, "👥 用户统计");
    m_tabWidget->addTab(m_systemStatsTab, "💻 系统性能");
    m_tabWidget->addTab(m_reportsTab, "📋 详细报表");
    
    m_mainLayout->addWidget(m_tabWidget);
}

void SystemStatsWidget::setupOverviewTab()
{
    m_overviewTab = new QWidget;
    m_overviewLayout = new QGridLayout(m_overviewTab);
    m_overviewLayout->setSpacing(15);
    
    // 统计卡片
    auto createStatsCard = [this](const QString& title, const QString& value, const QString& icon) -> QGroupBox* {
        QGroupBox* card = new QGroupBox;
        UIStyleManager::applyGroupBoxStyle(card);
        card->setMinimumHeight(120);
        
        QVBoxLayout* layout = new QVBoxLayout(card);
        layout->setAlignment(Qt::AlignCenter);
        
        QLabel* iconLabel = new QLabel(icon);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("font-size: 32px; margin-bottom: 5px;");
        
        QLabel* valueLabel = new QLabel(value);
        valueLabel->setAlignment(Qt::AlignCenter);
        UIStyleManager::applyLabelStyle(valueLabel, "title");
        
        QLabel* titleLabel = new QLabel(title);
        titleLabel->setAlignment(Qt::AlignCenter);
        UIStyleManager::applyLabelStyle(titleLabel, "caption");
        
        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(titleLabel);
        
        return card;
    };
    
    // 第一行统计卡片
    QGroupBox* usersCard = createStatsCard("总用户数", "1,247", "👥");
    QGroupBox* activeCard = createStatsCard("活跃用户", "892", "✅");
    QGroupBox* chatsCard = createStatsCard("总对话数", "5,431", "💬");
    QGroupBox* responseCard = createStatsCard("平均响应时间", "2.3秒", "⚡");
    
    m_overviewLayout->addWidget(usersCard, 0, 0);
    m_overviewLayout->addWidget(activeCard, 0, 1);
    m_overviewLayout->addWidget(chatsCard, 0, 2);
    m_overviewLayout->addWidget(responseCard, 0, 3);
    
    // 系统资源使用率
    QGroupBox* resourceGroup = new QGroupBox("系统资源");
    UIStyleManager::applyGroupBoxStyle(resourceGroup);
    QVBoxLayout* resourceLayout = new QVBoxLayout(resourceGroup);
    
    QLabel* loadLabel = new QLabel("CPU使用率:");
    m_systemLoad = new QProgressBar;
    m_systemLoad->setRange(0, 100);
    m_systemLoad->setValue(35);
    m_systemLoad->setFormat("%p%");
    
    QLabel* memoryLabel = new QLabel("内存使用率:");
    m_memoryUsage = new QProgressBar;
    m_memoryUsage->setRange(0, 100);
    m_memoryUsage->setValue(68);
    m_memoryUsage->setFormat("%p%");
    
    // 设置进度条样式
    QString progressStyle = QString(R"(
        QProgressBar {
            border: 2px solid %1;
            border-radius: 8px;
            text-align: center;
            background-color: %2;
            height: 20px;
        }
        QProgressBar::chunk {
            background-color: %3;
            border-radius: 6px;
        }
    )").arg("#dee2e6").arg("#f8f9fa").arg("#28a745");
    
    m_systemLoad->setStyleSheet(progressStyle);
    m_memoryUsage->setStyleSheet(progressStyle);
    
    resourceLayout->addWidget(loadLabel);
    resourceLayout->addWidget(m_systemLoad);
    resourceLayout->addWidget(memoryLabel);
    resourceLayout->addWidget(m_memoryUsage);
    
    m_overviewLayout->addWidget(resourceGroup, 1, 0, 1, 4);
}

void SystemStatsWidget::setupUserStatsTab()
{
    m_userStatsTab = new QWidget;
    m_userStatsLayout = new QHBoxLayout(m_userStatsTab);
    
    // 用户角色分布饼图
    QChart* chart = new QChart();
    QPieSeries* series = new QPieSeries();
    
    series->append("患者", 70.5);
    series->append("客服", 25.2);
    series->append("管理员", 4.3);
    
    // 设置饼图样式
    QPieSlice* visitorSlice = series->slices().at(0);
    visitorSlice->setColor(QColor("#3498db"));
    visitorSlice->setLabelVisible(true);
    
    QPieSlice* staffSlice = series->slices().at(1);
    staffSlice->setColor(QColor("#2ecc71"));
    staffSlice->setLabelVisible(true);
    
    QPieSlice* adminSlice = series->slices().at(2);
    adminSlice->setColor(QColor("#e74c3c"));
    adminSlice->setLabelVisible(true);
    
    chart->addSeries(series);
    chart->setTitle("用户角色分布");
    chart->legend()->setAlignment(Qt::AlignRight);
    
    m_userPieChart = new QChartView(chart);
    m_userPieChart->setRenderHint(QPainter::Antialiasing);
    
    // 用户详细统计表
    m_userStatsTable = new QTableWidget;
    m_userStatsTable->setColumnCount(4);
    QStringList headers = {"角色", "总数", "活跃", "占比"};
    m_userStatsTable->setHorizontalHeaderLabels(headers);
    
    m_userStatsTable->setRowCount(3);
    m_userStatsTable->setItem(0, 0, new QTableWidgetItem("患者"));
    m_userStatsTable->setItem(0, 1, new QTableWidgetItem("879"));
    m_userStatsTable->setItem(0, 2, new QTableWidgetItem("623"));
    m_userStatsTable->setItem(0, 3, new QTableWidgetItem("70.5%"));
    
    m_userStatsTable->setItem(1, 0, new QTableWidgetItem("客服"));
    m_userStatsTable->setItem(1, 1, new QTableWidgetItem("314"));
    m_userStatsTable->setItem(1, 2, new QTableWidgetItem("241"));
    m_userStatsTable->setItem(1, 3, new QTableWidgetItem("25.2%"));
    
    m_userStatsTable->setItem(2, 0, new QTableWidgetItem("管理员"));
    m_userStatsTable->setItem(2, 1, new QTableWidgetItem("54"));
    m_userStatsTable->setItem(2, 2, new QTableWidgetItem("28"));
    m_userStatsTable->setItem(2, 3, new QTableWidgetItem("4.3%"));
    
    m_userStatsTable->horizontalHeader()->setStretchLastSection(true);
    m_userStatsTable->setAlternatingRowColors(true);
    
    m_userStatsLayout->addWidget(m_userPieChart);
    m_userStatsLayout->addWidget(m_userStatsTable);
}

void SystemStatsWidget::setupSystemStatsTab()
{
    m_systemStatsTab = new QWidget;
    m_systemStatsLayout = new QVBoxLayout(m_systemStatsTab);
    
    // 性能趋势图
    QChart* perfChart = new QChart();
    QLineSeries* cpuSeries = new QLineSeries();
    QLineSeries* memorySeries = new QLineSeries();
    
    // 生成示例数据
    for (int i = 0; i < 24; ++i) {
        cpuSeries->append(i, 20 + QRandomGenerator::global()->bounded(30));
        memorySeries->append(i, 40 + QRandomGenerator::global()->bounded(40));
    }
    
    cpuSeries->setName("CPU使用率 (%)");
    memorySeries->setName("内存使用率 (%)");
    
    perfChart->addSeries(cpuSeries);
    perfChart->addSeries(memorySeries);
    perfChart->setTitle("24小时系统性能趋势");
    
    QValueAxis* axisX = new QValueAxis;
    axisX->setRange(0, 23);
    axisX->setTitleText("小时");
    perfChart->addAxis(axisX, Qt::AlignBottom);
    cpuSeries->attachAxis(axisX);
    memorySeries->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis;
    axisY->setRange(0, 100);
    axisY->setTitleText("使用率 (%)");
    perfChart->addAxis(axisY, Qt::AlignLeft);
    cpuSeries->attachAxis(axisY);
    memorySeries->attachAxis(axisY);
    
    m_performanceChart = new QChartView(perfChart);
    m_performanceChart->setRenderHint(QPainter::Antialiasing);
    
    // 资源详情
    m_resourceGroup = new QGroupBox("系统资源详情");
    UIStyleManager::applyGroupBoxStyle(m_resourceGroup);
    QGridLayout* resourceLayout = new QGridLayout(m_resourceGroup);
    
    QLabel* osLabel = new QLabel("操作系统:");
    QLabel* osValue = new QLabel("macOS 14.5.0");
    QLabel* qtLabel = new QLabel("Qt版本:");
    QLabel* qtValue = new QLabel(QT_VERSION_STR);
    QLabel* uptimeLabel = new QLabel("运行时间:");
    QLabel* uptimeValue = new QLabel("7天 3小时 25分钟");
    
    resourceLayout->addWidget(osLabel, 0, 0);
    resourceLayout->addWidget(osValue, 0, 1);
    resourceLayout->addWidget(qtLabel, 1, 0);
    resourceLayout->addWidget(qtValue, 1, 1);
    resourceLayout->addWidget(uptimeLabel, 2, 0);
    resourceLayout->addWidget(uptimeValue, 2, 1);
    
    m_systemStatsLayout->addWidget(m_performanceChart);
    m_systemStatsLayout->addWidget(m_resourceGroup);
}

void SystemStatsWidget::setupReportsTab()
{
    m_reportsTab = new QWidget;
    m_reportsLayout = new QVBoxLayout(m_reportsTab);
    
    // 报表类型选择
    QHBoxLayout* reportLayout = new QHBoxLayout;
    QLabel* typeLabel = new QLabel("报表类型:");
    m_reportType = new QComboBox;
    m_reportType->addItems({"用户活动报表", "对话统计报表", "系统性能报表", "错误日志报表"});
    
    reportLayout->addWidget(typeLabel);
    reportLayout->addWidget(m_reportType);
    reportLayout->addStretch();
    
    m_reportsLayout->addLayout(reportLayout);
    
    // 报表内容表格
    m_reportTable = new QTableWidget;
    m_reportTable->setColumnCount(5);
    QStringList headers = {"时间", "用户", "操作", "结果", "详情"};
    m_reportTable->setHorizontalHeaderLabels(headers);
    
    // 添加示例数据
    m_reportTable->setRowCount(10);
    for (int i = 0; i < 10; ++i) {
        m_reportTable->setItem(i, 0, new QTableWidgetItem(QDateTime::currentDateTime().addSecs(-i * 3600).toString("yyyy-MM-dd hh:mm")));
        m_reportTable->setItem(i, 1, new QTableWidgetItem(QString("用户%1").arg(i + 1)));
        m_reportTable->setItem(i, 2, new QTableWidgetItem("登录"));
        m_reportTable->setItem(i, 3, new QTableWidgetItem("成功"));
        m_reportTable->setItem(i, 4, new QTableWidgetItem("正常登录"));
    }
    
    m_reportTable->horizontalHeader()->setStretchLastSection(true);
    m_reportTable->setAlternatingRowColors(true);
    
    m_reportsLayout->addWidget(m_reportTable);
}

void SystemStatsWidget::updateOverviewStats()
{
    // 这里可以从数据库或API获取真实数据
    // 示例：更新进度条
    m_systemLoad->setValue(30 + QRandomGenerator::global()->bounded(40));
    m_memoryUsage->setValue(50 + QRandomGenerator::global()->bounded(30));
}

void SystemStatsWidget::updateUserStats()
{
    // 更新用户统计数据
}

void SystemStatsWidget::updateSystemStats()
{
    // 更新系统性能数据
}

void SystemStatsWidget::createCharts()
{
    // 创建或更新图表
}

void SystemStatsWidget::onDateRangeChanged()
{
    // 日期范围改变时触发
    QDate start = m_startDate->date();
    QDate end = m_endDate->date();
    
    if (start > end) {
        QMessageBox::warning(this, "日期错误", "开始日期不能晚于结束日期！");
        m_startDate->setDate(end.addDays(-7));
    }
}

void SystemStatsWidget::onRefreshStats()
{
    // 刷新统计数据
    updateOverviewStats();
    updateUserStats();
    updateSystemStats();
    
    QMessageBox::information(this, "刷新", "统计数据已刷新！");
}

void SystemStatsWidget::onExportReport()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出统计报表",
        QString("stats_report_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "CSV文件 (*.csv)"
    );
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            
            // 写入报表头
            out << "医院智慧客服系统统计报表\n";
            out << "生成时间," << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
            out << "统计范围," << m_startDate->date().toString("yyyy-MM-dd") << " 至 " << m_endDate->date().toString("yyyy-MM-dd") << "\n\n";
            
            // 写入概览数据
            out << "概览统计\n";
            out << "总用户数,1247\n";
            out << "活跃用户,892\n";
            out << "总对话数,5431\n";
            out << "平均响应时间,2.3秒\n\n";
            
            QMessageBox::information(this, "导出成功", 
                                   QString("统计报表已导出到:\n%1").arg(fileName));
        }
    }
}

void SystemStatsWidget::onTabChanged(int index)
{
    // 选项卡切换时触发相应的数据更新
    switch (index) {
    case 0: updateOverviewStats(); break;
    case 1: updateUserStats(); break;
    case 2: updateSystemStats(); break;
    case 3: /* 报表页面不需要特别更新 */ break;
    }
} 