#include "RoleSelector.h"

RoleSelector::RoleSelector(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_descLabel(nullptr)
    , m_btnPatient(nullptr)
    , m_btnStaff(nullptr)
    , m_btnAdmin(nullptr)
    , m_selectedRole(UserRole::Patient)
{
    setupUI();
}

void RoleSelector::setupUI()
{
    setWindowTitle("医院智慧客服系统 - 角色选择");
    setFixedSize(500, 500);
    setModal(true);
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(40, 40, 40, 40);
    m_mainLayout->setSpacing(20);
    
    // 标题
    m_titleLabel = new QLabel("🏥 医院智慧客服系统");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 28px;
            font-weight: bold;
            color: #1D1D1F;
            margin-bottom: 10px;
        }
    )");
    
    // 描述
    m_descLabel = new QLabel("请选择您的角色以进入系统");
    m_descLabel->setAlignment(Qt::AlignCenter);
    m_descLabel->setStyleSheet(R"(
        QLabel {
            font-size: 16px;
            color: #6D6D70;
            margin-bottom: 20px;
        }
    )");
    
    // 角色按钮 - 设计为独立卡片
    m_btnPatient = new QPushButton("👤 患者端\n在线咨询、常见问题、院内导航");
    m_btnStaff = new QPushButton("👩‍💼 客服端\n查看记录、问题统计、人工接管");
    m_btnAdmin = new QPushButton("👨‍💼 管理端\n用户管理、系统日志、服务配置");
    
    QString buttonStyle = R"(
        QPushButton {
            background-color: white;
            border: 2px solid #E5E5EA;
            border-radius: 12px;
            padding: 20px;
            font-size: 16px;
            font-weight: bold;
            text-align: center;
            color: #1D1D1F;
            min-height: 60px;
            margin: 8px 0px;
        }
        QPushButton:hover {
            border-color: #007AFF;
            background-color: #F2F8FF;
            transform: translateY(-2px);
        }
        QPushButton:pressed {
            background-color: #E3F2FD;
        }
    )";
    
    m_btnPatient->setStyleSheet(buttonStyle);
    m_btnStaff->setStyleSheet(buttonStyle);
    m_btnAdmin->setStyleSheet(buttonStyle);
    
    connect(m_btnPatient, &QPushButton::clicked, this, &RoleSelector::onPatientClicked);
    connect(m_btnStaff, &QPushButton::clicked, this, &RoleSelector::onStaffClicked);
    connect(m_btnAdmin, &QPushButton::clicked, this, &RoleSelector::onAdminClicked);
    
    // 添加到布局
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_descLabel);
    m_mainLayout->addSpacing(20);  // 标题和按钮之间的间隔
    
    // 创建独立的卡片容器
    QWidget* cardContainer = new QWidget;
    QVBoxLayout* cardLayout = new QVBoxLayout(cardContainer);
    cardLayout->setSpacing(15); // 卡片之间的间隔
    cardLayout->setContentsMargins(0, 0, 0, 0);
    
    cardLayout->addWidget(m_btnPatient);
    cardLayout->addWidget(m_btnStaff);
    cardLayout->addWidget(m_btnAdmin);
    
    m_mainLayout->addWidget(cardContainer);
    m_mainLayout->addSpacing(30);  // 底部间隔
    
    // 整体样式
    setStyleSheet(R"(
        QDialog {
            background-color: #F8F9FA;
        }
    )");
}

void RoleSelector::onPatientClicked()
{
    m_selectedRole = UserRole::Patient;
    accept();
}

void RoleSelector::onStaffClicked()
{
    m_selectedRole = UserRole::Staff;
    accept();
}

void RoleSelector::onAdminClicked()
{
    m_selectedRole = UserRole::Admin;
    accept();
} 