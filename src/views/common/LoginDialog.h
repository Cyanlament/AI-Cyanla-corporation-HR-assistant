#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>
#include <QCloseEvent>
#include "../../core/DatabaseManager.h"

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    
    UserInfo getLoggedInUser() const { return m_currentUser; }
    bool isLoginSuccessful() const { return m_loginSuccess; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onForgotPasswordClicked();
    void onRoleChanged();
    void onUsernameChanged();
    void onPasswordChanged();

private:
    void setupUI();
    void setupStyles();
    void setupAnimations();
    bool validateInput();
    void showMessage(const QString& message, bool isError = false);
    void clearMessage();
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    QFrame* m_loginFrame;
    QVBoxLayout* m_frameLayout;
    
    // 标题区域
    QLabel* m_logoLabel;
    QLabel* m_titleLabel;
    QLabel* m_subtitleLabel;
    
    // 输入区域
    QFrame* m_inputFrame;
    QGridLayout* m_inputLayout;
    QLabel* m_roleLabel;
    QComboBox* m_roleCombo;
    QLabel* m_usernameLabel;
    QLineEdit* m_usernameEdit;
    QLabel* m_passwordLabel;
    QLineEdit* m_passwordEdit;
    QCheckBox* m_rememberCheck;
    
    // 按钮区域
    QFrame* m_buttonFrame;
    QVBoxLayout* m_buttonLayout;
    QPushButton* m_loginButton;
    QHBoxLayout* m_linkLayout;
    QPushButton* m_registerButton;
    QPushButton* m_forgotButton;
    
    // 消息显示
    QLabel* m_messageLabel;
    
    // 动画效果
    QPropertyAnimation* m_frameAnimation;
    QGraphicsDropShadowEffect* m_shadowEffect;
    
    // 数据
    UserInfo m_currentUser;
    bool m_loginSuccess;
};

#endif // LOGINDIALOG_H 