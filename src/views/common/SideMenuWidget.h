#ifndef SIDEMENUWIDGET_H
#define SIDEMENUWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpacerItem>
#include "../../core/UserRole.h"

class SideMenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SideMenuWidget(UserRole role, QWidget *parent = nullptr);

private slots:
    void onListItemClicked(QListWidgetItem* item);
    void onLogoutButtonClicked();

private:
    void setupUI();
    void setupPatientMenu();
    void setupStaffMenu();
    void setupAdminMenu();
    void addMenuItem(const QString& text, const QString& icon, MenuAction action);

private:
    UserRole m_userRole;
    
    QVBoxLayout* m_layout;
    QLabel* m_logoLabel;
    QLabel* m_titleLabel;
    QListWidget* m_menuList;
    QPushButton* m_logoutButton;
    
    QList<MenuAction> m_menuActions;

signals:
    void menuItemClicked(MenuAction action);
    void logoutRequested();
};

#endif // SIDEMENUWIDGET_H 