#ifndef ROLESELECTOR_H
#define ROLESELECTOR_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "../../core/UserRole.h"

class RoleSelector : public QDialog
{
    Q_OBJECT

public:
    explicit RoleSelector(QWidget *parent = nullptr);
    
    UserRole getSelectedRole() const { return m_selectedRole; }

private slots:
    void onPatientClicked();
    void onStaffClicked();
    void onAdminClicked();

private:
    void setupUI();

private:
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QLabel* m_descLabel;
    
    QPushButton* m_btnPatient;
    QPushButton* m_btnStaff;
    QPushButton* m_btnAdmin;
    
    UserRole m_selectedRole;
};

#endif // ROLESELECTOR_H 