#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QFrame>
#include <QScrollArea>
#include <QProgressBar>
#include <QTimer>
#include "../../core/DatabaseManager.h"

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    
    void setSelectedRole(const QString& role);
    UserInfo getRegisteredUser() const { return m_registeredUser; }

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onRegisterClicked();
    void onCancelClicked();
    void onUsernameChanged();
    void onPasswordChanged();
    void onConfirmPasswordChanged();
    void onEmailChanged();
    void onRoleChanged();

private:
    void setupUI();
    void setupStyles();
    bool validateInput();
    void showMessage(const QString& message, bool isError = false);
    void clearMessage();
    void updatePasswordStrength();
    int calculatePasswordStrength(const QString& password);
    void updateRegisterButtonState();
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QFrame* m_registerFrame;
    QVBoxLayout* m_frameLayout;
    
    // 标题
    QLabel* m_titleLabel;
    QLabel* m_subtitleLabel;
    
    // 输入区域
    QFrame* m_inputFrame;
    QGridLayout* m_inputLayout;
    
    QLabel* m_roleLabel;
    QComboBox* m_roleCombo;
    
    QLabel* m_usernameLabel;
    QLineEdit* m_usernameEdit;
    QLabel* m_usernameHint;
    
    QLabel* m_passwordLabel;
    QLineEdit* m_passwordEdit;
    QProgressBar* m_passwordStrength;
    QLabel* m_passwordHint;
    
    QLabel* m_confirmPasswordLabel;
    QLineEdit* m_confirmPasswordEdit;
    QLabel* m_confirmPasswordHint;
    
    QLabel* m_realNameLabel;
    QLineEdit* m_realNameEdit;
    
    QLabel* m_emailLabel;
    QLineEdit* m_emailEdit;
    QLabel* m_emailHint;
    
    QLabel* m_phoneLabel;
    QLineEdit* m_phoneEdit;
    
    // 按钮区域
    QFrame* m_buttonFrame;
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_registerButton;
    QPushButton* m_cancelButton;
    
    // 消息显示
    QLabel* m_messageLabel;
    
    // 数据
    UserInfo m_registeredUser;
};

#endif // REGISTERDIALOG_H 