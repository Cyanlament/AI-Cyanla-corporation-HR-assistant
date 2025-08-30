#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include "../../core/UserRole.h"

class SideMenuWidget;
class TitleBarWidget;

class BaseWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BaseWindow(UserRole role, QWidget *parent = nullptr);
    virtual ~BaseWindow();

protected:
    // 添加功能页面到中央区域
    void addFunctionWidget(QWidget* widget, const QString& name);
    
    // 设置当前显示的页面
    void setCurrentWidget(int index);
    void setCurrentWidget(const QString& name);
    
    // 获取当前用户角色
    UserRole getUserRole() const { return m_userRole; }
    
    // 子类需要实现的虚函数
    virtual void setupMenu() = 0;
    virtual void setupFunctionWidgets() = 0;
    
    // 样式设置
    void applyModernStyle();

protected slots:
    virtual void onMenuItemClicked(MenuAction action) = 0;
    void onLogoutClicked();

private:
    void setupUI();
    void setupLayout();

protected:
    UserRole m_userRole;
    
    // UI组件
    QWidget* m_centralWidget;
    QHBoxLayout* m_mainLayout;
    
    SideMenuWidget* m_sideMenu;
    TitleBarWidget* m_titleBar;
    QStackedWidget* m_stackedWidget;
    
    // 功能页面管理
    QList<QWidget*> m_functionWidgets;
    QStringList m_widgetNames;

signals:
    void logoutRequested();
    void menuActionTriggered(MenuAction action);
};

#endif // BASEWINDOW_H 