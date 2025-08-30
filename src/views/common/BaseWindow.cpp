#include "BaseWindow.h"
#include "SideMenuWidget.h"
#include "TitleBarWidget.h"
#include <QApplication>
#include <QTimer>

BaseWindow::BaseWindow(UserRole role, QWidget *parent)
    : QMainWindow(parent)
    , m_userRole(role)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_sideMenu(nullptr)
    , m_titleBar(nullptr)
    , m_stackedWidget(nullptr)
{
    setupUI();
    setupLayout();
    applyModernStyle();
    
    // 延迟调用虚函数，通过定时器确保派生类构造完成
    QTimer::singleShot(0, this, [this]() {
        setupMenu();
        setupFunctionWidgets();
    });
}

BaseWindow::~BaseWindow()
{
    // Qt会自动清理子对象
}

void BaseWindow::setupUI()
{
    setWindowTitle("医院智慧客服系统");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // 创建中央widget
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主布局
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // 创建侧边菜单
    m_sideMenu = new SideMenuWidget(m_userRole, this);
    connect(m_sideMenu, &SideMenuWidget::menuItemClicked, 
            this, &BaseWindow::onMenuItemClicked);
    connect(m_sideMenu, &SideMenuWidget::logoutRequested,
            this, &BaseWindow::onLogoutClicked);
    
    // 创建右侧内容区域
    QWidget* rightWidget = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    
    // 创建标题栏
    m_titleBar = new TitleBarWidget(this);
    
    // 创建堆叠widget
    m_stackedWidget = new QStackedWidget;
    
    rightLayout->addWidget(m_titleBar);
    rightLayout->addWidget(m_stackedWidget);
    
    // 添加到主布局
    m_mainLayout->addWidget(m_sideMenu);
    m_mainLayout->addWidget(rightWidget);
    
    // 设置比例
    m_mainLayout->setStretch(0, 0);  // 侧边栏固定宽度
    m_mainLayout->setStretch(1, 1);  // 内容区域自适应
}

void BaseWindow::setupLayout()
{
    // 基础布局已在setupUI中完成
}

void BaseWindow::addFunctionWidget(QWidget* widget, const QString& name)
{
    if (widget && !name.isEmpty()) {
        m_functionWidgets.append(widget);
        m_widgetNames.append(name);
        m_stackedWidget->addWidget(widget);
    }
}

void BaseWindow::setCurrentWidget(int index)
{
    if (index >= 0 && index < m_stackedWidget->count()) {
        m_stackedWidget->setCurrentIndex(index);
        
        // 更新标题栏
        if (index < m_widgetNames.size()) {
            m_titleBar->setTitle(m_widgetNames[index]);
        }
    }
}

void BaseWindow::setCurrentWidget(const QString& name)
{
    int index = m_widgetNames.indexOf(name);
    if (index >= 0) {
        setCurrentWidget(index);
    }
}

void BaseWindow::applyModernStyle()
{
    // 应用现代化样式
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        
        QWidget {
            font-family: "SF Pro Display", "Microsoft YaHei", sans-serif;
            font-size: 14px;
        }
        
        QPushButton {
            background-color: #007AFF;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: 500;
        }
        
        QPushButton:hover {
            background-color: #0056CC;
        }
        
        QPushButton:pressed {
            background-color: #004499;
        }
        
        QPushButton:disabled {
            color: white;
            background-color: #C7C7CC;
        }
        
        QListWidget {
            background-color: white;
            border: 1px solid #E5E5EA;
            border-radius: 8px;
            outline: none;
        }
        
        QListWidget::item {
            padding: 12px;
            border-bottom: 1px solid #E5E5EA;
        }
        
        QListWidget::item:selected {
            background-color: #007AFF;
            color: white;
        }
        
        QListWidget::item:hover {
            background-color: #F2F2F7;
        }
        
        QStackedWidget {
            background-color: white;
            border-radius: 8px;
        }
    )");
}

void BaseWindow::onLogoutClicked()
{
    emit logoutRequested();
} 
