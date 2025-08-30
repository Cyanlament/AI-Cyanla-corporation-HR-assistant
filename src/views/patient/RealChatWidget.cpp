#include "RealChatWidget.h"
#include "../common/UIStyleManager.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QApplication>
#include <QKeyEvent>

RealChatWidget::RealChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_statusLabel(nullptr)
    , m_messageScrollArea(nullptr)
    , m_messageContainer(nullptr)
    , m_messageLayout(nullptr)
    , m_messageInput(nullptr)
    , m_sendButton(nullptr)
    , m_startChatButton(nullptr)
    , m_currentSessionId(-1)
    , m_dbManager(DatabaseManager::instance())
    , m_messageCheckTimer(new QTimer(this))
    , m_typingTimer(new QTimer(this))
    , m_isConnected(false)
    , m_isTyping(false)
{
    setupUI();
    
    // 连接数据库信号
    connect(m_dbManager, &DatabaseManager::newMessageReceived, 
            this, &RealChatWidget::onMessageReceived);
    connect(m_dbManager, &DatabaseManager::sessionCreated, 
            this, &RealChatWidget::onSessionCreated);
    connect(m_dbManager, &DatabaseManager::sessionUpdated, 
            this, &RealChatWidget::onSessionUpdated);
    
    // 定时检查新消息
    connect(m_messageCheckTimer, &QTimer::timeout, this, &RealChatWidget::checkForNewMessages);
    m_messageCheckTimer->start(2000); // 每2秒检查一次
    
    // 用户输入计时器
    connect(m_typingTimer, &QTimer::timeout, this, &RealChatWidget::onUserInput);
    m_typingTimer->setSingleShot(true);
}

RealChatWidget::~RealChatWidget()
{
    if (m_currentUser.id > 0) {
        m_dbManager->updateUserOnlineStatus(m_currentUser.id, false);
    }
}

void RealChatWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    // 状态栏
    m_statusLabel = new QLabel("客服聊天", this);
    UIStyleManager::applyLabelStyle(m_statusLabel, "title");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_statusLabel);
    
    setupMessageArea();
    setupInputArea();
    
    updateConnectionStatus();
}

void RealChatWidget::setupMessageArea()
{
    // 消息显示区域
    m_messageScrollArea = new QScrollArea(this);
    m_messageScrollArea->setWidgetResizable(true);
    m_messageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_messageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_messageContainer = new QWidget;
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setContentsMargins(10, 10, 10, 10);
    m_messageLayout->setSpacing(10);
    m_messageLayout->addStretch();
    
    m_messageScrollArea->setWidget(m_messageContainer);
    m_messageScrollArea->setMinimumHeight(400);
    
    UIStyleManager::applyScrollAreaStyle(m_messageScrollArea);
    
    m_mainLayout->addWidget(m_messageScrollArea);
}

void RealChatWidget::setupInputArea()
{
    // 开始聊天按钮
    m_startChatButton = new QPushButton("🚀 开始咨询", this);
    UIStyleManager::applyButtonStyle(m_startChatButton, "primary");
    connect(m_startChatButton, &QPushButton::clicked, this, &RealChatWidget::startNewChat);
    m_mainLayout->addWidget(m_startChatButton);
    
    // 输入区域
    QWidget* inputWidget = new QWidget(this);
    QVBoxLayout* inputLayout = new QVBoxLayout(inputWidget);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(10);
    
    // 消息输入框
    m_messageInput = new QTextEdit(this);
    // m_messageInput->setMaximumHeight(80);
    m_messageInput->setPlaceholderText("请输入您的问题...");
    UIStyleManager::applyTextEditStyle(m_messageInput);
    

    QHBoxLayout* sendLayout = new QHBoxLayout(inputWidget);
    // 发送按钮
    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setMinimumWidth(80);
    UIStyleManager::applyButtonStyle(m_sendButton, "primary");
    m_sendButton->setEnabled(false);
    sendLayout->addStretch();
    sendLayout->addWidget(m_sendButton);


    connect(m_messageInput, &QTextEdit::textChanged, [this]() {
        bool hasText = !m_messageInput->toPlainText().trimmed().isEmpty();
        m_sendButton->setEnabled(hasText && m_isConnected);
        
        if (hasText && !m_isTyping) {
            m_isTyping = true;
            m_typingTimer->start(3000); // 3秒后停止输入状态
        }
    });
    
    connect(m_sendButton, &QPushButton::clicked, this, &RealChatWidget::onSendMessage);
    
    // 键盘事件
    m_messageInput->installEventFilter(this);
    
    inputLayout->addWidget(m_messageInput);
    // inputLayout->addWidget(m_sendButton);
    inputLayout->addLayout(sendLayout);
    
    inputWidget->setVisible(false); // 初始隐藏
    m_mainLayout->addWidget(inputWidget);
}

void RealChatWidget::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;
    
    // 更新在线状态
    if (m_dbManager) {
        m_dbManager->updateUserOnlineStatus(user.id, true);
    }
    
    // 加载聊天历史
    loadChatHistory();
}

void RealChatWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    
    if (m_dbManager) {
        // 连接数据库信号
        connect(m_dbManager, &DatabaseManager::newMessageReceived, 
                this, &RealChatWidget::onMessageReceived);
        connect(m_dbManager, &DatabaseManager::sessionCreated, 
                this, &RealChatWidget::onSessionCreated);
        connect(m_dbManager, &DatabaseManager::sessionUpdated, 
                this, &RealChatWidget::onSessionUpdated);
    }
    
    updateConnectionStatus();
}

void RealChatWidget::startNewChat()
{
    if (m_currentUser.id <= 0) {
        QMessageBox::warning(this, "错误", "请先登录！");
        return;
    }
    
    // 创建新的聊天会话
    m_currentSessionId = m_dbManager->createChatSession(m_currentUser.id);
    
    if (m_currentSessionId > 0) {
        m_isConnected = true;
        m_startChatButton->setVisible(false);
        m_mainLayout->itemAt(m_mainLayout->count() - 1)->widget()->setVisible(true);
        
        updateConnectionStatus();
        
        // 添加欢迎消息
        ChatMessage welcomeMsg;
        welcomeMsg.id = 0;
        welcomeMsg.sessionId = m_currentSessionId;
        welcomeMsg.senderId = 0;
        welcomeMsg.senderName = "系统";
        welcomeMsg.senderRole = "system";
        welcomeMsg.content = "您好！请描述您的问题，我们会尽快为您安排客服。";
        welcomeMsg.timestamp = QDateTime::currentDateTime();
        welcomeMsg.messageType = 1;
        welcomeMsg.isRead = 1;
        
        addMessage(welcomeMsg);
    } else {
        QMessageBox::warning(this, "错误", "无法创建聊天会话！");
    }
}

void RealChatWidget::onSendMessage()
{
    QString content = m_messageInput->toPlainText().trimmed();
    if (content.isEmpty() || m_currentSessionId <= 0) {
        return;
    }
    
    // 发送消息到数据库
    int messageId = m_dbManager->sendMessage(m_currentSessionId, m_currentUser.id, content);
    
    if (messageId > 0) {
        m_messageInput->clear();
        m_isTyping = false;
        
        // 立即显示自己的消息
        ChatMessage message;
        message.id = messageId;
        message.sessionId = m_currentSessionId;
        message.senderId = m_currentUser.id;
        message.senderName = m_currentUser.realName.isEmpty() ? m_currentUser.username : m_currentUser.realName;
        message.senderRole = m_currentUser.role;
        message.content = content;
        message.timestamp = QDateTime::currentDateTime();
        message.messageType = 0;
        message.isRead = 1;
        
        addMessage(message);
    }
}

void RealChatWidget::onMessageReceived(const ChatMessage& message)
{
    // 只显示当前会话的消息，且不是自己发送的
    if (message.sessionId == m_currentSessionId && message.senderId != m_currentUser.id) {
        addMessage(message);
        
        // 标记消息为已读
        m_dbManager->markMessageAsRead(message.id);
    }
}

void RealChatWidget::onSessionCreated(const ChatSession& session)
{
    if (session.patientId == m_currentUser.id) {
        updateConnectionStatus();
    }
}

void RealChatWidget::onSessionUpdated(const ChatSession& session)
{
    if (session.id == m_currentSessionId) {
        updateConnectionStatus();
    }
}

void RealChatWidget::checkForNewMessages()
{
    if (m_currentSessionId <= 0) return;
    
    // 获取未读消息
    QList<ChatMessage> unreadMessages = m_dbManager->getUnreadMessages(m_currentUser.id);
    
    for (const ChatMessage& message : unreadMessages) {
        if (message.sessionId == m_currentSessionId && message.senderId != m_currentUser.id) {
            addMessage(message);
            m_dbManager->markMessageAsRead(message.id);
        }
    }
    
    // 同时检查会话状态是否有变化
    updateConnectionStatus();
}

void RealChatWidget::onUserInput()
{
    m_isTyping = false;
}

void RealChatWidget::addMessage(const ChatMessage& message)
{
    QWidget* messageBubble = createMessageBubble(message);
    
    // 移除stretch，添加消息，再添加stretch
    QLayoutItem* stretch = m_messageLayout->takeAt(m_messageLayout->count() - 1);
    m_messageLayout->addWidget(messageBubble);
    m_messageLayout->addItem(stretch);
    
    // 滚动到底部
    QTimer::singleShot(50, this, &RealChatWidget::scrollToBottom);
    
    m_lastMessageTime = message.timestamp;
    
    // 如果是系统消息（比如客服接入通知），更新连接状态
    if (message.messageType == 1 && message.content.contains("已接入")) {
        QTimer::singleShot(100, this, &RealChatWidget::updateConnectionStatus);
    }
}

QWidget* RealChatWidget::createMessageBubble(const ChatMessage& message)
{
    QWidget* bubble = new QWidget;
    bubble->setStyleSheet(R"(
        QWidget {
            background-color: white;
        })");
    QHBoxLayout* layout = new QHBoxLayout(bubble);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建消息内容
    QFrame* messageFrame = new QFrame;
    QVBoxLayout* frameLayout = new QVBoxLayout(messageFrame);
    frameLayout->setContentsMargins(12, 8, 12, 8);
    frameLayout->setSpacing(4);
    
    // 发送者名称和时间
    QLabel* headerLabel = new QLabel;
    QString headerText = QString("%1  %2").arg(message.senderName).arg(formatTime(message.timestamp));
    headerLabel->setText(headerText);
    headerLabel->setStyleSheet("font-size: 11px; color: #8E8E93;");
    frameLayout->addWidget(headerLabel);
    
    // 消息内容
    QLabel* contentLabel = new QLabel(message.content);
    contentLabel->setWordWrap(true);
    contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    frameLayout->addWidget(contentLabel);
    
    // 根据发送者设置样式和对齐
    if (message.senderId == m_currentUser.id) {
        // 自己的消息 - 右对齐，蓝色
        layout->addStretch();
        layout->addWidget(messageFrame);
        
        messageFrame->setStyleSheet(R"(
            QFrame {
                background-color: #007AFF;
                border-radius: 18px;
                max-width: 300px;
            }
        )");
        contentLabel->setStyleSheet("color: white; font-size: 14px;");
        headerLabel->setStyleSheet("font-size: 11px; color: rgba(255,255,255,0.8);");
        
    } else if (message.messageType == 1) {
        // 系统消息 - 居中，灰色
        layout->addStretch();
        layout->addWidget(messageFrame);
        layout->addStretch();
        
        messageFrame->setStyleSheet(R"(
            QFrame {
                background-color: #F2F2F7;
                border-radius: 18px;
                max-width: 250px;
            }
        )");
        contentLabel->setStyleSheet("color: #8E8E93; font-size: 13px; text-align: center;");
        
    } else {
        // 对方的消息 - 左对齐，灰色
        layout->addWidget(messageFrame);
        layout->addStretch();
        
        messageFrame->setStyleSheet(R"(
            QFrame {
                background-color: #E5E5EA;
                border-radius: 18px;
                max-width: 300px;
            }
        )");
        contentLabel->setStyleSheet("color: #000000; font-size: 14px;");
    }
    
    return bubble;
}

void RealChatWidget::scrollToBottom()
{
    QScrollBar* scrollBar = m_messageScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void RealChatWidget::updateConnectionStatus()
{
    if (m_currentSessionId <= 0) {
        m_statusLabel->setText(" 客服聊天 - 未连接");
        return;
    }
    
    ChatSession session = m_dbManager->getChatSession(m_currentSessionId);
    
    switch (session.status) {
        case 0:
            m_statusLabel->setText(" 客服聊天 - 会话已结束");
            break;
        case 1:
            if (!session.staffName.isEmpty()) {
                m_statusLabel->setText(QString("与客服 %1 对话中").arg(session.staffName));
            } else {
                m_statusLabel->setText(" 与客服对话中");
            }
            break;
        case 2:
            m_statusLabel->setText(" 客服聊天 - 等待客服接入...");
            break;
        default:
            m_statusLabel->setText(" 客服聊天");
            break;
    }
}

void RealChatWidget::loadChatHistory()
{
    if (m_currentUser.id <= 0) return;
    
    // 获取用户的最近会话
    QList<ChatSession> sessions = m_dbManager->getPatientSessions(m_currentUser.id);
    
    if (!sessions.isEmpty()) {
        ChatSession lastSession = sessions.first();
        if (lastSession.status > 0) {
            // 如果有进行中的会话，自动加载
            m_currentSessionId = lastSession.id;
            m_isConnected = true;
            m_startChatButton->setVisible(false);
            m_mainLayout->itemAt(m_mainLayout->count() - 1)->widget()->setVisible(true);
            
            // 加载消息历史
            QList<ChatMessage> messages = m_dbManager->getChatMessages(m_currentSessionId);
            for (const ChatMessage& message : messages) {
                addMessage(message);
            }
            
            updateConnectionStatus();
        }
    }
}

QString RealChatWidget::formatTime(const QDateTime& time)
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = time.secsTo(now);
    
    if (secs < 60) {
        return "刚刚";
    } else if (secs < 3600) {
        return QString("%1分钟前").arg(secs / 60);
    } else if (time.date() == now.date()) {
        return time.toString("hh:mm");
    } else {
        return time.toString("MM-dd hh:mm");
    }
}

bool RealChatWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_messageInput && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                // 不是 Shift+Enter，发送消息
                onSendMessage();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

// MOC generated automatically 
