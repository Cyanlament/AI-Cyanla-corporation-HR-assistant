#ifndef REALCHATWIDGET_H
#define REALCHATWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDateTime>
#include "../../core/DatabaseManager.h"

class RealChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RealChatWidget(QWidget *parent = nullptr);
    ~RealChatWidget();
    
    void setCurrentUser(const UserInfo& user);
    void setDatabaseManager(DatabaseManager* dbManager);
    void startNewChat();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSendMessage();
    void onMessageReceived(const ChatMessage& message);
    void onSessionCreated(const ChatSession& session);
    void onSessionUpdated(const ChatSession& session);
    void checkForNewMessages();
    void onUserInput();

private:
    void setupUI();
    void setupMessageArea();
    void setupInputArea();
    void addMessage(const ChatMessage& message);
    void scrollToBottom();
    void updateConnectionStatus();
    void loadChatHistory();
    QString formatTime(const QDateTime& time);
    QWidget* createMessageBubble(const ChatMessage& message);

    // UI 组件
    QVBoxLayout* m_mainLayout;
    QLabel* m_statusLabel;
    QScrollArea* m_messageScrollArea;
    QWidget* m_messageContainer;
    QVBoxLayout* m_messageLayout;
    QTextEdit* m_messageInput;
    QPushButton* m_sendButton;
    QPushButton* m_startChatButton;
    
    // 数据
    UserInfo m_currentUser;
    int m_currentSessionId;
    DatabaseManager* m_dbManager;
    QTimer* m_messageCheckTimer;
    QTimer* m_typingTimer;
    
    // 状态
    bool m_isConnected;
    bool m_isTyping;
    QDateTime m_lastMessageTime;
};

#endif // REALCHATWIDGET_H 