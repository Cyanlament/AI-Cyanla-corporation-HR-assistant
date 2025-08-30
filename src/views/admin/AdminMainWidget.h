#ifndef ADMINMAINWIDGET_H
#define ADMINMAINWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include "UserManageWidget.h"
#include "SystemStatsWidget.h"
#include "SystemConfigWidget.h"
#include "AuditLogWidget.h"
#include "../../core/DatabaseManager.h"
#include "../staff/StatsWidget.h"

class AdminMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdminMainWidget(QWidget *parent = nullptr);
    
    void setCurrentUser(const UserInfo& user);

private:
    void setupUI();
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    UserManageWidget* m_userManageWidget;
    SystemStatsWidget* m_systemStatsWidget;
    SystemConfigWidget* m_systemConfigWidget;
    AuditLogWidget* m_auditLogWidget;
    StatsWidget* m_statsWidget;
    
    UserInfo m_currentUser;
};

#endif // ADMINMAINWIDGET_H 
