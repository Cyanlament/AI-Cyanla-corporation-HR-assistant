#ifndef AUDITLOGWIDGET_H
#define AUDITLOGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QSplitter>
#include <QTabWidget>

class AuditLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AuditLogWidget(QWidget *parent = nullptr);

private slots:
    void onSearchLogs();
    void onClearLogs();
    void onExportLogs();
    void onRefreshLogs();
    void onLogSelectionChanged();
    void onTabChanged(int index);

private:
    void setupUI();
    void setupOperationLogTab();
    void setupChatLogTab();
    void setupSystemLogTab();
    void loadOperationLogs();
    void loadChatLogs();
    void loadSystemLogs();
    void showLogDetails(const QString& details);

private:
    QVBoxLayout* m_mainLayout;
    
    // 搜索筛选区域
    QGroupBox* m_searchGroup;
    QLineEdit* m_searchEdit;
    QComboBox* m_logType;
    QComboBox* m_logLevel;
    QDateTimeEdit* m_startTime;
    QDateTimeEdit* m_endTime;
    QPushButton* m_btnSearch;
    QPushButton* m_btnClear;
    QPushButton* m_btnExport;
    QPushButton* m_btnRefresh;
    
    // 选项卡
    QTabWidget* m_tabWidget;
    
    // 操作日志选项卡
    QWidget* m_operationTab;
    QSplitter* m_operationSplitter;
    QTableWidget* m_operationTable;
    QTextEdit* m_operationDetails;
    
    // 聊天日志选项卡
    QWidget* m_chatTab;
    QSplitter* m_chatSplitter;
    QTableWidget* m_chatTable;
    QTextEdit* m_chatDetails;
    
    // 系统日志选项卡
    QWidget* m_systemTab;
    QSplitter* m_systemSplitter;
    QTableWidget* m_systemTable;
    QTextEdit* m_systemDetails;
    
    // 统计信息
    QLabel* m_statsLabel;
};

#endif // AUDITLOGWIDGET_H 