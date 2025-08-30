#ifndef RECORDWIDGET_H
#define RECORDWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QGroupBox>
#include <QLabel>
#include <QTextEdit>
#include <QSplitter>

class RecordWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecordWidget(QWidget *parent = nullptr);

private slots:
    void onSearchClicked();
    void onRecordSelected();
    void onFilterChanged();

private:
    void setupUI();
    void loadRecords();
    void filterRecords();

private:
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_filterLayout;
    QHBoxLayout* m_contentLayout;
    
    // 过滤控件
    QLineEdit* m_searchEdit;
    QComboBox* m_statusCombo;
    QDateEdit* m_dateFrom;
    QDateEdit* m_dateTo;
    QPushButton* m_btnSearch;
    QPushButton* m_btnClear;
    
    // 记录表格
    QGroupBox* m_recordGroup;
    QTableWidget* m_recordTable;
    
    // 详情显示
    QGroupBox* m_detailGroup;
    QTextEdit* m_detailDisplay;
    QPushButton* m_btnReply;
    QPushButton* m_btnClose;
    
    // 数据
    struct RecordItem {
        QString userId;
        QString userName;
        QString question;
        QString response;
        QString status;
        QString timestamp;
        QString category;
    };
    
    QList<RecordItem> m_recordData;
    QList<RecordItem> m_filteredData;
};

#endif // RECORDWIDGET_H 