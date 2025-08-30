#ifndef STAFFMAINWIDGET_H
#define STAFFMAINWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include "StaffChatManager.h"
#include "ConsultationWidget.h"
#include "StatsWidget.h"
#include "KnowledgeBaseWidget.h"
#include "../../core/DatabaseManager.h"

class StaffMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StaffMainWidget(QWidget *parent = nullptr);
    
    void setCurrentUser(const UserInfo& user);
    void setDatabaseManager(DatabaseManager* dbManager);

private:
    void setupUI();
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    StaffChatManager* m_chatManager;
    ConsultationWidget* m_consultationWidget;
    StatsWidget* m_statsWidget;
    KnowledgeBaseWidget* m_knowledgeBaseWidget;
    
    UserInfo m_currentUser;
    DatabaseManager* m_dbManager;
};

#endif // STAFFMAINWIDGET_H 