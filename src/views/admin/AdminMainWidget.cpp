#include "AdminMainWidget.h"


AdminMainWidget::AdminMainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AdminMainWidget::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;
    // 管理员界面可以在这里根据需要更新组件状态
}

void AdminMainWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_tabWidget = new QTabWidget;
    // 设置选项卡的位置为垂直方向
    // m_tabWidget->setTabPosition(QTabWidget::West);
    
    // 创建各个功能页面
    m_userManageWidget = new UserManageWidget;
    m_systemStatsWidget = new SystemStatsWidget;
    m_systemConfigWidget = new SystemConfigWidget;
    m_auditLogWidget = new AuditLogWidget;
    m_statsWidget = new StatsWidget;
    
    // 添加选项卡
    m_tabWidget->addTab(m_userManageWidget, "用户管理");
    // m_tabWidget->addTab(m_statsWidget, "高频问题统计");
    m_tabWidget->addTab(m_systemStatsWidget, "📊 系统统计");
    m_tabWidget->addTab(m_systemConfigWidget, "⚙️ 系统配置");
    //modify
    m_tabWidget->addTab(m_systemConfigWidget, "部门管理");
    //modify
    m_tabWidget->addTab(m_auditLogWidget, "审计日志");
    
    m_mainLayout->addWidget(m_tabWidget);


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
