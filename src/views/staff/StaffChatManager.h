#ifndef STAFFCHATMANAGER_H
#define STAFFCHATMANAGER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QGroupBox>
#include <QDateTime>
#include "../../core/DatabaseManager.h"

class StaffChatManager : public QWidget
{
    Q_OBJECT

public:
    explicit StaffChatManager(QWidget *parent = nullptr);
    ~StaffChatManager();
    
    void setCurrentUser(const UserInfo& user);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSessionSelectionChanged();
    void onSendMessage();
    void onAcceptSession(int sessionId);
    void onCloseSession(int sessionId);
    void onMessageReceived(const ChatMessage& message);
    void onSessionCreated(const ChatSession& session);
    void onSessionUpdated(const ChatSession& session);
    void checkForNewSessions();
    void checkForNewMessages();
    void refreshSessionList();

private:
    void setupUI();
    void setupSessionList();
    void setupChatArea();
    void setupWaitingList();
    void loadSessionList();
    void loadChatHistory(int sessionId);
    void addMessage(const ChatMessage& message);
    void scrollToBottom();
    QWidget* createMessageBubble(const ChatMessage& message);
    QListWidgetItem* createSessionItem(const ChatSession& session);
    QString formatTime(const QDateTime& time);
    void updateSessionItemStyle(QListWidgetItem* item, const ChatSession& session);

    // UI 组件
    QHBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // 左侧会话列表
    QWidget* m_leftPanel;
    QHBoxLayout* m_leftLayout;
    QGroupBox* m_activeSessionsGroup;
    QListWidget* m_activeSessionsList;
    QGroupBox* m_waitingSessionsGroup;
    QListWidget* m_waitingSessionsList;
    QLabel* m_statsLabel;
    
    // 右侧聊天区域
    QWidget* m_rightPanel;
    QVBoxLayout* m_rightLayout;
    QLabel* m_chatTitleLabel;
    QScrollArea* m_messageScrollArea;
    QWidget* m_messageContainer;
    QVBoxLayout* m_messageLayout;
    QTextEdit* m_messageInput;
    QPushButton* m_sendButton;
    QPushButton* m_closeSessionButton;
    
    // 数据
    UserInfo m_currentUser;
    int m_currentSessionId;
    DatabaseManager* m_dbManager;
    QTimer* m_sessionCheckTimer;
    QTimer* m_messageCheckTimer;
    
    // 会话映射
    QMap<int, ChatSession> m_sessions;
    QMap<QListWidgetItem*, int> m_itemToSessionId;
};

#endif // STAFFCHATMANAGER_H 
