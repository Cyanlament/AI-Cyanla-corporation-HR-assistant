#ifndef PATIENTMAINWIDGET_H
#define PATIENTMAINWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include "ChatWidget.h"
#include "RealChatWidget.h"
#include "FAQWidget.h"
#include "AppointmentWidget.h"
#include "../../core/DatabaseManager.h"

class PatientMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PatientMainWidget(QWidget *parent = nullptr);
    
    void setCurrentUser(const UserInfo& user);
    void setDatabaseManager(DatabaseManager* dbManager);

private slots:
    void onRequestHumanService(const QString& userId, const QString& userName, const QString& context);

private:
    void setupUI();
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    ChatWidget* m_chatWidget;
    RealChatWidget* m_realChatWidget;
    FAQWidget* m_faqWidget;
    AppointmentWidget* m_appointmentWidget;
    
    UserInfo m_currentUser;
};

#endif // PATIENTMAINWIDGET_H 