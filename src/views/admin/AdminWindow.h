#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include "../common/BaseWindow.h"
#include <QToolBar>
#include <QAction>

class UserManageWidget;
class SystemStatsWidget;
class SystemConfigWidget;
class AuditLogWidget;

class AdminWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();

protected:
    void setupMenu() override;
    void setupFunctionWidgets() override;

protected slots:
    void onMenuItemClicked(MenuAction action) override;

private slots:
    void onBackupClicked();
    void onMaintenanceClicked();
    void onSystemInfoClicked();

private:
    void createWidgets();
    void setupToolBar();

private:
    // 功能组件
    UserManageWidget* m_userManageWidget;
    SystemStatsWidget* m_systemStatsWidget;
    SystemConfigWidget* m_systemConfigWidget;
    AuditLogWidget* m_auditLogWidget;
    
    // 工具栏
    QToolBar* m_toolBar;
    QAction* m_actBackup;
    QAction* m_actMaintenance;
    QAction* m_actSystemInfo;
};

#endif // ADMINWINDOW_H 