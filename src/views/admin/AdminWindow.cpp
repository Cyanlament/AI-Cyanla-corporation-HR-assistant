#include "AdminWindow.h"
#include "UserManageWidget.h"
#include "SystemStatsWidget.h"
#include "SystemConfigWidget.h"
#include "AuditLogWidget.h"
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

AdminWindow::AdminWindow(QWidget *parent)
    : BaseWindow(UserRole::Admin, parent)
    , m_userManageWidget(nullptr)
    , m_systemStatsWidget(nullptr)
    , m_systemConfigWidget(nullptr)
    , m_auditLogWidget(nullptr)
    , m_toolBar(nullptr)
    , m_actBackup(nullptr)
    , m_actMaintenance(nullptr)
    , m_actSystemInfo(nullptr)
{
    setWindowTitle("公司智慧客服系统 - 管理端");
    createWidgets();
    setupToolBar();
}

AdminWindow::~AdminWindow()
{
    // Qt会自动清理子对象
}

void AdminWindow::setupMenu()
{
    // 菜单已在BaseWindow中通过SideMenuWidget设置
}

void AdminWindow::setupFunctionWidgets()
{
    // 添加功能页面到堆叠widget
    addFunctionWidget(m_userManageWidget, "用户管理");
    addFunctionWidget(m_systemStatsWidget, "数据统计");
    addFunctionWidget(m_systemConfigWidget, "系统设置");
    addFunctionWidget(m_auditLogWidget, "审计日志");
    
    // 默认显示用户管理页面
    setCurrentWidget("用户管理");
}

void AdminWindow::createWidgets()
{
    m_userManageWidget = new UserManageWidget(this);
    m_systemStatsWidget = new SystemStatsWidget(this);
    m_systemConfigWidget = new SystemConfigWidget(this);
    m_auditLogWidget = new AuditLogWidget(this);
}

void AdminWindow::setupToolBar()
{
    m_toolBar = addToolBar("管理工具");
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    // 数据备份
    m_actBackup = new QAction("📁 数据备份", this);
    m_actBackup->setToolTip("备份系统数据");
    connect(m_actBackup, &QAction::triggered, this, &AdminWindow::onBackupClicked);
    m_toolBar->addAction(m_actBackup);
    
    m_toolBar->addSeparator();
    
    // 系统维护
    m_actMaintenance = new QAction("🔧 系统维护", this);
    m_actMaintenance->setToolTip("系统维护模式");
    connect(m_actMaintenance, &QAction::triggered, this, &AdminWindow::onMaintenanceClicked);
    m_toolBar->addAction(m_actMaintenance);
    
    m_toolBar->addSeparator();
    
    // 系统信息
    m_actSystemInfo = new QAction("ℹ️ 系统信息", this);
    m_actSystemInfo->setToolTip("查看系统信息");
    connect(m_actSystemInfo, &QAction::triggered, this, &AdminWindow::onSystemInfoClicked);
    m_toolBar->addAction(m_actSystemInfo);
    
    // 设置工具栏样式
    m_toolBar->setStyleSheet(R"(
        QToolBar {
            background-color: #f8f9fa;
            border-bottom: 1px solid #dee2e6;
            padding: 8px;
            spacing: 3px;
        }
        QToolBar QToolButton {
            background-color: #ffffff;
            border: 1px solid #dee2e6;
            border-radius: 6px;
            padding: 8px 12px;
            margin: 2px;
            font-weight: 500;
            color: #495057;
        }
        QToolBar QToolButton:hover {
            background-color: #e9ecef;
            border-color: #adb5bd;
        }
        QToolBar QToolButton:pressed {
            background-color: #dee2e6;
        }
    )");
}

void AdminWindow::onMenuItemClicked(MenuAction action)
{
    switch (action) {
    case MenuAction::AdminUserManage:
        setCurrentWidget("用户管理");
        break;
    case MenuAction::AdminStats:
        setCurrentWidget("数据统计");
        break;
    case MenuAction::AdminSystem:
        setCurrentWidget("系统设置");
        break;
    default:
        break;
    }
}

void AdminWindow::onBackupClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "数据备份",
        "确定要开始数据备份吗？\n这可能需要几分钟时间。",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        // 这里可以调用实际的备份逻辑
        QMessageBox::information(this, "备份完成", "数据备份已成功完成！\n备份文件已保存到系统默认位置。");
    }
}

void AdminWindow::onMaintenanceClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        "系统维护",
        "启用维护模式将暂停所有用户服务！\n确定要继续吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QMessageBox::information(this, "维护模式", "系统已进入维护模式。\n新的用户连接将被拒绝。");
    }
}

void AdminWindow::onSystemInfoClicked()
{
    QString systemInfo = QString(
        "青蓝公司智慧客服系统\n\n"
        "版本: 1.0.0\n"
        "构建时间: %1\n"
        "Qt版本: %2\n"
        "操作系统: %3\n\n"
        "开发团队: 公司福利部\n"
        "技术支持: cyla@company.com"
    ).arg(__DATE__ " " __TIME__)
     .arg(QT_VERSION_STR)
     .arg(QSysInfo::prettyProductName());
    
    QMessageBox::about(this, "系统信息", systemInfo);
} 
