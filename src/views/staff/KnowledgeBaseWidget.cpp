#include "KnowledgeBaseWidget.h"
#include <QTreeWidgetItem>

KnowledgeBaseWidget::KnowledgeBaseWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadKnowledgeBase();
}

void KnowledgeBaseWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 搜索区域
    m_searchLayout = new QHBoxLayout;
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("搜索知识库...");
    m_searchButton = new QPushButton("搜索");
    
    m_searchLayout->addWidget(m_searchEdit);
    m_searchLayout->addWidget(m_searchButton);
    
    // 主体区域
    m_splitter = new QSplitter(Qt::Horizontal);
    
    m_categoryTree = new QTreeWidget;
    m_categoryTree->setHeaderLabel("知识分类");
    m_categoryTree->setMaximumWidth(300);
    
    m_contentArea = new QTextEdit;
    m_contentArea->setReadOnly(true);
    m_contentArea->setPlaceholderText("选择知识条目查看详细内容...");
    
    m_splitter->addWidget(m_categoryTree);
    m_splitter->addWidget(m_contentArea);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    
    // 按钮区域
    m_buttonLayout = new QHBoxLayout;
    m_addButton = new QPushButton("添加");
    m_editButton = new QPushButton("编辑");
    m_deleteButton = new QPushButton("删除");
    
    m_buttonLayout->addWidget(m_addButton);
    m_buttonLayout->addWidget(m_editButton);
    m_buttonLayout->addWidget(m_deleteButton);
    m_buttonLayout->addStretch();
    
    m_mainLayout->addLayout(m_searchLayout);
    m_mainLayout->addWidget(m_splitter);
    m_mainLayout->addLayout(m_buttonLayout);
}

void KnowledgeBaseWidget::loadKnowledgeBase()
{
    // 添加示例知识分类
    QTreeWidgetItem* appointment = new QTreeWidgetItem(m_categoryTree);
    appointment->setText(0, "挂号相关");
    
    QTreeWidgetItem* appointmentFlow = new QTreeWidgetItem(appointment);
    appointmentFlow->setText(0, "挂号流程");
    
    QTreeWidgetItem* appointmentTime = new QTreeWidgetItem(appointment);
    appointmentTime->setText(0, "挂号时间");
    
    QTreeWidgetItem* medical = new QTreeWidgetItem(m_categoryTree);
    medical->setText(0, "就医指南");
    
    QTreeWidgetItem* department = new QTreeWidgetItem(medical);
    department->setText(0, "科室介绍");
    
    QTreeWidgetItem* examination = new QTreeWidgetItem(medical);
    examination->setText(0, "检查项目");
    
    m_categoryTree->expandAll();
} 