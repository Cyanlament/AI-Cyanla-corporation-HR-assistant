#include "mainwindow.h"
#include "src/views/admin/AdminMainWidget.h"
#include "src/views/staff/StaffMainWidget.h"
#include "src/views/patient/PatientMainWidget.h"
#include "src/views/common/LoginDialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QMenu>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_dbManager(nullptr)
{
    setupUI();
    setupMenus();
    // setupStatusBar();
    
    setWindowTitle("医院智慧客服系统");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // 居中显示
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 创建中央堆叠窗口
    m_centralStack = new QStackedWidget;
    setCentralWidget(m_centralStack);
    
    // 预创建各角色的主界面
    AdminMainWidget* adminWidget = new AdminMainWidget;
    StaffMainWidget* staffWidget = new StaffMainWidget;
    PatientMainWidget* patientWidget = new PatientMainWidget;
    
    m_centralStack->addWidget(adminWidget);  // 索引 0
    m_centralStack->addWidget(staffWidget);  // 索引 1
    m_centralStack->addWidget(patientWidget); // 索引 2
}

void MainWindow::setupMenus()
{
    //modify
    // 系统菜单
    // m_systemMenu = menuBar()->addMenu("系统(&S)");
    m_systemMenu = new QMenu();
    
    m_adminAction = m_systemMenu->addAction("管理员界面(&A)");
    m_adminAction->setStatusTip("切换到管理员界面");
    connect(m_adminAction, &QAction::triggered, this, &MainWindow::showAdminPanel);
    
    m_staffAction = m_systemMenu->addAction("客服界面(&T)");
    m_staffAction->setStatusTip("切换到客服界面");
    connect(m_staffAction, &QAction::triggered, this, &MainWindow::showStaffPanel);
    
    m_patientAction = m_systemMenu->addAction("患者界面(&P)");
    m_patientAction->setStatusTip("切换到患者界面");
    connect(m_patientAction, &QAction::triggered, this, &MainWindow::showPatientPanel);
    
    m_systemMenu->addSeparator();
    
    QAction* exitAction = m_systemMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("退出应用程序");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    m_adminAction->setVisible(false);
    m_staffAction->setVisible(false);
    m_patientAction->setVisible(false);
    exitAction->setVisible(false);

    
    // 用户菜单
    m_userMenu = menuBar()->addMenu("用户(&U)");
    
    m_profileAction = m_userMenu->addAction("个人资料(&P)");
    m_profileAction->setStatusTip("查看和编辑个人资料");
    connect(m_profileAction, &QAction::triggered, this, &MainWindow::showUserProfile);
    
    m_userMenu->addSeparator();
    
    m_logoutAction = m_userMenu->addAction("注销登录(&L)");
    m_logoutAction->setStatusTip("注销当前用户并返回登录界面");
    connect(m_logoutAction, &QAction::triggered, this, &MainWindow::onLogout);
    
    // 帮助菜单
    m_helpMenu = menuBar()->addMenu("帮助(&H)");
    
    m_aboutAction = m_helpMenu->addAction("关于(&A)");
    m_aboutAction->setStatusTip("关于医院智慧客服系统系统");
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupStatusBar()
{
    // 用户信息标签
    m_userLabel = new QLabel;
    m_roleLabel = new QLabel;
    
    statusBar()->addWidget(new QLabel("当前用户:"));
    statusBar()->addWidget(m_userLabel);
    statusBar()->addWidget(new QLabel("|"));
    statusBar()->addWidget(new QLabel("角色:"));
    statusBar()->addWidget(m_roleLabel);
    statusBar()->addPermanentWidget(new QLabel("医院智慧客服系统v1.0.0"));
}

void MainWindow::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;
    
    // 更新状态栏
    // m_userLabel->setText(user.realName.isEmpty() ? user.username : user.realName);
    // m_roleLabel->setText(user.role);
    
    // 更新窗口标题
    setWindowTitle(QString("医院智慧客服系统 [%1 - %2]")
                   .arg(user.realName.isEmpty() ? user.username : user.realName)
                   .arg(user.role));
    
    // 传递用户信息给各个界面组件
    AdminMainWidget* adminWidget = qobject_cast<AdminMainWidget*>(m_centralStack->widget(0));
    if (adminWidget) {
        adminWidget->setCurrentUser(user);
    }
    
    StaffMainWidget* staffWidget = qobject_cast<StaffMainWidget*>(m_centralStack->widget(1));
    if (staffWidget) {
        staffWidget->setCurrentUser(user);
    }
    
    PatientMainWidget* patientWidget = qobject_cast<PatientMainWidget*>(m_centralStack->widget(2));
    if (patientWidget) {
        patientWidget->setCurrentUser(user);
        if (m_dbManager) {
            patientWidget->setDatabaseManager(m_dbManager);
        }
    }
    
    // 根据角色更新UI
    updateUIForRole();
}

void MainWindow::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    
    // 如果已经设置了用户和数据库管理器，传递给患者界面
    if (m_dbManager && !m_currentUser.username.isEmpty()) {
        PatientMainWidget* patientWidget = qobject_cast<PatientMainWidget*>(m_centralStack->widget(2));
        if (patientWidget) {
            patientWidget->setDatabaseManager(m_dbManager);
        }
        
        StaffMainWidget* staffWidget = qobject_cast<StaffMainWidget*>(m_centralStack->widget(1));
        if (staffWidget) {
            staffWidget->setDatabaseManager(m_dbManager);
        }
    }
}

void MainWindow::updateUIForRole()
{
    // 根据用户角色显示相应界面
    if (m_currentUser.role == "管理员") {
        m_centralStack->setCurrentIndex(0);
        m_adminAction->setEnabled(true);
        m_staffAction->setEnabled(true);
        m_patientAction->setEnabled(true);
    } else if (m_currentUser.role == "客服") {
        m_centralStack->setCurrentIndex(1);
        m_adminAction->setEnabled(false);
        m_staffAction->setEnabled(true);
        m_patientAction->setEnabled(false);
    } else if (m_currentUser.role == "患者") {
        m_centralStack->setCurrentIndex(2);
        m_adminAction->setEnabled(false);
        m_staffAction->setEnabled(false);
        m_patientAction->setEnabled(true);
    }
}

void MainWindow::showAdminPanel()
{
    if (m_currentUser.role == "管理员") {
        m_centralStack->setCurrentIndex(0);
        statusBar()->showMessage("已切换到管理员界面", 2000);
    } else {
        QMessageBox::warning(this, "权限不足", "您没有访问管理员界面的权限！");
    }
}

void MainWindow::showStaffPanel()
{
    if (m_currentUser.role == "管理员" || m_currentUser.role == "客服") {
        m_centralStack->setCurrentIndex(1);
        statusBar()->showMessage("已切换到客服界面", 2000);
    } else {
        QMessageBox::warning(this, "权限不足", "您没有访问客服界面的权限！");
    }
}

void MainWindow::showPatientPanel()
{
    // 所有用户都可以访问患者界面
    m_centralStack->setCurrentIndex(2);
    statusBar()->showMessage("已切换到患者界面", 2000);
}

void MainWindow::showUserProfile()
{
    QString profileInfo = QString(
        "用户资料\n\n"
        "用户名: %1\n"
        "真实姓名: %2\n"
        "邮箱: %3\n"
        "手机号: %4\n"
        "角色: %5\n"
        "注册时间: %6\n"
        "最后登录: %7"
    ).arg(m_currentUser.username)
     .arg(m_currentUser.realName.isEmpty() ? "未设置" : m_currentUser.realName)
     .arg(m_currentUser.email.isEmpty() ? "未设置" : m_currentUser.email)
     .arg(m_currentUser.phone.isEmpty() ? "未设置" : m_currentUser.phone)
     .arg(m_currentUser.role)
     .arg(m_currentUser.createdAt.toString("yyyy-MM-dd hh:mm:ss"))
     .arg(m_currentUser.lastLogin.toString("yyyy-MM-dd hh:mm:ss"));
    
    QMessageBox::information(this, "个人资料", profileInfo);
}

void MainWindow::onLogout()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "注销确认",
        "确定要注销当前用户吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        // 关闭主窗口
        close();
        
        // 重新显示登录对话框
        LoginDialog loginDialog;
        if (loginDialog.exec() == QDialog::Accepted) {
            UserInfo newUser = loginDialog.getLoggedInUser();
            setCurrentUser(newUser);
            show();
        } else {
            // 用户取消登录，退出应用程序
            QApplication::quit();
        }
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于 医院智慧客服系统",
        "<h2>医院智慧客服系统</h2>"
        "<p>版本: 1.0.0</p>"
        "<p>这是一个基于Qt开发的医院智慧客服系统，</p>"
        // "<p>提供患者咨询、客服管理、系统管理等功能。</p>"
        "<br>"
        // "<p>主要特性:</p>"
        // "<ul>"
        // "<li>安全的用户认证系统</li>"
        // "<li>多角色权限管理</li>"
        // "<li>智能客服对话</li>"
        // "<li>数据统计分析</li>"
        // "<li>系统配置管理</li>"
        // "</ul>"
        "<br>"
        "<p>© lulullu.</p>"
    );
}
