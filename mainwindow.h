#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QStackedWidget>
#include "src/core/DatabaseManager.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void setCurrentUser(const UserInfo& user);
    UserInfo getCurrentUser() const { return m_currentUser; }
    void setDatabaseManager(DatabaseManager* dbManager);

private slots:
    void showAdminPanel();
    void showStaffPanel();
    void showPatientPanel();
    void showUserProfile();
    void onLogout();
    void onAbout();

private:
    void setupUI();
    void setupMenus();
    void setupStatusBar();
    void updateUIForRole();
    
    // UI组件
    QStackedWidget* m_centralStack;
    QMenuBar* m_menuBar;
    QStatusBar* m_statusBar;
    QLabel* m_userLabel;
    QLabel* m_roleLabel;
    
    // 菜单和动作
    QMenu* m_systemMenu;
    QMenu* m_userMenu;
    QMenu* m_helpMenu;
    QAction* m_adminAction;
    QAction* m_staffAction;
    QAction* m_patientAction;
    QAction* m_profileAction;
    QAction* m_logoutAction;
    QAction* m_aboutAction;
    
    // 数据
    UserInfo m_currentUser;
    DatabaseManager* m_dbManager;
};

#endif // MAINWINDOW_H
