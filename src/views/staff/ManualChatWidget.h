#ifndef MANUALCHATWIDGET_H
#define MANUALCHATWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QSplitter>
#include <QGroupBox>

class ManualChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManualChatWidget(QWidget *parent = nullptr);

private slots:
    void onSendMessage();
    void onUserSelected();

private:
    void setupUI();
    void loadUsers();

private:
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_contentLayout;
    
    // 用户列表
    QGroupBox* m_userGroup;
    QListWidget* m_userList;
    
    // 聊天区域
    QGroupBox* m_chatGroup;
    QTextEdit* m_chatDisplay;
    QLineEdit* m_messageInput;
    QPushButton* m_btnSend;
    QPushButton* m_btnTransfer;
};

#endif // MANUALCHATWIDGET_H 