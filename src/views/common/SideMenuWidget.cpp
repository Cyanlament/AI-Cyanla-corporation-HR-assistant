#include "SideMenuWidget.h"
#include <QListWidgetItem>

SideMenuWidget::SideMenuWidget(UserRole role, QWidget *parent)
    : QWidget(parent)
    , m_userRole(role)
    , m_layout(nullptr)
    , m_logoLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_menuList(nullptr)
    , m_logoutButton(nullptr)
{
    setupUI();
}

void SideMenuWidget::setupUI()
{
    setFixedWidth(250);
    setStyleSheet(R"(
        SideMenuWidget {
            background-color: #2C3E50;
            border-right: 1px solid #34495E;
        }
        
        QLabel {
            color: white;
            font-weight: bold;
        }
        
        QListWidget {
            background-color: transparent;
            border: none;
            outline: none;
            color: #FFFFFF;
            font-size: 16px;
        }
        
        QListWidget::item {
            padding: 15px 20px;
            border-bottom: 1px solid #34495E;
            min-height: 20px;
            color: #FFFFFF;
            background-color: transparent;
            font-size: 16px;
            font-weight: normal;
        }
        
        QListWidget::item:selected {
            background-color: #3498DB;
            color: #FFFFFF;
            font-weight: bold;
        }
        
        QListWidget::item:hover {
            background-color: #34495E;
            color: #FFFFFF;
        }
        
        QPushButton {
            background-color: #E74C3C;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px;
            font-weight: bold;
            margin: 10px;
            font-size: 14px;
        }
        
        QPushButton:hover {
            background-color: #C0392B;
        }
        
        QPushButton:pressed {
            background-color: #A93226;
        }
    )");
    
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 20, 0, 20);
    m_layout->setSpacing(0);
    
    // Logo和标题
    m_logoLabel = new QLabel("🏥");
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setStyleSheet("font-size: 48px; padding: 20px;");
    
    m_titleLabel = new QLabel("智慧客服系统");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 18px; padding-bottom: 30px;");
    
    // 菜单列表
    m_menuList = new QListWidget;
    connect(m_menuList, &QListWidget::itemClicked,
            this, &SideMenuWidget::onListItemClicked);
    
    // 根据角色设置菜单
    switch (m_userRole) {
    case UserRole::visitor:
        setupvisitorMenu();
        break;
    case UserRole::Staff:
        setupStaffMenu();
        break;
    case UserRole::Admin:
        setupAdminMenu();
        break;
    }
    
    // 添加弹性空间
    QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    
    // 退出按钮
    m_logoutButton = new QPushButton("退出登录");
    connect(m_logoutButton, &QPushButton::clicked,
            this, &SideMenuWidget::onLogoutButtonClicked);
    
    // 添加到布局
    m_layout->addWidget(m_logoLabel);
    m_layout->addWidget(m_titleLabel);
    m_layout->addWidget(m_menuList);
    m_layout->addSpacerItem(spacer);
    m_layout->addWidget(m_logoutButton);
}

void SideMenuWidget::setupvisitorMenu()
{
    addMenuItem("💬 智能分诊", "chat", MenuAction::visitorChat);
    addMenuItem("❓ 常见问题", "faq", MenuAction::visitorAppointment);
    addMenuItem("🗺️ 院内导航", "map", MenuAction::visitorMap);
}

void SideMenuWidget::setupStaffMenu()
{
    addMenuItem("💬 客服管理", "chat_manage", MenuAction::StaffChatManage);
    addMenuItem("👥 患者列表", "visitor_list", MenuAction::StaffvisitorList);
    addMenuItem("📚 知识库", "knowledge", MenuAction::StaffKnowledge);
}

void SideMenuWidget::setupAdminMenu()
{
    addMenuItem("👤 用户管理", "user_manage", MenuAction::AdminUserManage);
    addMenuItem("📊 数据统计", "stats", MenuAction::AdminStats);
    addMenuItem("⚙️ 系统设置", "system", MenuAction::AdminSystem);
}

void SideMenuWidget::addMenuItem(const QString& text, const QString& icon, MenuAction action)
{
    QListWidgetItem* item = new QListWidgetItem(text);
    item->setData(Qt::UserRole, static_cast<int>(action));
    m_menuList->addItem(item);
    m_menuActions.append(action);
}

void SideMenuWidget::onListItemClicked(QListWidgetItem* item)
{
    if (item) {
        int actionValue = item->data(Qt::UserRole).toInt();
        MenuAction action = static_cast<MenuAction>(actionValue);
        emit menuItemClicked(action);
    }
}

void SideMenuWidget::onLogoutButtonClicked()
{
    emit logoutRequested();
} 