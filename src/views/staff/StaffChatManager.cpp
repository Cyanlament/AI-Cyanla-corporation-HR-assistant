#include "StaffChatManager.h"
#include "../common/UIStyleManager.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QApplication>
#include <QKeyEvent>
#include <QSplitter>

StaffChatManager::StaffChatManager(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_leftPanel(nullptr)
    , m_rightPanel(nullptr)
    , m_activeSessionsList(nullptr)
    , m_waitingSessionsList(nullptr)
    , m_messageScrollArea(nullptr)
    , m_messageContainer(nullptr)
    , m_messageLayout(nullptr)
    , m_messageInput(nullptr)
    , m_sendButton(nullptr)
    , m_closeSessionButton(nullptr)
    , m_currentSessionId(-1)
    , m_dbManager(DatabaseManager::instance())
    , m_sessionCheckTimer(new QTimer(this))
    , m_messageCheckTimer(new QTimer(this))
{
    setupUI();
    
    // 连接数据库信号
    connect(m_dbManager, &DatabaseManager::newMessageReceived, 
            this, &StaffChatManager::onMessageReceived);
    connect(m_dbManager, &DatabaseManager::sessionCreated, 
            this, &StaffChatManager::onSessionCreated);
    connect(m_dbManager, &DatabaseManager::sessionUpdated, 
            this, &StaffChatManager::onSessionUpdated);
    
    // 定时检查新会话和消息
    connect(m_sessionCheckTimer, &QTimer::timeout, this, &StaffChatManager::checkForNewSessions);
    connect(m_messageCheckTimer, &QTimer::timeout, this, &StaffChatManager::checkForNewMessages);
    
    m_sessionCheckTimer->start(3000); // 每3秒检查新会话
    m_messageCheckTimer->start(2000); // 每2秒检查新消息
}

StaffChatManager::~StaffChatManager()
{
    if (m_currentUser.id > 0) {
        m_dbManager->updateUserOnlineStatus(m_currentUser.id, false);
    }
}

void StaffChatManager::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    // 创建分割器
    m_splitter = new QSplitter(Qt::Vertical, this);
    
    setupSessionList();
    setupChatArea();
    
    m_splitter->addWidget(m_leftPanel);
    m_splitter->addWidget(m_rightPanel);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 2);
    
    m_mainLayout->addWidget(m_splitter);
}

void StaffChatManager::setupSessionList()
{
    m_leftPanel = new QWidget;
    m_leftLayout = new QHBoxLayout(m_leftPanel);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftLayout->setSpacing(10);
    
    // 统计信息
    // m_statsLabel = new QLabel("客服聊天", this);
    // UIStyleManager::applyLabelStyle(m_statsLabel, "title");
    // m_statsLabel->setAlignment(Qt::AlignCenter);
    // m_leftLayout->addWidget(m_statsLabel);
    
    // 等待接入的会话
    m_waitingSessionsGroup = new QGroupBox("等待接入", this);
    UIStyleManager::applyGroupBoxStyle(m_waitingSessionsGroup);
    QVBoxLayout* waitingLayout = new QVBoxLayout(m_waitingSessionsGroup);
    
    m_waitingSessionsList = new QListWidget(this);
    m_waitingSessionsList->setMaximumHeight(150);
    waitingLayout->addWidget(m_waitingSessionsList);
    
    connect(m_waitingSessionsList, &QListWidget::itemDoubleClicked, [this](QListWidgetItem* item) {
        int sessionId = m_itemToSessionId.value(item, -1);
        if (sessionId > 0) {
            onAcceptSession(sessionId);
        }
    });
    
    m_leftLayout->addWidget(m_waitingSessionsGroup);
    
    // 进行中的会话
    m_activeSessionsGroup = new QGroupBox("进行中的对话", this);
    UIStyleManager::applyGroupBoxStyle(m_activeSessionsGroup);
    QVBoxLayout* activeLayout = new QVBoxLayout(m_activeSessionsGroup);
    
    m_activeSessionsList = new QListWidget(this);
    activeLayout->addWidget(m_activeSessionsList);
    
    connect(m_activeSessionsList, &QListWidget::itemClicked, 
            this, &StaffChatManager::onSessionSelectionChanged);
    
    m_leftLayout->addWidget(m_activeSessionsGroup);
}

void StaffChatManager::setupChatArea()
{
    m_rightPanel = new QWidget;
    m_rightLayout = new QVBoxLayout(m_rightPanel);
    m_rightLayout->setContentsMargins(0, 0, 0, 0);
    m_rightLayout->setSpacing(10);
    
    // 聊天标题
    m_chatTitleLabel = new QLabel("当前会话", this);
    UIStyleManager::applyLabelStyle(m_chatTitleLabel, "subtitle");
    m_chatTitleLabel->setAlignment(Qt::AlignCenter);
    m_rightLayout->addWidget(m_chatTitleLabel);
    
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
    UIStyleManager::applyScrollAreaStyle(m_messageScrollArea);
    
    m_rightLayout->addWidget(m_messageScrollArea);
    
    // 输入区域
    QWidget* inputWidget = new QWidget(this);
    QHBoxLayout* inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(10);
    
    // 消息输入框
    m_messageInput = new QTextEdit(this);
    // m_messageInput->setMaximumHeight(80);
    m_messageInput->setPlaceholderText("输入回复消息...");
    UIStyleManager::applyTextEditStyle(m_messageInput);
    m_messageInput->setEnabled(false);
    
    // 按钮区域
    QWidget* buttonWidget = new QWidget;
    QVBoxLayout* buttonLayout = new QVBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(5);
    
    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setMinimumWidth(80);
    UIStyleManager::applyButtonStyle(m_sendButton, "primary");
    m_sendButton->setEnabled(false);
    
    m_closeSessionButton = new QPushButton("结束对话", this);
    m_closeSessionButton->setMinimumWidth(80);
    UIStyleManager::applyButtonStyle(m_closeSessionButton, "error");
    m_closeSessionButton->setEnabled(false);
    
    buttonLayout->addWidget(m_sendButton);
    buttonLayout->addWidget(m_closeSessionButton);
    buttonLayout->addStretch();
    
    connect(m_messageInput, &QTextEdit::textChanged, [this]() {
        bool hasText = !m_messageInput->toPlainText().trimmed().isEmpty();
        m_sendButton->setEnabled(hasText && m_currentSessionId > 0);
    });
    
    connect(m_sendButton, &QPushButton::clicked, this, &StaffChatManager::onSendMessage);
    connect(m_closeSessionButton, &QPushButton::clicked, [this]() {
        if (m_currentSessionId > 0) {
            onCloseSession(m_currentSessionId);
        }
    });
    
    // 键盘事件
    m_messageInput->installEventFilter(this);
    
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(buttonWidget);
    
    m_rightLayout->addWidget(inputWidget);
}

void StaffChatManager::setCurrentUser(const UserInfo& user)
{
    m_currentUser = user;
    
    // 更新在线状态
    m_dbManager->updateUserOnlineStatus(user.id, true);
    
    // 更新统计标签
    // m_statsLabel->setText(QString("客服工作台 - %1").arg(user.realName.isEmpty() ? user.username : user.realName));
    
    // 加载会话列表
    loadSessionList();
}

void StaffChatManager::loadSessionList()
{
    if (m_currentUser.id <= 0) return;
    
    // 清空列表
    m_activeSessionsList->clear();
    m_waitingSessionsList->clear();
    m_itemToSessionId.clear();
    m_sessions.clear();
    
    // 获取活跃会话
    QList<ChatSession> activeSessions = m_dbManager->getActiveSessions();
    
    for (const ChatSession& session : activeSessions) {
        m_sessions[session.id] = session;
        
        QListWidgetItem* item = createSessionItem(session);
        
        if (session.status == 2) {
            // 等待中的会话
            m_waitingSessionsList->addItem(item);
        } else if (session.status == 1 && session.staffId == m_currentUser.id) {
            // 自己负责的进行中会话
            m_activeSessionsList->addItem(item);
        }
        
        m_itemToSessionId[item] = session.id;
    }
    
    // 更新统计信息
    int waitingCount = m_waitingSessionsList->count();
    int activeCount = m_activeSessionsList->count();
    m_waitingSessionsGroup->setTitle(QString("等待接入 (%1)").arg(waitingCount));
    m_activeSessionsGroup->setTitle(QString("当前对话 (%1)").arg(activeCount));
}

QListWidgetItem* StaffChatManager::createSessionItem(const ChatSession& session)
{
    QListWidgetItem* item = new QListWidgetItem;
    
    QString text = QString("%1\n最后消息: %2")
                   .arg(session.patientName)
                   .arg(formatTime(session.lastMessageAt));
    
    item->setText(text);
    updateSessionItemStyle(item, session);
    
    return item;
}

void StaffChatManager::updateSessionItemStyle(QListWidgetItem* item, const ChatSession& session)
{
    if (session.status == 2) {
        // 等待中 - 橙色
        item->setBackground(QBrush(QColor("#FFF3CD")));
        item->setForeground(QBrush(QColor("#856404")));
    } else if (session.status == 1) {
        // 进行中 - 绿色
        item->setBackground(QBrush(QColor("#D4EDDA")));
        item->setForeground(QBrush(QColor("#155724")));
    }
}

void StaffChatManager::onSessionSelectionChanged()
{
    QListWidgetItem* currentItem = m_activeSessionsList->currentItem();
    if (!currentItem) return;
    
    int sessionId = m_itemToSessionId.value(currentItem, -1);
    if (sessionId <= 0) return;
    
    m_currentSessionId = sessionId;
    ChatSession session = m_sessions.value(sessionId);
    
    // 更新聊天标题
    m_chatTitleLabel->setText(QString("与 %1 的对话").arg(session.patientName));
    
    // 启用输入
    m_messageInput->setEnabled(true);
    m_closeSessionButton->setEnabled(true);
    
    // 清空消息区域
    QLayoutItem* child;
    while ((child = m_messageLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
    m_messageLayout->addStretch();
    
    // 加载聊天历史
    loadChatHistory(sessionId);
    
    // 标记消息为已读
    m_dbManager->markSessionAsRead(sessionId, m_currentUser.id);
}

void StaffChatManager::loadChatHistory(int sessionId)
{
    QList<ChatMessage> messages = m_dbManager->getChatMessages(sessionId);
    
    for (const ChatMessage& message : messages) {
        addMessage(message);
    }
}

void StaffChatManager::onAcceptSession(int sessionId)
{
    if (m_dbManager->updateChatSession(sessionId, m_currentUser.id)) {
        // 刷新会话列表
        refreshSessionList();
        
        // 自动选择这个会话
        for (int i = 0; i < m_activeSessionsList->count(); ++i) {
            QListWidgetItem* item = m_activeSessionsList->item(i);
            if (m_itemToSessionId.value(item) == sessionId) {
                m_activeSessionsList->setCurrentItem(item);
                onSessionSelectionChanged();
                break;
            }
        }
    }
}

void StaffChatManager::onCloseSession(int sessionId)
{
    int ret = QMessageBox::question(this, "确认", "确定要结束这个对话吗？", 
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_dbManager->closeChatSession(sessionId)) {
            // 清空当前选择
            m_currentSessionId = -1;
            m_chatTitleLabel->setText("普通会话");
            m_messageInput->setEnabled(false);
            m_closeSessionButton->setEnabled(false);
            
            // 清空消息区域
            QLayoutItem* child;
            while ((child = m_messageLayout->takeAt(0)) != nullptr) {
                if (child->widget()) {
                    delete child->widget();
                }
                delete child;
            }
            m_messageLayout->addStretch();
            
            // 刷新会话列表
            refreshSessionList();
        }
    }
}

void StaffChatManager::onSendMessage()
{
    QString content = m_messageInput->toPlainText().trimmed();
    if (content.isEmpty() || m_currentSessionId <= 0) {
        return;
    }
    
    // 发送消息到数据库
    int messageId = m_dbManager->sendMessage(m_currentSessionId, m_currentUser.id, content);
    
    if (messageId > 0) {
        m_messageInput->clear();
        
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

void StaffChatManager::onMessageReceived(const ChatMessage& message)
{
    // 如果是当前会话的消息且不是自己发送的，显示消息
    if (message.sessionId == m_currentSessionId && message.senderId != m_currentUser.id) {
        addMessage(message);
        m_dbManager->markMessageAsRead(message.id);
    }
    
    // 刷新会话列表以更新最后消息时间
    refreshSessionList();
}

void StaffChatManager::onSessionCreated(const ChatSession& session)
{
    // 新会话创建，刷新列表
    refreshSessionList();
}

void StaffChatManager::onSessionUpdated(const ChatSession& session)
{
    // 会话更新，刷新列表
    refreshSessionList();
}

void StaffChatManager::checkForNewSessions()
{
    refreshSessionList();
}

void StaffChatManager::checkForNewMessages()
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
}

void StaffChatManager::refreshSessionList()
{
    // 保存当前选择
    int currentSelectedId = m_currentSessionId;
    
    // 重新加载
    loadSessionList();
    
    // 恢复选择
    if (currentSelectedId > 0) {
        for (int i = 0; i < m_activeSessionsList->count(); ++i) {
            QListWidgetItem* item = m_activeSessionsList->item(i);
            if (m_itemToSessionId.value(item) == currentSelectedId) {
                m_activeSessionsList->setCurrentItem(item);
                break;
            }
        }
    }
}

void StaffChatManager::addMessage(const ChatMessage& message)
{
    QWidget* messageBubble = createMessageBubble(message);
    
    // 移除stretch，添加消息，再添加stretch
    QLayoutItem* stretch = m_messageLayout->takeAt(m_messageLayout->count() - 1);
    m_messageLayout->addWidget(messageBubble);
    m_messageLayout->addItem(stretch);
    
    // 滚动到底部
    QTimer::singleShot(50, this, &StaffChatManager::scrollToBottom);
}

QWidget* StaffChatManager::createMessageBubble(const ChatMessage& message)
{
    QWidget* bubble = new QWidget;
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

void StaffChatManager::scrollToBottom()
{
    QScrollBar* scrollBar = m_messageScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

QString StaffChatManager::formatTime(const QDateTime& time)
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

bool StaffChatManager::eventFilter(QObject* obj, QEvent* event)
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
