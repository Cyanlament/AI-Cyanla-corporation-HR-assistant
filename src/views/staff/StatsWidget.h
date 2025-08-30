#ifndef STATSWIDGET_H
#define STATSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QDateEdit>
#include <QSplitter>
#include <QProgressBar>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QTimer>
#include <QMap>
#include <QStringList>

// 可选：支持图表功能（需要Qt Charts模块）
#ifdef QT_CHARTS_LIB
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>

// 前向声明
QT_BEGIN_NAMESPACE
class QChartView;
class QChart;
QT_END_NAMESPACE
#endif

// 时间范围枚举
enum class TimeRange {
    Today,          // 今天
    Last7Days,      // 近7天
    LastMonth,      // 本月
    Last3Months,    // 近3个月
    Custom          // 自定义时间范围
};

// 统计数据结构
struct QuestionStats {
    QString question;           // 问题内容
    int count;                  // 出现次数
    double percentage;          // 占比
    QDateTime firstOccurrence;  // 首次出现时间
    QDateTime lastOccurrence;   // 最后出现时间
    QStringList keywords;       // 相关关键词
};

// 统计摘要数据
struct StatsSummary {
    int totalQuestions;         // 总问题数
    int uniqueQuestions;        // 不重复问题数
    int topQuestionCount;       // 最高频问题次数
    QString topQuestion;        // 最高频问题内容
    double avgQuestionsPerDay;  // 平均每日问题数
};

class StatsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatsWidget(QWidget *parent = nullptr);
    ~StatsWidget();

private slots:
    // 数据刷新和筛选
    void onRefreshStatsClicked();
    void onTimeRangeChanged();
    void onCustomDateChanged();
    
    // 导出功能
    void onExportCSVClicked();
    void onExportExcelClicked();
    void onExportPDFClicked();
    
    // 表格交互
    void onTableItemClicked(QTableWidgetItem* item);
    void onTableItemDoubleClicked(QTableWidgetItem* item);
    
    // 图表切换
    void onChartTypeChanged();
    void onShowDetailsClicked();
    
    // 数据更新
    void updateStatistics();
    void updateProgressBar(int value);

private:
    // UI初始化
    void setupUI();
    void setupFilterArea();
    void setupStatsTable();
    void setupSummaryArea();
    void setupExportArea();
    void setupChartArea();
    
    // 数据处理
    void initDatabase();
    void loadQuestionStats();
    void processQuestionData();
    void calculateSummaryStats();
    
    // 关键词处理
    QStringList extractKeywords(const QString& question);
    QString normalizeQuestion(const QString& question);
    QMap<QString, int> aggregateByKeywords(const QList<QuestionStats>& stats);
    
    // 表格管理
    void populateStatsTable();
    void updateTableRow(int row, const QuestionStats& stats);
    void sortTableByColumn(int column);
    
    // 图表管理
    void updateCharts();
    void createBarChart();
    void createPieChart();
    void createTrendChart();
    
    // 导出功能
    bool exportToCSV(const QString& fileName);
    bool exportToExcel(const QString& fileName);
    bool exportToPDF(const QString& fileName);
    QString generateStatsReport();
    
    // 工具方法
    QString formatTimeRange(TimeRange range);
    QDateTime getStartDateTime(TimeRange range);
    QDateTime getEndDateTime(TimeRange range);
    QString formatPercentage(double percentage);
    QString formatCount(int count);
    
    // 新增缺失的方法声明
    void loadMockData();
    void applyModernStyle();

private:
    // 主布局
    QVBoxLayout* m_mainLayout;
    QSplitter* m_mainSplitter;
    
    // 筛选区域
    QGroupBox* m_filterGroup;
    QHBoxLayout* m_filterLayout;
    QComboBox* m_comboTimeRange;
    QDateEdit* m_dateFrom;
    QDateEdit* m_dateTo;
    QPushButton* m_btnRefreshStats;
    QPushButton* m_btnShowDetails;
    QLabel* m_lblLastUpdate;
    
    // 统计摘要区域
    QGroupBox* m_summaryGroup;
    QGridLayout* m_summaryLayout;
    QLabel* m_lblTotalQuestions;
    QLabel* m_lblUniqueQuestions;
    QLabel* m_lblTopQuestion;
    QLabel* m_lblAvgPerDay;
    QProgressBar* m_progressBar;
    
    // 统计表格区域
    QGroupBox* m_tableGroup;
    QVBoxLayout* m_tableLayout;
    QTableWidget* m_tableStats;
    QLabel* m_lblTableInfo;
    
    // 图表区域（可选）
    QGroupBox* m_chartGroup;
    QVBoxLayout* m_chartLayout;
    QComboBox* m_comboChartType;
    
#ifdef QT_CHARTS_LIB
    QChartView* m_chartView;
    QChart* m_barChart;
    QChart* m_pieChart;
    QChart* m_trendChart;
#else
    QLabel* m_chartPlaceholder;
#endif
    
    // 导出区域
    QGroupBox* m_exportGroup;
    QHBoxLayout* m_exportLayout;
    QPushButton* m_btnExportCSV;
    QPushButton* m_btnExportExcel;
    QPushButton* m_btnExportPDF;
    QLabel* m_lblExportStatus;
    
    // 数据管理
    QSqlDatabase m_database;
    QList<QuestionStats> m_questionStats;
    StatsSummary m_summary;
    QMap<QString, int> m_keywordStats;
    
    // 状态管理
    TimeRange m_currentTimeRange;
    QDateTime m_customStartDate;
    QDateTime m_customEndDate;
    QTimer* m_refreshTimer;
    bool m_isLoading;
    int m_currentSortColumn;
    Qt::SortOrder m_currentSortOrder;
    
    // 配置参数
    static const int MAX_DISPLAY_ROWS = 100;
    static const int MIN_KEYWORD_LENGTH = 2;
    static const int AUTO_REFRESH_INTERVAL = 300000; // 5分钟
};

#endif // STATSWIDGET_H 