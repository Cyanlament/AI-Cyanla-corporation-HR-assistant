#include "StaffMainWidget.h"
#include<QTabBar>

StaffMainWidget::StaffMainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void StaffMainWidget::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;

    // 更新聊天管理器的用户信息
    if (m_chatManager) {
        m_chatManager->setCurrentUser(user);
    }
}

void StaffMainWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    
    // 传递数据库管理器给聊天管理器
    if (m_chatManager) {
        // m_chatManager->setDatabaseManager(dbManager); // TODO: 实现此方法
    }
}

void StaffMainWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_tabWidget = new QTabWidget;
    // 设置选项卡的位置为垂直方向
    // m_tabWidget->setTabPosition(QTabWidget::South);
    
    // 创建各个功能页面
    m_chatManager = new StaffChatManager;        // 新的实时聊天管理器
    m_consultationWidget = new ConsultationWidget;
    m_statsWidget = new StatsWidget;
    m_knowledgeBaseWidget = new KnowledgeBaseWidget;
    
    // 添加选项卡
    // m_tabWidget->setTabBar(new QTabBar());
    m_tabWidget->addTab(m_chatManager, "客服聊天");

    //modify
    m_tabWidget->addTab(m_consultationWidget, "咨询管理");
    m_tabWidget->addTab(m_statsWidget, "高频问题统计");
    m_tabWidget->addTab(m_knowledgeBaseWidget, "知识库");
    
    m_mainLayout->addWidget(m_tabWidget);
     // m_tabWidget->tabBar()->hide();

    this->setStyleSheet(R"(
        QWidget {
            background-color: #CAEFFD;
        }

QTabWidget {
    background-color: #CAEFFD;
    border: 1px solid #dcdcdc;
}

QTabWidget::pane {
    border: 1px solid #dcdcdc;
}

QTabBar::tab {
    background-color: #e0e0e0;
    border: 1px solid #dcdcdc;
    padding: 5px;
    margin-right: 2px;
    color: #333;
}

QTabBar::tab:selected {
    background-color: #ffffff;
    border-bottom: 2px solid #2196F3;
}

QTabBar::tab:hover {
    background-color: #d0d0d0;
}

)");
} 
