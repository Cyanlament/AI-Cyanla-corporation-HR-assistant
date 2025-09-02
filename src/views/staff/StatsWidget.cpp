#include "StatsWidget.h"
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QSqlError>
#include <QDebug>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QApplication>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

StatsWidget::StatsWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_mainSplitter(nullptr)
    , m_comboTimeRange(nullptr)
    , m_btnRefreshStats(nullptr)
    , m_tableStats(nullptr)
    , m_currentTimeRange(TimeRange::Last7Days)
    , m_isLoading(false)
    , m_currentSortColumn(-1)
    , m_currentSortOrder(Qt::DescendingOrder)
{
    initDatabase();
    setupUI();
    
    // 初始化刷新定时器
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setSingleShot(false);
    m_refreshTimer->setInterval(AUTO_REFRESH_INTERVAL);
    connect(m_refreshTimer, &QTimer::timeout, this, &StatsWidget::updateStatistics);
    
    // 加载初始数据
    QTimer::singleShot(100, this, &StatsWidget::updateStatistics);
}

StatsWidget::~StatsWidget()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

void StatsWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Vertical);
    
    setupFilterArea();
    setupSummaryArea();
    setupStatsTable();
    setupChartArea();
    setupExportArea();
    
    m_mainLayout->addWidget(m_mainSplitter);
    
    // 应用样式
    applyModernStyle();
}

void StatsWidget::setupFilterArea()
{
    m_filterGroup = new QGroupBox("🔍 筛选条件");
    m_filterGroup->hide();
    m_filterLayout = new QHBoxLayout(m_filterGroup);
    
    // 时间范围选择
    m_filterLayout->addWidget(new QLabel("时间范围:"));
    m_comboTimeRange = new QComboBox;
    m_comboTimeRange->addItem("今天", static_cast<int>(TimeRange::Today));
    m_comboTimeRange->addItem("近7天", static_cast<int>(TimeRange::Last7Days));
    m_comboTimeRange->addItem("本月", static_cast<int>(TimeRange::LastMonth));
    m_comboTimeRange->addItem("近3个月", static_cast<int>(TimeRange::Last3Months));
    m_comboTimeRange->addItem("自定义", static_cast<int>(TimeRange::Custom));
    m_comboTimeRange->setCurrentIndex(1); // 默认近7天
    
    // 自定义日期范围
    m_dateFrom = new QDateEdit(QDate::currentDate().addDays(-7));
    m_dateTo = new QDateEdit(QDate::currentDate());
    m_dateFrom->setVisible(false);
    m_dateTo->setVisible(false);
    
    // 刷新按钮
    m_btnRefreshStats = new QPushButton("🔄 刷新");
    m_btnShowDetails = new QPushButton("📊 详情");
    m_btnRefreshStats->hide();
    m_btnShowDetails->hide();


    
    // 最后更新时间
    m_lblLastUpdate = new QLabel("最后更新: --");
    
    m_filterLayout->addWidget(m_comboTimeRange);
    m_filterLayout->addWidget(new QLabel(" 从:"));
    m_filterLayout->addWidget(m_dateFrom);
    m_filterLayout->addWidget(new QLabel(" 到:"));
    m_filterLayout->addWidget(m_dateTo);
    m_filterLayout->addWidget(m_btnRefreshStats);
    m_filterLayout->addWidget(m_btnShowDetails);
    m_filterLayout->addStretch();
    m_filterLayout->addWidget(m_lblLastUpdate);
    
    // 连接信号
    connect(m_comboTimeRange, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StatsWidget::onTimeRangeChanged);
    connect(m_dateFrom, &QDateEdit::dateChanged, this, &StatsWidget::onCustomDateChanged);
    connect(m_dateTo, &QDateEdit::dateChanged, this, &StatsWidget::onCustomDateChanged);
    connect(m_btnRefreshStats, &QPushButton::clicked, this, &StatsWidget::onRefreshStatsClicked);
    connect(m_btnShowDetails, &QPushButton::clicked, this, &StatsWidget::onShowDetailsClicked);
    
    m_mainSplitter->addWidget(m_filterGroup);
}

void StatsWidget::setupSummaryArea()
{
    m_summaryGroup = new QGroupBox("📈 统计摘要");
    m_summaryGroup->hide();
    m_summaryLayout = new QGridLayout(m_summaryGroup);
    
    // 创建摘要标签
    m_lblTotalQuestions = new QLabel("0");
    m_lblUniqueQuestions = new QLabel("0");
    m_lblTopQuestion = new QLabel("--");
    m_lblAvgPerDay = new QLabel("0.0");
    
    // 进度条
    m_progressBar = new QProgressBar;
    m_progressBar->setVisible(false);
    
    // 布局设置
    m_summaryLayout->addWidget(new QLabel("总问题数:"), 0, 0);
    m_summaryLayout->addWidget(m_lblTotalQuestions, 0, 1);
    m_summaryLayout->addWidget(new QLabel("不重复问题:"), 0, 2);
    m_summaryLayout->addWidget(m_lblUniqueQuestions, 0, 3);
    
    m_summaryLayout->addWidget(new QLabel("最高频问题:"), 1, 0);
    m_summaryLayout->addWidget(m_lblTopQuestion, 1, 1, 1, 2);
    m_summaryLayout->addWidget(new QLabel("日均问题数:"), 1, 3);
    m_summaryLayout->addWidget(m_lblAvgPerDay, 1, 4);
    
    m_summaryLayout->addWidget(m_progressBar, 2, 0, 1, 5);
    
    m_mainSplitter->addWidget(m_summaryGroup);
}

void StatsWidget::setupStatsTable()
{
    m_tableGroup = new QGroupBox("问题排行");
    m_tableLayout = new QVBoxLayout(m_tableGroup);
    
    // 表格信息标签
    m_lblTableInfo = new QLabel("显示前100个高频问题");
    m_lblTableInfo->setStyleSheet("color: #666; font-size: 12px;");
    
    // 创建表格
    m_tableStats = new QTableWidget;
    m_tableStats->setColumnCount(6);
    m_tableStats->setHorizontalHeaderLabels({
        "排名", "问题内容", "出现次数", "占比(%)", "首次时间", "最后时间"
    });
    
    // 表格设置
    m_tableStats->setSortingEnabled(true);
    m_tableStats->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableStats->setAlternatingRowColors(true);
    m_tableStats->horizontalHeader()->setStretchLastSection(true);
    m_tableStats->verticalHeader()->setVisible(false);
    
    // 设置列宽
    m_tableStats->setColumnWidth(0, 60);   // 排名
    m_tableStats->setColumnWidth(1, 300);  // 问题内容
    m_tableStats->setColumnWidth(2, 80);   // 次数
    m_tableStats->setColumnWidth(3, 80);   // 占比
    m_tableStats->setColumnWidth(4, 120);  // 首次时间
    m_tableStats->setColumnWidth(5, 120);  // 最后时间
    
    // 连接表格信号
    connect(m_tableStats, &QTableWidget::itemClicked, 
            this, &StatsWidget::onTableItemClicked);
    connect(m_tableStats, &QTableWidget::itemDoubleClicked, 
            this, &StatsWidget::onTableItemDoubleClicked);
    
    m_tableLayout->addWidget(m_lblTableInfo);
    m_tableLayout->addWidget(m_tableStats);
    
    m_mainSplitter->addWidget(m_tableGroup);
}

void StatsWidget::setupChartArea()
{
    m_chartGroup = new QGroupBox("📊 图表分析");
    m_chartGroup->hide();
    m_chartLayout = new QVBoxLayout(m_chartGroup);
    
    // 图表类型选择
    QHBoxLayout* chartControlLayout = new QHBoxLayout;
    chartControlLayout->addWidget(new QLabel("图表类型:"));
    
    m_comboChartType = new QComboBox;
    m_comboChartType->addItem("柱状图", "bar");
    m_comboChartType->addItem("饼图", "pie");
    m_comboChartType->addItem("趋势图", "trend");
    
    chartControlLayout->addWidget(m_comboChartType);
    chartControlLayout->addStretch();
    
    m_chartLayout->addLayout(chartControlLayout);
    
#ifdef QT_CHARTS_LIB
    // 如果支持QtCharts，创建图表视图
    m_chartView = new QChartView;
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(300);
    m_chartLayout->addWidget(m_chartView);
    
    connect(m_comboChartType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StatsWidget::onChartTypeChanged);
#else
    // 如果不支持QtCharts，显示占位符
    m_chartPlaceholder = new QLabel("图表功能需要Qt Charts模块支持\n当前版本不可用");
    m_chartPlaceholder->setAlignment(Qt::AlignCenter);
    m_chartPlaceholder->setMinimumHeight(300);
    m_chartPlaceholder->setStyleSheet("border: 2px dashed #ccc; color: #999; font-size: 14px;");
    m_chartLayout->addWidget(m_chartPlaceholder);
#endif
    
    m_mainSplitter->addWidget(m_chartGroup);
}

void StatsWidget::setupExportArea()
{
    m_exportGroup = new QGroupBox("导出功能");
    m_exportGroup->hide();
    m_exportLayout = new QHBoxLayout(m_exportGroup);
    
    m_btnExportCSV = new QPushButton("导出CSV");
    m_btnExportExcel = new QPushButton("导出Excel");
    m_btnExportPDF = new QPushButton("导出PDF");
    
    m_lblExportStatus = new QLabel("准备就绪");
    m_lblExportStatus->setStyleSheet("color: #666;");
    
    connect(m_btnExportCSV, &QPushButton::clicked, this, &StatsWidget::onExportCSVClicked);
    connect(m_btnExportExcel, &QPushButton::clicked, this, &StatsWidget::onExportExcelClicked);
    connect(m_btnExportPDF, &QPushButton::clicked, this, &StatsWidget::onExportPDFClicked);
    
    m_exportLayout->addWidget(m_btnExportCSV);
    m_exportLayout->addWidget(m_btnExportExcel);
    m_exportLayout->addWidget(m_btnExportPDF);
    m_exportLayout->addStretch();
    m_exportLayout->addWidget(m_lblExportStatus);
    
    m_mainSplitter->addWidget(m_exportGroup);
}

void StatsWidget::initDatabase()
{
    // 初始化数据库连接
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    
    m_database = QSqlDatabase::addDatabase("QSQLITE", "stats_db");
    m_database.setDatabaseName(dbPath + "/question_stats.db");
    
    if (!m_database.open()) {
        qWarning() << "Failed to open database:" << m_database.lastError().text();
        return;
    }
    
    // 创建表结构（模拟）
    QSqlQuery query(m_database);
    query.exec("CREATE TABLE IF NOT EXISTS question_records ("
              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
              "user_id TEXT,"
              "question TEXT,"
              "keywords TEXT,"
              "category TEXT,"
              "timestamp TEXT)");
    
    // 插入模拟数据
    loadMockData();
}

void StatsWidget::loadMockData()
{
    // 检查是否已有数据
    QSqlQuery checkQuery(m_database);
    checkQuery.exec("SELECT COUNT(*) FROM question_records");
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QSqlQuery clearQuery(m_database);
        if (!clearQuery.exec("DELETE FROM question_records")) {
            qWarning() << "Failed to clear table:" << clearQuery.lastError().text();
            return;
        }
        qDebug() << "Existing data cleared from question_records table";
    }
    
    // 生成模拟的高频问题数据
    QStringList mockQuestions = {
        "丰川祥子是什么杯？", "小祥听说你认识明星若叶睦，能给我引荐一下吗？", "大祥你的三技能是打单？？", "丰川祥子你DPS多少？", "惩戒部在哪？", "听说情报部的队长和构筑部的副队长是色彩？真的？？", "抽卡开门时间是几点？", "血狼破军为什么退出黑蓑？", "涛哥是滚木吗？为什么查不到信息？", "如何获取体检报告？", "脑啡肽领取在哪里？上回真是溜爽了", "楼上疯了？毒品都磕", "我的ALEPHEGO在哪里？", "丰川祥子为什么退出crychic？", "医保报销的流程是什么？", "如何申请超大杯证明？", "公司体检项目有哪些？", "同事尸体化验结果如何解读？", "安保部手术的具体费用包括哪些？", "堂吉诃德说话为什么这么唐？", "项目四？", "五级员工具体属性为多少？", "如何保养EGO？", "妮妮的发型是怎么理的？", "怎么离职？", "义体手术后的恢复需要多长时间？", "血狼破军怎么死了？", "为什么要演奏春日影？"
    };
    
    QStringList categories = {"面试预约", "入职咨询", "公司问题", "部门导航", "其他问题"};
    
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO question_records (user_id, question, keywords, category, timestamp) "
                 "VALUES (?, ?, ?, ?, ?)");
    
    // 生成随机时间和频次
    QDateTime baseTime = QDateTime::currentDateTime().addDays(-30);

    for (int i = 0; i < 1000; ++i) {
        // 某些问题出现频率更高
        QString question;
        if (i < 300) { // 30%的记录是高频问题
            question = mockQuestions[QRandomGenerator::global()->bounded(10)]; // 前10个问题
        } else {
            question = mockQuestions[QRandomGenerator::global()->bounded(mockQuestions.size())];
        }
        
        QString userId = QString("user_%1").arg(QRandomGenerator::global()->bounded(1, 200));
        QString category = categories[QRandomGenerator::global()->bounded(categories.size())];
        
        QDateTime timestamp = baseTime.addSecs(QRandomGenerator::global()->bounded(86400 * 30));
        
        query.bindValue(0, userId);
        query.bindValue(1, question);
        query.bindValue(2, extractKeywords(question).join(","));
        query.bindValue(3, category);
        query.bindValue(4, timestamp.toString(Qt::ISODate));
        query.exec();
    }
}

void StatsWidget::updateStatistics()
{
    if (m_isLoading) return;
    
    m_isLoading = true;
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    
    loadQuestionStats();
    processQuestionData();
    calculateSummaryStats();
    populateStatsTable();
    updateCharts();
    
    m_lblLastUpdate->setText("最后更新: " + QDateTime::currentDateTime().toString("hh:mm:ss"));
    m_progressBar->setVisible(false);
    m_isLoading = false;
}

void StatsWidget::loadQuestionStats()
{
    m_questionStats.clear();
    m_progressBar->setValue(20);
    
    QDateTime startTime = getStartDateTime(m_currentTimeRange);
    QDateTime endTime = getEndDateTime(m_currentTimeRange);
    
    QSqlQuery query(m_database);
    query.prepare("SELECT question, COUNT(*) as count, "
                 "MIN(timestamp) as first_time, MAX(timestamp) as last_time "
                 "FROM question_records "
                 "WHERE timestamp BETWEEN ? AND ? "
                 "GROUP BY question "
                 "ORDER BY count DESC "
                 "LIMIT ?");
    
    query.bindValue(0, startTime.toString(Qt::ISODate));
    query.bindValue(1, endTime.toString(Qt::ISODate));
    query.bindValue(2, MAX_DISPLAY_ROWS);
    
    m_progressBar->setValue(40);
    
    if (query.exec()) {
        while (query.next()) {
            QuestionStats stats;
            stats.question = query.value("question").toString();
            stats.count = query.value("count").toInt();
            stats.firstOccurrence = QDateTime::fromString(query.value("first_time").toString(), Qt::ISODate);
            stats.lastOccurrence = QDateTime::fromString(query.value("last_time").toString(), Qt::ISODate);
            stats.keywords = extractKeywords(stats.question);
            
            m_questionStats.append(stats);
        }
    }
    
    m_progressBar->setValue(60);
}

void StatsWidget::processQuestionData()
{
    if (m_questionStats.isEmpty()) return;
    
    // 计算总数和百分比
    int totalCount = 0;
    for (const auto& stats : m_questionStats) {
        totalCount += stats.count;
    }
    
    for (auto& stats : m_questionStats) {
        stats.percentage = (totalCount > 0) ? (stats.count * 100.0 / totalCount) : 0.0;
    }
    
    m_progressBar->setValue(80);
}

void StatsWidget::calculateSummaryStats()
{
    m_summary = StatsSummary();
    
    if (!m_questionStats.isEmpty()) {
        // 计算总问题数
        for (const auto& stats : m_questionStats) {
            m_summary.totalQuestions += stats.count;
        }
        
        m_summary.uniqueQuestions = m_questionStats.size();
        m_summary.topQuestion = m_questionStats.first().question;
        m_summary.topQuestionCount = m_questionStats.first().count;
        
        // 计算平均每日问题数
        QDateTime startTime = getStartDateTime(m_currentTimeRange);
        QDateTime endTime = getEndDateTime(m_currentTimeRange);
        int days = startTime.daysTo(endTime) + 1;
        m_summary.avgQuestionsPerDay = (days > 0) ? (m_summary.totalQuestions * 1.0 / days) : 0.0;
    }
    
    // 更新摘要显示
    m_lblTotalQuestions->setText(formatCount(m_summary.totalQuestions));
    m_lblUniqueQuestions->setText(formatCount(m_summary.uniqueQuestions));
    m_lblTopQuestion->setText(m_summary.topQuestion.length() > 30 ? 
                             m_summary.topQuestion.left(30) + "..." : m_summary.topQuestion);
    m_lblAvgPerDay->setText(QString::number(m_summary.avgQuestionsPerDay, 'f', 1));
    
    m_progressBar->setValue(100);
}

void StatsWidget::populateStatsTable()
{
    m_tableStats->setRowCount(m_questionStats.size());
    
    for (int i = 0; i < m_questionStats.size(); ++i) {
        updateTableRow(i, m_questionStats[i]);
    }
    
    m_lblTableInfo->hide();
    // 更新表格信息
    m_lblTableInfo->setText(QString("显示 %1 个高频问题 (时间范围: %2)")
                           .arg(m_questionStats.size())
                           .arg(formatTimeRange(m_currentTimeRange)));
}

void StatsWidget::updateTableRow(int row, const QuestionStats& stats)
{
    // 排名
    QTableWidgetItem* rankItem = new QTableWidgetItem(QString::number(row + 1));
    rankItem->setTextAlignment(Qt::AlignCenter);
    
    // 问题内容
    QTableWidgetItem* questionItem = new QTableWidgetItem(stats.question);
    questionItem->setToolTip(stats.question);
    
    // 出现次数
    QTableWidgetItem* countItem = new QTableWidgetItem(formatCount(stats.count));
    countItem->setTextAlignment(Qt::AlignCenter);
    countItem->setData(Qt::UserRole, stats.count); // 用于排序
    
    // 占比
    QTableWidgetItem* percentItem = new QTableWidgetItem(formatPercentage(stats.percentage));
    percentItem->setTextAlignment(Qt::AlignCenter);
    percentItem->setData(Qt::UserRole, stats.percentage);
    
    // 时间
    QTableWidgetItem* firstTimeItem = new QTableWidgetItem(
        stats.firstOccurrence.toString("MM-dd hh:mm"));
    firstTimeItem->setTextAlignment(Qt::AlignCenter);
    
    QTableWidgetItem* lastTimeItem = new QTableWidgetItem(
        stats.lastOccurrence.toString("MM-dd hh:mm"));
    lastTimeItem->setTextAlignment(Qt::AlignCenter);
    
    m_tableStats->setItem(row, 0, rankItem);
    m_tableStats->setItem(row, 1, questionItem);
    m_tableStats->setItem(row, 2, countItem);
    m_tableStats->setItem(row, 3, percentItem);
    m_tableStats->setItem(row, 4, firstTimeItem);
    m_tableStats->setItem(row, 5, lastTimeItem);
}

// 槽函数实现
void StatsWidget::onRefreshStatsClicked()
{
    updateStatistics();
}

void StatsWidget::onTimeRangeChanged()
{
    TimeRange newRange = static_cast<TimeRange>(m_comboTimeRange->currentData().toInt());
    m_currentTimeRange = newRange;
    
    bool isCustom = (newRange == TimeRange::Custom);
    m_dateFrom->setVisible(isCustom);
    m_dateTo->setVisible(isCustom);
    
    if (!isCustom) {
        updateStatistics();
    }
}

void StatsWidget::onCustomDateChanged()
{
    if (m_currentTimeRange == TimeRange::Custom) {
        m_customStartDate = QDateTime(m_dateFrom->date(), QTime(0, 0, 0));
        m_customEndDate = QDateTime(m_dateTo->date().addDays(1), QTime(0, 0, 0)); // 包含结束日期
        updateStatistics();
    }
}

void StatsWidget::onExportCSVClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "导出CSV文件",
        QString("问题统计_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "CSV文件 (*.csv)");
    
    if (!fileName.isEmpty()) {
        if (exportToCSV(fileName)) {
            m_lblExportStatus->setText("CSV导出成功");
            QMessageBox::information(this, "导出成功", "CSV文件已保存至:\n" + fileName);
        } else {
            m_lblExportStatus->setText("CSV导出失败");
            QMessageBox::warning(this, "导出失败", "CSV文件导出失败，请检查文件权限。");
        }
    }
}

void StatsWidget::onTableItemClicked(QTableWidgetItem* item)
{
    if (!item) return;
    
    int row = item->row();
    if (row < m_questionStats.size()) {
        const QuestionStats& stats = m_questionStats[row];
        
        // 在状态栏显示详细信息
        QString info = QString("问题: %1 | 频次: %2 | 占比: %3%")
                      .arg(stats.question)
                      .arg(stats.count)
                      .arg(stats.percentage, 0, 'f', 2);
        
        m_lblExportStatus->setText(info);
    }
}

void StatsWidget::onTableItemDoubleClicked(QTableWidgetItem* item)
{
    if (!item) return;
    
    int row = item->row();
    if (row < m_questionStats.size()) {
        const QuestionStats& stats = m_questionStats[row];
        
        // 显示详细信息对话框
        QString details = QString(
            "问题详情\n\n"
            "内容: %1\n"
            "出现次数: %2\n"
            "占比: %3%\n"
            "首次出现: %4\n"
            "最后出现: %5\n"
            "相关关键词: %6")
            .arg(stats.question)
            .arg(stats.count)
            .arg(stats.percentage, 0, 'f', 2)
            .arg(stats.firstOccurrence.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(stats.lastOccurrence.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(stats.keywords.join(", "));
        
        QMessageBox::information(this, "问题详情", details);
    }
}

// 工具方法实现
QString StatsWidget::formatTimeRange(TimeRange range)
{
    switch (range) {
    case TimeRange::Today:
        return "今天";
    case TimeRange::Last7Days:
        return "近7天";
    case TimeRange::LastMonth:
        return "本月";
    case TimeRange::Last3Months:
        return "近3个月";
    case TimeRange::Custom:
        return QString("%1 至 %2")
               .arg(m_customStartDate.toString("MM-dd"))
               .arg(m_customEndDate.toString("MM-dd"));
    }
    return "未知";
}

QDateTime StatsWidget::getStartDateTime(TimeRange range)
{
    QDateTime now = QDateTime::currentDateTime();
    
    switch (range) {
    case TimeRange::Today:
        return QDateTime(now.date(), QTime(0, 0, 0));
    case TimeRange::Last7Days:
        return now.addDays(-7);
    case TimeRange::LastMonth:
        return QDateTime(QDate(now.date().year(), now.date().month(), 1), QTime(0, 0, 0));
    case TimeRange::Last3Months:
        return now.addMonths(-3);
    case TimeRange::Custom:
        return m_customStartDate;
    }
    return now.addDays(-7);
}

QDateTime StatsWidget::getEndDateTime(TimeRange range)
{
    QDateTime now = QDateTime::currentDateTime();
    
    switch (range) {
    case TimeRange::Today:
        return QDateTime(now.date().addDays(1), QTime(0, 0, 0));
    case TimeRange::Custom:
        return m_customEndDate;
    default:
        return now;
    }
}

QString StatsWidget::formatPercentage(double percentage)
{
    return QString("%1%").arg(percentage, 0, 'f', 1);
}

QString StatsWidget::formatCount(int count)
{
    if (count >= 1000) {
        return QString("%1k").arg(count / 1000.0, 0, 'f', 1);
    }
    return QString::number(count);
}

QStringList StatsWidget::extractKeywords(const QString& question)
{
    QStringList keywords;
    
    // 简单的关键词提取（移除常见词汇，提取有意义的词）
    QStringList commonWords = {"如何", "怎么", "什么", "哪里", "可以", "需要", "多少", "时候"};
    QStringList words = question.split(QRegularExpression("[\\s\\?？，。！!\\.,;；:]"), Qt::SkipEmptyParts);
    
    for (const QString& word : words) {
        if (word.length() >= MIN_KEYWORD_LENGTH && !commonWords.contains(word)) {
            keywords.append(word);
        }
    }
    
    return keywords;
}

bool StatsWidget::exportToCSV(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif
    
    // 写入BOM头
    out << "\xEF\xBB\xBF";
    
    // 写入表头
    out << "排名,问题内容,出现次数,占比(%),首次时间,最后时间\n";
    
    // 写入数据
    for (int i = 0; i < m_questionStats.size(); ++i) {
        const QuestionStats& stats = m_questionStats[i];
        out << QString("%1,\"%2\",%3,%4,%5,%6\n")
               .arg(i + 1)
               .arg(stats.question)
               .arg(stats.count)
               .arg(stats.percentage, 0, 'f', 2)
               .arg(stats.firstOccurrence.toString("yyyy-MM-dd hh:mm"))
               .arg(stats.lastOccurrence.toString("yyyy-MM-dd hh:mm"));
    }
    
    return true;
}

void StatsWidget::applyModernStyle()
{
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #E5E5EA;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            background-color: white;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 8px 0 8px;
            color: #1D1D1F;
        }
        QTableWidget {
            gridline-color: #E5E5EA;
            background-color: white;
            alternate-background-color: #F8F9FA;
            border: 1px solid #E5E5EA;
            border-radius: 6px;
        }
        QTableWidget::item {
            padding: 8px;
            border: none;
        }
        QTableWidget::item:selected {
            background-color: #007AFF;
            color: white;
        }
        QHeaderView::section {
            background-color: #F2F2F7;
            padding: 8px;
            border: 1px solid #E5E5EA;
            font-weight: bold;
        }
        QPushButton {
            background-color: #007AFF;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #0056CC;
        }
        QPushButton:pressed {
            background-color: #004499;
        }
        QComboBox {
            border: 1px solid #D1D1D6;
            border-radius: 6px;
            padding: 4px 8px;
            background-color: white;
            min-width: 120px;
        }
        QDateEdit {
            border: 1px solid #D1D1D6;
            border-radius: 6px;
            padding: 4px 8px;
            background-color: white;
        }
        QProgressBar {
            border: 1px solid #D1D1D6;
            border-radius: 4px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #007AFF;
            border-radius: 3px;
        }
        QLabel {
            color: #1D1D1F;
        }
    )");
}

QString StatsWidget::generateStatsReport()
{
    QString report = QString(
        "高频问题统计报告\n"
        "===================\n\n"
        "统计时间范围: %1\n"
        "总问题数: %2\n"
        "不重复问题数: %3\n"
        "最高频问题: %4 (%5次)\n"
        "日均问题数: %6\n\n"
        "前10高频问题:\n")
        .arg(formatTimeRange(m_currentTimeRange))
        .arg(m_summary.totalQuestions)
        .arg(m_summary.uniqueQuestions)
        .arg(m_summary.topQuestion)
        .arg(m_summary.topQuestionCount)
        .arg(m_summary.avgQuestionsPerDay, 0, 'f', 1);
    
    int count = qMin(10, m_questionStats.size());
    for (int i = 0; i < count; ++i) {
        const QuestionStats& stats = m_questionStats[i];
        report += QString("%1. %2 (%3次, %4%)\n")
                  .arg(i + 1)
                  .arg(stats.question)
                  .arg(stats.count)
                  .arg(stats.percentage, 0, 'f', 1);
    }
    
    return report;
}

// 未完全实现的方法（基础框架）
void StatsWidget::onExportExcelClicked() 
{
    QMessageBox::information(this, "功能开发中", "Excel导出功能正在开发中...");
}

void StatsWidget::onExportPDFClicked() 
{
    QMessageBox::information(this, "功能开发中", "PDF导出功能正在开发中...");
}

void StatsWidget::onChartTypeChanged() 
{
#ifdef QT_CHARTS_LIB
    updateCharts();
#endif
}

void StatsWidget::onShowDetailsClicked() 
{
    QString report = generateStatsReport();
    QMessageBox::information(this, "统计报告", report);
}

void StatsWidget::updateCharts() 
{
#ifdef QT_CHARTS_LIB
    // 图表更新逻辑（需要QtCharts支持）
    // 这里可以根据m_comboChartType的选择创建不同类型的图表
#endif
}

// 占位方法实现
void StatsWidget::createBarChart() { }
void StatsWidget::createPieChart() { }
void StatsWidget::createTrendChart() { }
void StatsWidget::updateProgressBar(int value) { Q_UNUSED(value) }
QString StatsWidget::normalizeQuestion(const QString& question) { return question; }
QMap<QString, int> StatsWidget::aggregateByKeywords(const QList<QuestionStats>& stats) { Q_UNUSED(stats) return QMap<QString, int>(); }
void StatsWidget::sortTableByColumn(int column) { Q_UNUSED(column) }
bool StatsWidget::exportToExcel(const QString& fileName) { Q_UNUSED(fileName) return false; }
bool StatsWidget::exportToPDF(const QString& fileName) { Q_UNUSED(fileName) return false; } 
