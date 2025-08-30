#include "StaffWindow.h"
#include "RecordWidget.h"
#include "StatsWidget.h"
#include "ManualChatWidget.h"
#include <QMessageBox>

StaffWindow::StaffWindow(QWidget *parent)
    : BaseWindow(UserRole::Staff, parent)
    , m_recordWidget(nullptr)
    , m_statsWidget(nullptr)
    , m_manualChatWidget(nullptr)
    , m_toolBar(nullptr)
    , m_actRefresh(nullptr)
    , m_actExport(nullptr)
    , m_actSettings(nullptr)
{
    setWindowTitle("公司智慧客服系统 - 客服端");
    createWidgets();
    setupToolBar();
}

StaffWindow::~StaffWindow()
{
    // Qt会自动清理子对象
}

void StaffWindow::setupMenu()
{
    // 菜单已在BaseWindow中通过SideMenuWidget设置
}

void StaffWindow::setupFunctionWidgets()
{
    // 添加功能页面到堆叠widget
    addFunctionWidget(m_recordWidget, "提问记录");
    addFunctionWidget(m_statsWidget, "问题统计");
    addFunctionWidget(m_manualChatWidget, "人工接管");
    
    // 默认显示记录页面
    setCurrentWidget("提问记录");
}

void StaffWindow::createWidgets()
{
    m_recordWidget = new RecordWidget(this);
    m_statsWidget = new StatsWidget(this);
    m_manualChatWidget = new ManualChatWidget(this);
}

void StaffWindow::setupToolBar()
{
    m_toolBar = addToolBar("工具栏");
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    // 刷新按钮
    m_actRefresh = new QAction("🔄 刷新", this);
    m_actRefresh->setToolTip("刷新当前数据");
    connect(m_actRefresh, &QAction::triggered, this, &StaffWindow::onRefreshClicked);
    
    // 导出按钮
    m_actExport = new QAction("📊 导出", this);
    m_actExport->setToolTip("导出数据报表");
    connect(m_actExport, &QAction::triggered, this, &StaffWindow::onExportClicked);
    
    // 设置按钮
    m_actSettings = new QAction("⚙️ 设置", this);
    m_actSettings->setToolTip("系统设置");
    connect(m_actSettings, &QAction::triggered, this, &StaffWindow::onSettingsClicked);
    
    m_toolBar->addAction(m_actRefresh);
    m_toolBar->addAction(m_actExport);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_actSettings);
    
    // 工具栏样式
    m_toolBar->setStyleSheet(R"(
        QToolBar {
            background-color: #F8F9FA;
            border: none;
            border-bottom: 1px solid #E5E5EA;
            spacing: 5px;
        }
        QToolButton {
            background-color: transparent;
            border: 1px solid transparent;
            border-radius: 6px;
            padding: 8px 16px;
            margin: 2px;
        }
        QToolButton:hover {
            background-color: #E3F2FD;
            border-color: #BBDEFB;
        }
        QToolButton:pressed {
            background-color: #BBDEFB;
        }
    )");
}

void StaffWindow::onMenuItemClicked(MenuAction action)
{
    switch (action) {
    case MenuAction::StaffChatManage:
        setCurrentWidget("人工接管");
        break;
    case MenuAction::StaffvisitorList:
        setCurrentWidget("提问记录");
        break;
    case MenuAction::StaffKnowledge:
        setCurrentWidget("问题统计");
        break;
    default:
        break;
    }
}

void StaffWindow::onRefreshClicked()
{
    QMessageBox::information(this, "刷新", "数据已刷新");
}

void StaffWindow::onExportClicked()
{
    QMessageBox::information(this, "导出", "数据导出功能暂未实现");
}

void StaffWindow::onSettingsClicked()
{
    QMessageBox::information(this, "设置", "系统设置功能暂未实现");
} 