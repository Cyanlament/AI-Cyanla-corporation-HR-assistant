#ifndef SYSTEMSTATSWIDGET_H
#define SYSTEMSTATSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QGroupBox>
#include <QProgressBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>

// 前向声明
QT_BEGIN_NAMESPACE
class QChartView;
QT_END_NAMESPACE

class SystemStatsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemStatsWidget(QWidget *parent = nullptr);

private slots:
    void onDateRangeChanged();
    void onRefreshStats();
    void onExportReport();
    void onTabChanged(int index);

private:
    void setupUI();
    void setupOverviewTab();
    void setupUserStatsTab();
    void setupSystemStatsTab();
    void setupReportsTab();
    void updateOverviewStats();
    void updateUserStats();
    void updateSystemStats();
    void createCharts();

private:
    QVBoxLayout* m_mainLayout;
    
    // 日期范围选择
    QGroupBox* m_dateGroup;
    QDateEdit* m_startDate;
    QDateEdit* m_endDate;
    QPushButton* m_btnRefresh;
    QPushButton* m_btnExport;
    
    // 选项卡
    QTabWidget* m_tabWidget;
    
    // 概览选项卡
    QWidget* m_overviewTab;
    QGridLayout* m_overviewLayout;
    QLabel* m_totalUsers;
    QLabel* m_activeUsers;
    QLabel* m_totalChats;
    QLabel* m_avgResponseTime;
    QProgressBar* m_systemLoad;
    QProgressBar* m_memoryUsage;
    
    // 用户统计选项卡
    QWidget* m_userStatsTab;
    QHBoxLayout* m_userStatsLayout;
    QChartView* m_userPieChart;
    QTableWidget* m_userStatsTable;
    
    // 系统统计选项卡
    QWidget* m_systemStatsTab;
    QVBoxLayout* m_systemStatsLayout;
    QChartView* m_performanceChart;
    QGroupBox* m_resourceGroup;
    
    // 报表选项卡
    QWidget* m_reportsTab;
    QVBoxLayout* m_reportsLayout;
    QComboBox* m_reportType;
    QTableWidget* m_reportTable;
};

#endif // SYSTEMSTATSWIDGET_H 