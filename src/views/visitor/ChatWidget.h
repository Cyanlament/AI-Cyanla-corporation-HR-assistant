#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextBrowser>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QComboBox>
#include <QTimer>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QButtonGroup>
#include <QGroupBox>
#include "../../core/DatabaseManager.h"
#include "../../core/AIApiClient.h"

// 消息类型枚举
enum class MessageType {
    User,           // 用户消息
    Robot,          // 机器人消息
    System,         // 系统消息
    QuickReply      // 快捷回复
};

// AIHR消息结构体
struct AIMessage {
    QString content;
    MessageType type;
    QDateTime timestamp;
    QString sessionId;
};

// HR建议结构体
struct TriageAdvice {
    QString department;     // 推荐部门
    QString reason;         // 推荐理由
    QString action;         // 建议操作
    bool needAppointment;   // 是否需要预约
    bool fitness;     // 合适程度
};

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();
    
    void setDatabaseManager(DatabaseManager* dbManager);
    void setUserInfo(const QString& userId, const QString& userName);

signals:
    void requestHumanService(const QString& userId, const QString& userName, const QString& context);

private slots:
    // 消息发送相关
    void onSendMessage();
    void onQuickButtonClicked(QAbstractButton* button);
    void onInputTextChanged();
    
    // AI响应相关
    void onAIResponseReady();
    void simulateTyping();
    void onAITriageResponse(const AIDiagnosisResult& result);
    void onAIApiError(const QString& error);
    
    // 交互按钮相关
    void onActionButtonClicked();
    void onDepartmentSelected();
    void onTransferToHuman();  // 转人工服务
    
    // 功能按钮相关
    void onClearChatClicked();
    void onSaveChatClicked();
    void onSettingsClicked();

private:
    // UI设置
    void setupUI();
    void setupChatArea();
    void setupQuickButtonsArea();
    void setupInputArea();
    void setupToolBar();
    
    // 消息处理
    void addMessage(const AIMessage& message);
    void displayMessage(const AIMessage& message);
    void scrollToBottom();
    
    // AIHR逻辑
    TriageAdvice analyzeSymptoms(const QString& userInput);
    QString generateAIResponse(const QString& userInput);
    void processTriageAdvice(const TriageAdvice& advice);
    
    // 快捷按钮管理
    void setupQuickButtons();
    void addQuickButton(const QString& text, const QString& responseTemplate);
    void updateQuickButtons(const QStringList& suggestions);
    
    // 交互组件
    void addActionButtons(const QStringList& actions);
    void addDepartmentSelector(const QStringList& departments);
    void addTransferOption();  // 添加转人工选项
    void clearInteractionComponents();
    
    // 数据库操作
    void initDatabase();
    void saveChatHistory();
    void loadChatHistory();
    QString generateSessionId();
    
    // 工具方法
    QString formatTimestamp(const QDateTime& timestamp);
    QString extractKeywords(const QString& text);
    QStringList getSymptomKeywords(const QString& text);

private:
    // 主布局
    QVBoxLayout* m_mainLayout;
    
    // 工具栏
    QHBoxLayout* m_toolBarLayout;
    QPushButton* m_btnClearChat;
    QPushButton* m_btnSaveChat;
    QPushButton* m_btnSettings;
    QLabel* m_statusLabel;
    
    // 聊天显示区域
    QScrollArea* m_chatScrollArea;
    QWidget* m_chatContainer;
    QVBoxLayout* m_chatLayout;
    
    // 快捷按钮区域
    QGroupBox* m_quickButtonsGroup;
    QGridLayout* m_quickButtonsLayout;
    QButtonGroup* m_quickButtonGroup;
    
    // 动态交互区域
    QWidget* m_interactionWidget;
    QVBoxLayout* m_interactionLayout;
    
    // 输入区域
    QWidget* m_inputWidget;
    QVBoxLayout* m_inputLayout;
    QTextEdit* m_messageInput;
    QPushButton* m_btnSend;
    QPushButton* m_btnVoice;     // 语音输入（预留）
    QPushButton* m_btnEmoji;     // 表情输入（预留）
    
    // AI处理相关
    QTimer* m_typingTimer;       // 模拟打字效果
    QTimer* m_responseTimer;     // 模拟AI响应延迟
    QString m_pendingResponse;   // 待发送的AI响应
    bool m_isAITyping;          // AI是否正在输入
    AIApiClient* m_aiApiClient;  // AI API客户端
    
    // 数据存储
    QList<AIMessage> m_chatHistory;
    QString m_currentSessionId;
    QSqlDatabase m_database;
    
    // HR相关
    QStringList m_departments;   // 部门列表
    QMap<QString, QStringList> m_qualityKeywords; // 品质关键词映射
    
    // 状态管理
    bool m_isInitialized;
    int m_messageCount;
    QString m_currentContext;    // 当前对话上下文
    QString m_userId;
    QString m_userName;
    DatabaseManager* m_dbManager;
};

#endif // CHATWIDGET_H 
