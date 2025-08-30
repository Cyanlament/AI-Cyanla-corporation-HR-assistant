#ifndef USERMANAGEWIDGET_H
#define USERMANAGEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QDateEdit>
#include <QCheckBox>
#include <QDialog>
#include "../../core/UserRole.h"

class UserManageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserManageWidget(QWidget *parent = nullptr);

private slots:
    void onAddUser();
    void onEditUser();
    void onDeleteUser();
    void onRefreshUsers();
    void onSearchUsers();
    void onUserSelectionChanged();
    void onExportUsers();

private:
    void setupUI();
    void loadUsers();
    void addUserToTable(int row, const QString& id, const QString& name, 
                       const QString& role, const QString& status, 
                       const QString& lastLogin);

private:
    QVBoxLayout* m_mainLayout;
    
    // 顶部工具栏
    QHBoxLayout* m_toolLayout;
    QPushButton* m_btnAdd;
    QPushButton* m_btnEdit;
    QPushButton* m_btnDelete;
    QPushButton* m_btnRefresh;
    QPushButton* m_btnExport;
    
    // 搜索区域
    QGroupBox* m_searchGroup;
    QLineEdit* m_searchEdit;
    QComboBox* m_roleFilter;
    QComboBox* m_statusFilter;
    QPushButton* m_btnSearch;
    
    // 用户列表
    QTableWidget* m_userTable;
    
    // 统计信息
    QLabel* m_statsLabel;
};

// 用户编辑对话框
class UserEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserEditDialog(QWidget *parent = nullptr);
    void setUserData(const QString& id, const QString& name, 
                    const QString& role, bool isActive);
    
    QString getUserId() const;
    QString getUserName() const;
    QString getUserRole() const;
    bool isUserActive() const;

private:
    void setupUI();

private:
    QVBoxLayout* m_layout;
    QLineEdit* m_editId;
    QLineEdit* m_editName;
    QComboBox* m_comboRole;
    QCheckBox* m_checkActive;
    QPushButton* m_btnOk;
    QPushButton* m_btnCancel;
};

#endif // USERMANAGEWIDGET_H 