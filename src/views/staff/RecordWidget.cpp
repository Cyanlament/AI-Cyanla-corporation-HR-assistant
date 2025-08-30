#include "RecordWidget.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDate>
#include <QMessageBox>

RecordWidget::RecordWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_filterLayout(nullptr)
    , m_contentLayout(nullptr)
    , m_searchEdit(nullptr)
    , m_statusCombo(nullptr)
    , m_dateFrom(nullptr)
    , m_dateTo(nullptr)
    , m_btnSearch(nullptr)
    , m_btnClear(nullptr)
    , m_recordGroup(nullptr)
    , m_recordTable(nullptr)
    , m_detailGroup(nullptr)
    , m_detailDisplay(nullptr)
    , m_btnReply(nullptr)
    , m_btnClose(nullptr)
{
    setupUI();
    loadRecords();
}

void RecordWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);
    
    // 过滤区域
    QGroupBox* filterGroup = new QGroupBox("查询条件");
    m_filterLayout = new QHBoxLayout(filterGroup);
    
    QLabel* searchLabel = new QLabel("关键词:");
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("输入用户名或问题关键词...");
    
    QLabel* statusLabel = new QLabel("状态:");
    m_statusCombo = new QComboBox;
    m_statusCombo->addItems({"全部", "已回复", "未回复", "处理中"});
    
    QLabel* dateLabel = new QLabel("日期:");
    m_dateFrom = new QDateEdit(QDate::currentDate().addDays(-7));
    m_dateFrom->setCalendarPopup(true);
    m_dateTo = new QDateEdit(QDate::currentDate());
    m_dateTo->setCalendarPopup(true);
    
    m_btnSearch = new QPushButton("🔍 搜索");
    m_btnClear = new QPushButton("🗑️ 清空");
    
    connect(m_btnSearch, &QPushButton::clicked, this, &RecordWidget::onSearchClicked);
    connect(m_btnClear, &QPushButton::clicked, [this]() {
        m_searchEdit->clear();
        m_statusCombo->setCurrentIndex(0);
        m_dateFrom->setDate(QDate::currentDate().addDays(-7));
        m_dateTo->setDate(QDate::currentDate());
        filterRecords();
    });
    
    m_filterLayout->addWidget(searchLabel);
    m_filterLayout->addWidget(m_searchEdit);
    m_filterLayout->addWidget(statusLabel);
    m_filterLayout->addWidget(m_statusCombo);
    m_filterLayout->addWidget(dateLabel);
    m_filterLayout->addWidget(m_dateFrom);
    m_filterLayout->addWidget(new QLabel("至"));
    m_filterLayout->addWidget(m_dateTo);
    m_filterLayout->addWidget(m_btnSearch);
    m_filterLayout->addWidget(m_btnClear);
    m_filterLayout->addStretch();
    
    // 内容区域
    m_contentLayout = new QHBoxLayout;
    
    // 记录表格
    m_recordGroup = new QGroupBox("咨询记录");
    QVBoxLayout* tableLayout = new QVBoxLayout(m_recordGroup);
    
    m_recordTable = new QTableWidget;
    m_recordTable->setColumnCount(6);
    QStringList headers = {"用户ID", "用户名", "问题摘要", "状态", "分类", "时间"};
    m_recordTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_recordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recordTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_recordTable->setAlternatingRowColors(true);
    m_recordTable->horizontalHeader()->setStretchLastSection(true);
    m_recordTable->setColumnWidth(0, 80);
    m_recordTable->setColumnWidth(1, 100);
    m_recordTable->setColumnWidth(2, 200);
    m_recordTable->setColumnWidth(3, 80);
    m_recordTable->setColumnWidth(4, 100);
    
    connect(m_recordTable, &QTableWidget::itemSelectionChanged, 
            this, &RecordWidget::onRecordSelected);
    
    tableLayout->addWidget(m_recordTable);
    
    // 详情显示
    m_detailGroup = new QGroupBox("详细信息");
    QVBoxLayout* detailLayout = new QVBoxLayout(m_detailGroup);
    
    m_detailDisplay = new QTextEdit;
    m_detailDisplay->setReadOnly(true);
    m_detailDisplay->setPlaceholderText("选择记录查看详细信息...");
    
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_btnReply = new QPushButton("💬 回复");
    m_btnClose = new QPushButton("✅ 关闭");
    
    connect(m_btnReply, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "回复", "回复功能暂未实现");
    });
    
    connect(m_btnClose, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "关闭", "工单关闭功能暂未实现");
    });
    
    buttonLayout->addWidget(m_btnReply);
    buttonLayout->addWidget(m_btnClose);
    buttonLayout->addStretch();
    
    detailLayout->addWidget(m_detailDisplay);
    detailLayout->addLayout(buttonLayout);
    
    // 添加到内容布局
    m_contentLayout->addWidget(m_recordGroup);
    m_contentLayout->addWidget(m_detailGroup);
    m_contentLayout->setStretch(0, 2);
    m_contentLayout->setStretch(1, 1);
    
    // 添加到主布局
    m_mainLayout->addWidget(filterGroup);
    m_mainLayout->addLayout(m_contentLayout);
    
    // 样式设置
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #E5E5EA;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 10px 0 10px;
        }
        QTableWidget {
            border: 1px solid #CED4DA;
            border-radius: 6px;
            background-color: white;
            gridline-color: #F1F3F4;
        }
        QTableWidget::item {
            padding: 8px;
        }
        QTableWidget::item:selected {
            background-color: #007AFF;
            color: white;
        }
        QLineEdit, QComboBox, QDateEdit {
            border: 1px solid #CED4DA;
            border-radius: 4px;
            padding: 6px 8px;
        }
        QPushButton {
            background-color: #007AFF;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0056CC;
        }
    )");
}

void RecordWidget::loadRecords()
{
    m_recordData.clear();
    
    // 模拟数据
    m_recordData.append({"U001", "张三", "如何预约挂号？", "您可以通过官方微信...", "已回复", "2024-01-15 09:30", "挂号预约"});
    m_recordData.append({"U002", "李四", "医院在哪里？", "我院地址位于...", "已回复", "2024-01-15 10:15", "医院信息"});
    m_recordData.append({"U003", "王五", "检查结果什么时候出来？", "", "未回复", "2024-01-15 11:00", "检查结果"});
    m_recordData.append({"U004", "赵六", "医保怎么报销？", "正在处理中...", "处理中", "2024-01-15 14:20", "医保报销"});
    m_recordData.append({"U005", "孙七", "停车场收费标准？", "停车场收费标准为...", "已回复", "2024-01-14 16:45", "其他服务"});
    
    m_filteredData = m_recordData;
    filterRecords();
}

void RecordWidget::onSearchClicked()
{
    filterRecords();
}

void RecordWidget::onRecordSelected()
{
    int row = m_recordTable->currentRow();
    if (row >= 0 && row < m_filteredData.size()) {
        const RecordItem& record = m_filteredData[row];
        
        QString detailText = QString(R"(
<h3>📋 咨询详情</h3>
<hr>
<p><b>用户ID:</b> %1</p>
<p><b>用户名:</b> %2</p>
<p><b>分类:</b> %3</p>
<p><b>状态:</b> %4</p>
<p><b>时间:</b> %5</p>
<br>
<h4>💬 用户问题:</h4>
<p style="background-color: #F8F9FA; padding: 10px; border-left: 4px solid #007AFF;">%6</p>
<br>
<h4>🤖 系统回复:</h4>
<p style="background-color: #F0F8F0; padding: 10px; border-left: 4px solid #34C759;">%7</p>
        )")
        .arg(record.userId)
        .arg(record.userName)
        .arg(record.category)
        .arg(record.status)
        .arg(record.timestamp)
        .arg(record.question)
        .arg(record.response.isEmpty() ? "暂无回复" : record.response);
        
        m_detailDisplay->setHtml(detailText);
    }
}

void RecordWidget::onFilterChanged()
{
    filterRecords();
}

void RecordWidget::filterRecords()
{
    m_filteredData.clear();
    m_recordTable->setRowCount(0);
    
    QString keyword = m_searchEdit->text().toLower();
    QString status = m_statusCombo->currentText();
    QDate dateFrom = m_dateFrom->date();
    QDate dateTo = m_dateTo->date();
    
    for (const RecordItem& record : m_recordData) {
        // 关键词过滤
        bool matchKeyword = keyword.isEmpty() || 
                           record.userName.toLower().contains(keyword) ||
                           record.question.toLower().contains(keyword);
        
        // 状态过滤
        bool matchStatus = (status == "全部" || record.status == status);
        
        // 日期过滤
        QDate recordDate = QDate::fromString(record.timestamp.split(" ")[0], "yyyy-MM-dd");
        bool matchDate = recordDate >= dateFrom && recordDate <= dateTo;
        
        if (matchKeyword && matchStatus && matchDate) {
            m_filteredData.append(record);
            
            int row = m_recordTable->rowCount();
            m_recordTable->insertRow(row);
            
            m_recordTable->setItem(row, 0, new QTableWidgetItem(record.userId));
            m_recordTable->setItem(row, 1, new QTableWidgetItem(record.userName));
            m_recordTable->setItem(row, 2, new QTableWidgetItem(record.question.left(30) + "..."));
            m_recordTable->setItem(row, 3, new QTableWidgetItem(record.status));
            m_recordTable->setItem(row, 4, new QTableWidgetItem(record.category));
            m_recordTable->setItem(row, 5, new QTableWidgetItem(record.timestamp));
            
            // 根据状态设置行颜色
            QColor rowColor;
            if (record.status == "已回复") {
                rowColor = QColor(240, 248, 240); // 浅绿色
            } else if (record.status == "未回复") {
                rowColor = QColor(255, 245, 245); // 浅红色
            } else if (record.status == "处理中") {
                rowColor = QColor(255, 248, 220); // 浅黄色
            }
            
            for (int col = 0; col < m_recordTable->columnCount(); ++col) {
                if (!rowColor.isValid()) continue;
                QTableWidgetItem* item = m_recordTable->item(row, col);
                if (item) {
                    item->setBackground(rowColor);
                }
            }
        }
    }
    
    m_detailDisplay->clear();
} 