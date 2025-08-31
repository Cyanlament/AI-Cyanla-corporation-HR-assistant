#include "ChatWidget.h"
#include <QGroupBox>
#include <QScrollBar>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QSqlError>
#include <QUuid>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QDebug>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_toolBarLayout(nullptr)
    , m_btnClearChat(nullptr)
    , m_btnSaveChat(nullptr)
    , m_btnSettings(nullptr)
    , m_statusLabel(nullptr)
    , m_chatScrollArea(nullptr)
    , m_chatContainer(nullptr)
    , m_chatLayout(nullptr)
    , m_quickButtonsGroup(nullptr)
    , m_quickButtonsLayout(nullptr)
    , m_quickButtonGroup(nullptr)
    , m_interactionWidget(nullptr)
    , m_interactionLayout(nullptr)
    , m_inputWidget(nullptr)
    , m_inputLayout(nullptr)
    , m_messageInput(nullptr)
    , m_btnSend(nullptr)
    , m_btnVoice(nullptr)
    , m_btnEmoji(nullptr)
    , m_typingTimer(nullptr)
    , m_responseTimer(nullptr)
    , m_isAITyping(false)
    , m_isInitialized(false)
    , m_messageCount(0)
    , m_dbManager(nullptr)
    , m_aiApiClient(new AIApiClient(this))
{
    initDatabase();
    setupUI();
    setupQuickButtons();
    
    m_currentSessionId = generateSessionId();
    m_isInitialized = true;
    
    // 连接AI API客户端信号
    connect(m_aiApiClient, &AIApiClient::chatResponseReceived,
            this, &ChatWidget::onAIChatResponse);
    connect(m_aiApiClient, &AIApiClient::apiError,
            this, &ChatWidget::onAIApiError);
    connect(m_aiApiClient, &AIApiClient::requestStarted, [this]() {
        m_isAITyping = true;
        m_statusLabel->setText("智能HR助手正在分析中...");
        m_btnSend->setEnabled(false);
    });
    connect(m_aiApiClient, &AIApiClient::requestFinished, [this]() {
        m_isAITyping = false;
        m_statusLabel->setText("智能HR助手");
        m_btnSend->setEnabled(true);
    });
    
    // 发送欢迎消息
    QTimer::singleShot(500, [this]() {
        AIMessage welcomeMsg;
        welcomeMsg.content = "您好！我是青蓝公司智能HR助手\n\n我可以帮助您：\n• 告诉我您的性格特长，推荐合适部门\n• 解答公司相关问题\n•  识别您与公司及部门合适度，还能帮您安排面试 \n• 转接人工客服\n\n请描述您的问题开始咨询！";
        welcomeMsg.type = MessageType::Robot;
        welcomeMsg.timestamp = QDateTime::currentDateTime();
        welcomeMsg.sessionId = m_currentSessionId;
        addMessage(welcomeMsg);
    });
}

ChatWidget::~ChatWidget()
{
    if (m_database.isOpen()) {
        saveChatHistory();
        m_database.close();
    }
}

void ChatWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
}

void ChatWidget::setUserInfo(const QString& userId, const QString& userName)
{
    m_userId = userId;
    m_userName = userName;
    qDebug() << "ChatWidget 设置用户信息 - ID:" << userId << "名称:" << userName;
}

void ChatWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    setupToolBar();
    setupChatArea();
    setupQuickButtonsArea();
    setupInputArea();
    
    // 应用整体样式
    setStyleSheet(R"(
        ChatWidget {
            background-color: #F8F9FA;
        }
    )");
}

void ChatWidget::setupToolBar()
{
    // 工具栏
    m_toolBarLayout = new QHBoxLayout;
    
    m_statusLabel = new QLabel("智能HR助手");
    m_statusLabel->setStyleSheet(R"(
        QLabel {
            font-size: 18px;
            font-weight: bold;
            color: #1D1D1F;
            padding: 5px;
        }
    )");
    
    m_btnClearChat = new QPushButton("清空");
    m_btnSaveChat = new QPushButton("保存");
    m_btnSettings = new QPushButton("设置");
    m_btnSaveChat->hide();
    m_btnSettings->hide();
    
    QString toolButtonStyle = R"(
        QPushButton {
            background-color: #F2F2F7;
            border: 1px solid #D1D1D6;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 13px;
            color: #1D1D1F;
        }
        QPushButton:hover {
            background-color: #E5E5EA;
        }
        QPushButton:pressed {
            background-color: #D1D1D6;
        }
    )";
    
    m_btnClearChat->setStyleSheet(toolButtonStyle);
    m_btnSaveChat->setStyleSheet(toolButtonStyle);
    m_btnSettings->setStyleSheet(toolButtonStyle);
    
    connect(m_btnClearChat, &QPushButton::clicked, this, &ChatWidget::onClearChatClicked);
    connect(m_btnSaveChat, &QPushButton::clicked, this, &ChatWidget::onSaveChatClicked);
    connect(m_btnSettings, &QPushButton::clicked, this, &ChatWidget::onSettingsClicked);
    
    m_toolBarLayout->addWidget(m_statusLabel);
    m_toolBarLayout->addStretch();
    m_toolBarLayout->addWidget(m_btnClearChat);
    m_toolBarLayout->addWidget(m_btnSaveChat);
    m_toolBarLayout->addWidget(m_btnSettings);
    
    m_mainLayout->addLayout(m_toolBarLayout);
}

void ChatWidget::setupChatArea()
{
    // 聊天显示区域
    m_chatScrollArea = new QScrollArea;
    m_chatScrollArea->setWidgetResizable(true);
    m_chatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_chatScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_chatContainer = new QWidget;
    m_chatLayout = new QVBoxLayout(m_chatContainer);
    m_chatLayout->setContentsMargins(10, 10, 10, 10);
    m_chatLayout->setSpacing(15);
    m_chatLayout->addStretch(); // 消息从底部开始
    
    m_chatScrollArea->setWidget(m_chatContainer);
    m_chatScrollArea->setMinimumHeight(400);
    
    m_chatScrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: white;
            border: 1px solid #E5E5EA;
            border-radius: 12px;
        }
        QScrollArea QWidget {
            background-color: white;
        }
        QScrollBar:vertical {
            background-color: #F2F2F7;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background-color: #C7C7CC;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #A8A8AF;
        }
    )");
    
    m_mainLayout->addWidget(m_chatScrollArea);
}

void ChatWidget::setupQuickButtonsArea()
{
    // 快捷按钮区域
    m_quickButtonsGroup = new QGroupBox("快捷咨询");
    m_quickButtonsGroup->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            color: #1D1D1F;
            border: 1px solid #E5E5EA;
            border-radius: 8px;
            margin-top: 8px;
            padding-top: 8px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 8px 0 8px;
        }
    )");
    
    m_quickButtonsLayout = new QGridLayout(m_quickButtonsGroup);
    m_quickButtonsLayout->setSpacing(8);
    m_quickButtonGroup = new QButtonGroup(this);
    connect(m_quickButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(onQuickButtonClicked(QAbstractButton*)));
    
    m_mainLayout->addWidget(m_quickButtonsGroup);
    m_quickButtonsGroup->hide();
    
    // 动态交互区域
    m_interactionWidget = new QWidget;
    m_interactionLayout = new QVBoxLayout(m_interactionWidget);
    m_interactionLayout->setContentsMargins(0, 0, 0, 0);
    m_interactionWidget->hide(); // 初始隐藏
    
    m_mainLayout->addWidget(m_interactionWidget);
}

void ChatWidget::setupInputArea()
{
    // 输入区域
    m_inputWidget = new QWidget;
    m_inputLayout = new QVBoxLayout(m_inputWidget);
    m_inputLayout->setContentsMargins(0, 0, 0, 0);
    m_inputLayout->setSpacing(10);
    
    m_messageInput = new QTextEdit;
    m_messageInput->setMaximumHeight(100);//标记一下
    m_messageInput->setPlaceholderText("请描述您的问题...");
    
    QHBoxLayout* sendLayout = new QHBoxLayout(m_inputWidget);
    m_btnSend = new QPushButton("发送");
    m_btnVoice = new QPushButton("🎤");
    m_btnEmoji = new QPushButton("😊");
    m_btnVoice->hide();
    m_btnEmoji->hide();
    sendLayout->addStretch();
    sendLayout->addWidget(m_btnSend);

    
    m_btnSend->setFixedSize(80, 36);
    m_btnVoice->setFixedSize(36, 36);
    m_btnEmoji->setFixedSize(36, 36);
    
    QString inputStyle = R"(
        QTextEdit {
            border: 1px solid #D1D1D6;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 14px;
            background-color: white;
        }
        QTextEdit:focus {
            border-color: #007AFF;
        }
    )";
    
    QString sendButtonStyle = R"(
        QPushButton {
            background-color: #007AFF;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0056CC;
        }
        QPushButton:pressed {
            background-color: #004499;
        }
        QPushButton:disabled {
            background-color: #C7C7CC;
        }
    )";
    
    QString iconButtonStyle = R"(
        QPushButton {
            background-color: #F2F2F7;
            border: 1px solid #D1D1D6;
            border-radius: 8px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #E5E5EA;
        }
    )";
    
    m_messageInput->setStyleSheet(inputStyle);
    m_btnSend->setStyleSheet(sendButtonStyle);
    m_btnVoice->setStyleSheet(iconButtonStyle);
    m_btnEmoji->setStyleSheet(iconButtonStyle);
    
    connect(m_messageInput, &QTextEdit::textChanged, this, &ChatWidget::onInputTextChanged);
    connect(m_btnSend, &QPushButton::clicked, this, &ChatWidget::onSendMessage);
    
    m_inputLayout->addWidget(m_messageInput);
    m_inputLayout->addWidget(m_btnVoice); //功能还需进一步开发
    m_inputLayout->addWidget(m_btnEmoji);
    m_inputLayout->addWidget(m_btnSend);
    m_inputLayout->addLayout(sendLayout);
    
    m_mainLayout->addWidget(m_inputWidget);
    
    // 初始化定时器
    m_typingTimer = new QTimer(this);
    m_typingTimer->setSingleShot(true);
    connect(m_typingTimer, &QTimer::timeout, this, &ChatWidget::simulateTyping);
    
    m_responseTimer = new QTimer(this);
    m_responseTimer->setSingleShot(true);
    connect(m_responseTimer, &QTimer::timeout, this, &ChatWidget::onAIResponseReady);
}

void ChatWidget::setupQuickButtons()
{
    // 初始化部门列表
    m_departments = {
                     "控制部",
                     "福利部",
                     "记录部",
                     "培训部",
                     "研发部",
                     "情报部",
                     "安保部",
                     "中央本部一区",
                     "中央本部二区",
                     "惩戒部"
    };
    
    // 初始化性格特长关键词映射
    m_qualityKeywords["勇气"] = {"勇气", "勇敢", "强壮", "积极", "上进","外向"};
    m_qualityKeywords["谨慎"] = {"谨慎", "内向", "善良", "细心"};
    m_qualityKeywords["自律"] = {"自律", "约束", "纪律", "规矩"};
    m_qualityKeywords["正义"] = {"正义", "责任", "热情", "梦想"};

    


    QStringList quickQuestions = { "转人工客服"};
    
    int row = 0, col = 0;
    for (const QString& question : quickQuestions) {
        addQuickButton(question, "");
        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }
}

void ChatWidget::addQuickButton(const QString& text, const QString& responseTemplate)
{
    QPushButton* button = new QPushButton(text);
    button->setStyleSheet(R"(
        QPushButton {
            background-color: white;
            border: 1px solid #D1D1D6;
            border-radius: 8px;
            padding: 10px 15px;
            font-size: 13px;
            color: #1D1D1F;
            text-align: left;
        }
        QPushButton:hover {
            background-color: #F2F8FF;
            border-color: #007AFF;
        }
        QPushButton:pressed {
            background-color: #E3F2FD;
        }
    )");
    
    button->setProperty("responseTemplate", responseTemplate);
    m_quickButtonGroup->addButton(button);
    
    // 计算按钮位置
    int buttonCount = m_quickButtonGroup->buttons().size() - 1;
    int row = buttonCount / 3;
    int col = buttonCount % 3;
    m_quickButtonsLayout->addWidget(button, row, col);
}

void ChatWidget::onQuickButtonClicked(QAbstractButton* button)
{
    QPushButton* pushButton = qobject_cast<QPushButton*>(button);
    if (pushButton) {
        QString buttonText = pushButton->text();
        qDebug() << "快捷按钮被点击:" << buttonText;
        
        // 检查是否是转人工服务按钮
        if (buttonText.contains("转人工客服")) {
            qDebug() << "检测到转人工客服按钮点击！";
            onTransferToHuman();
            return;
        }
        
        // 移除emoji，提取关键词
        QString cleanText = buttonText.remove(QRegularExpression("[🤒😷🤕🤧😣🔴👁️👂🦷]")).trimmed();
        
        m_messageInput->setPlainText("我想咨询" + cleanText + "的问题");
        onSendMessage();
    }
}

void ChatWidget::onTransferToHuman()
{
    // 调试信息
    qDebug() << "转人工按钮被点击！用户ID:" << m_userId << "用户名:" << m_userName;
    
    // 添加转人工提示消息
    AIMessage systemMsg;
    systemMsg.content = "正在为您转接人工客服，请稍候...";
    systemMsg.type = MessageType::System;
    systemMsg.timestamp = QDateTime::currentDateTime();
    systemMsg.sessionId = m_currentSessionId;
    addMessage(systemMsg);
    
    // 发出转人工信号，包含当前对话上下文
    QString context = "";
    for (const AIMessage& msg : m_chatHistory) {
        if (msg.type == MessageType::User) {
            context += "访客：" + msg.content + "\n";
        } else if (msg.type == MessageType::Robot) {
            context += "AI助手：" + msg.content + "\n";
        }
    }
    
    qDebug() << "发射转人工信号，上下文长度:" << context.length();
    emit requestHumanService(m_userId, m_userName, context);
    
    // 添加转人工说明
    addTransferOption();
}

void ChatWidget::addTransferOption()
{
    clearInteractionComponents();
    
    QWidget* transferWidget = new QWidget;
    QVBoxLayout* transferLayout = new QVBoxLayout(transferWidget);
    transferLayout->setSpacing(10);
    
    QLabel* infoLabel = new QLabel("已为您转接人工客服服务：");
    infoLabel->setStyleSheet(R"(
        QLabel {
            font-size: 14px;
            color: #1D1D1F;
            font-weight: bold;
            padding: 5px;
        }
    )");
    
    QLabel* detailLabel = new QLabel("• 请切换到\"客服咨询\"选项卡继续对话\n• 您的对话记录已同步给客服\n• 如需重新使用AIHR，请点击下方按钮");
    detailLabel->setStyleSheet(R"(
        QLabel {
            font-size: 13px;
            color: #666666;
            padding: 5px 10px;
            line-height: 1.4;
        }
    )");
    
    QPushButton* backToAIButton = new QPushButton("返回AIHR");
    backToAIButton->setStyleSheet(R"(
        QPushButton {
            background-color: #34C759;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #30A855;
        }
    )");
    
    connect(backToAIButton, &QPushButton::clicked, [this]() {
        clearInteractionComponents();
        AIMessage backMsg;
        backMsg.content = "欢迎回到AI智能HR！有什么可以帮助您的吗？";
        backMsg.type = MessageType::Robot;
        backMsg.timestamp = QDateTime::currentDateTime();
        backMsg.sessionId = m_currentSessionId;
        addMessage(backMsg);
    });
    
    transferLayout->addWidget(infoLabel);
    transferLayout->addWidget(detailLabel);
    transferLayout->addWidget(backToAIButton);
    transferLayout->addStretch();
    
    m_interactionLayout->addWidget(transferWidget);
    m_interactionWidget->show();   // 标记
}

void ChatWidget::onSendMessage()
{
    QString text = m_messageInput->toPlainText().trimmed();
    if (text.isEmpty() || m_isAITyping) {
        return;
    }
    
    // 创建用户消息
    AIMessage userMsg;
    userMsg.content = text;
    userMsg.type = MessageType::User;
    userMsg.timestamp = QDateTime::currentDateTime();
    userMsg.sessionId = m_currentSessionId;
    
    addMessage(userMsg);
    m_messageInput->clear();
    m_currentContext = text;
    
    // 检查是否直接触发转人工关键词
    QString lowerText = text.toLower();
    if (lowerText.contains("转人工") || lowerText.contains("换人工") || lowerText.contains("要人工") || 
        lowerText.contains("人工客服") || lowerText.contains("真人客服") || lowerText.contains("联系客服")) {
        QTimer::singleShot(500, this, &ChatWidget::onTransferToHuman);
        return;
    }
    
    // 构建对话历史
    QString conversationHistory;
    int recentMsgCount = qMin(5, m_chatHistory.size()); // 只取最近5条消息作为上下文
    for (int i = m_chatHistory.size() - recentMsgCount; i < m_chatHistory.size(); i++) {
        const AIMessage& msg = m_chatHistory[i];
        if (msg.type == MessageType::User) {
            conversationHistory += "访客：" + msg.content + "\n";
        } else if (msg.type == MessageType::Robot) {
            conversationHistory += "AI助手：" + msg.content + "\n";
        }
    }
    
    // 使用真实的AI API进行HR
    m_aiApiClient->sendChatRequest(text, conversationHistory);
}

void ChatWidget::onAIResponseReady()
{
    m_isAITyping = false;
    m_statusLabel->setText("智能HR助手");
    m_btnSend->setEnabled(true);
    
    if (!m_pendingResponse.isEmpty()) {
        AIMessage aiMsg;
        aiMsg.content = m_pendingResponse;
        aiMsg.type = MessageType::Robot;
        aiMsg.timestamp = QDateTime::currentDateTime();
        aiMsg.sessionId = m_currentSessionId;
        
        addMessage(aiMsg);
        m_pendingResponse.clear();
        
        // 分析是否需要添加交互组件
        ChatAdvice advice = analyzeQualitys(m_currentContext);
        if (!advice.department.isEmpty()) {
            processChatAdvice(advice);
        }
    }
}

QString ChatWidget::generateAIResponse(const QString& userInput)
{
    // AIHR逻辑
    QString input = userInput.toLower();
    
    // 检查合适情况
    QStringList fitnessKeywords = {"天才", "特色", "首脑", "爪牙", "血魔", "都市之星"};
    for (const QString& keyword : fitnessKeywords) {
        if (input.contains(keyword)) {
            return "⚠️ 根据您描述的特点，您可能是我们公司在找的人才！！建议您立即前往惩戒部寻找堂吉诃德先生参与面试与培训！\n\n惩戒部位置：公司1楼\n惩戒部电话：114514";
        }
    }
    
    // 勇气相关
    if (input.contains("勇气") || input.contains("勇敢") || input.contains("强壮")) {
        return "根据您的发热品质，我需要了解更多信息：\n\n• 体温多少度？\n• 持续多长时间了？\n• 是否伴随其他品质？\n\n一般情况下：\n🌡️ 38.5°C以下：建议物理降温\n🌡️ 38.5°C以上：建议控制部就诊\n🚨 持续高热：建议惩戒部\n\n如需更详细的诊断，建议点击下方转人工客服。";
    }
    
    // 头痛相关
    if (input.contains("头疼") || input.contains("头痛") || input.contains("头晕")) {
        return "关于头痛品质，我来帮您分析：\n\n请问：\n• 疼痛程度如何？\n• 是否伴随恶心呕吐？\n• 最近有没有外伤？\n\n建议部门：\n🧠 神经控制部：偏头痛、神经性头痛\n👁️ 眼科：视力相关头痛\n🏥 控制部：感冒引起的头痛\n\n如需专业医生诊断，可转接人工客服。";
    }
    
    // 咳嗽相关
    if (input.contains("咳嗽") || input.contains("咳痰")) {
        return "咳嗽品质分析：\n\n请描述：\n• 干咳还是有痰？\n• 持续时间？\n• 是否伴随发热？\n\n推荐部门：\n🫁 呼吸控制部：持续咳嗽、咳痰\n👶 培训部：小儿咳嗽\n🏥 控制部：一般性咳嗽\n\n需要详细诊断建议转人工客服。";
    }
    
    // 转人工相关 - 直接触发转人工
    if (input.contains("转人工") || input.contains("换人工") || input.contains("要人工") || 
        input.contains("人工客服") || input.contains("真人客服") || input.contains("联系客服")) {
        // 直接触发转人工
        QTimer::singleShot(500, this, &ChatWidget::onTransferToHuman);
        return "好的，正在为您转接人工客服，请稍候...";
    }
    
    // 一般人工咨询提示
    if (input.contains("人工") || input.contains("客服") || input.contains("医生")) {
        return "我可以为您转接人工客服：\n\n🏥 人工客服可以提供：\n• 专业医疗咨询\n• 详细品质分析\n• 预约挂号协助\n• 公司相关服务\n\n💬 输入\"转人工\"可直接转接\n📱 或点击下方\"转人工客服\"按钮";
    }
    
    // 预约相关
    if (input.contains("预约") || input.contains("挂号")) {
        return "关于预约挂号：\n\n📱 预约方式：\n• 微信公众号预约\n• 手机APP预约\n• 现场挂号\n• 电话预约：400-123-4567\n\n⏰ 预约时间：\n• 普通门诊：提前3天\n• 专家门诊：提前7天\n\n需要预约协助？建议转接人工客服。";
    }
    
    // 默认响应
    return "我理解您的品质描述。为了给您更准确的建议，请提供更多详细信息：\n\n• 品质持续时间\n• 疼痛或不适程度\n• 是否伴随其他品质\n• 您的年龄范围\n\n如需专业医生诊断，建议转人工客服获得更详细的医疗建议。";
}

// 实现其他必要的方法
void ChatWidget::addMessage(const AIMessage& message)
{
    m_chatHistory.append(message);
    displayMessage(message);
    m_messageCount++;
    
    // 自动保存每10条消息
    if (m_messageCount % 10 == 0) {
        saveChatHistory();
    }
}

void ChatWidget::displayMessage(const AIMessage& message)
{
    QWidget* messageWidget = new QWidget;
    QHBoxLayout* messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建消息气泡
    QLabel* bubbleLabel = new QLabel;
    bubbleLabel->setText(message.content);
    bubbleLabel->setWordWrap(true);
    bubbleLabel->setTextFormat(Qt::RichText);
    bubbleLabel->setOpenExternalLinks(true);
    
    // 时间戳
    QLabel* timeLabel = new QLabel(formatTimestamp(message.timestamp));
    timeLabel->setStyleSheet("color: #8E8E93; font-size: 11px;");
    
    QString bubbleStyle;
    if (message.type == MessageType::User) {
        // 用户消息 - 右对齐，蓝色
        bubbleStyle = R"(
            QLabel {
                background-color: #007AFF;
                color: white;
                border-radius: 12px;
                padding: 12px 16px;
                margin-left: 60px;
                font-size: 14px;
                line-height: 1.4;
            }
        )";
        messageLayout->addStretch();
        
        QVBoxLayout* rightLayout = new QVBoxLayout;
        rightLayout->addWidget(bubbleLabel);
        rightLayout->addWidget(timeLabel);
        timeLabel->setAlignment(Qt::AlignRight);
        
        messageLayout->addLayout(rightLayout);
        
    } else {
        // 机器人消息 - 左对齐，灰色
        bubbleStyle = R"(
            QLabel {
                background-color: #F2F2F7;
                color: #1D1D1F;
                border-radius: 12px;
                padding: 12px 16px;
                margin-right: 60px;
                font-size: 14px;
                line-height: 1.4;
            }
        )";
        
        // 添加机器人头像
        QLabel* avatarLabel = new QLabel("😊");
        avatarLabel->setFixedSize(48, 48);
        avatarLabel->setAlignment(Qt::AlignCenter);
        avatarLabel->setStyleSheet(R"(
            QLabel {
                background-color: #00BFFF;
                border-radius: 24px;
                font-size: 24px;
            }
        )");
        
        QVBoxLayout* leftLayout = new QVBoxLayout;
        leftLayout->addWidget(bubbleLabel);
        leftLayout->addWidget(timeLabel);
        timeLabel->setAlignment(Qt::AlignLeft);
        
        messageLayout->addWidget(avatarLabel);
        messageLayout->addLayout(leftLayout);
        messageLayout->addStretch();
    }
    
    bubbleLabel->setStyleSheet(bubbleStyle);
    
    // 移除最后的弹性空间，添加新消息，再添加弹性空间
    m_chatLayout->removeItem(m_chatLayout->itemAt(m_chatLayout->count() - 1));
    m_chatLayout->addWidget(messageWidget);
    m_chatLayout->addStretch();
    
    // 滚动到底部
    QTimer::singleShot(100, this, &ChatWidget::scrollToBottom);
}

void ChatWidget::scrollToBottom()
{
    QScrollBar* scrollBar = m_chatScrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

ChatAdvice ChatWidget::analyzeQualitys(const QString& userInput)
{
    ChatAdvice advice;
    QString input = userInput.toLower();
    
    // 简单的品质分析逻辑
    if (input.contains("发烧") || input.contains("发热")) {
        advice.department = "控制部";
        advice.reason = "发热品质通常需要控制部医生评估";
        advice.needAppointment = true;
        advice.fitness = input.contains("高烧") || input.contains("39");
    }
    else if (input.contains("咳嗽") || input.contains("呼吸")) {
        advice.department = "呼吸控制部";
        advice.reason = "呼吸道品质建议看呼吸控制部";
        advice.needAppointment = true;
    }
    else if (input.contains("头痛") || input.contains("头晕")) {
        advice.department = "神经控制部";
        advice.reason = "头部不适建议神经控制部检查";
        advice.needAppointment = true;
    }
    
    return advice;
}

void ChatWidget::processChatAdvice(const ChatAdvice& advice)
{
    if (advice.fitness) {
        // 合适情况，添加合适就诊按钮
        addActionButtons({"立即急诊", "拨打120", "转人工客服"});
    } else if (advice.needAppointment) {
        // 需要预约，添加预约按钮
        // addActionButtons({"📅 预约" + advice.department, "🔍 查看更多部门", "👤 转人工客服"});
    }
}

void ChatWidget::addActionButtons(const QStringList& actions)
{
    clearInteractionComponents();
    
    QWidget* actionWidget = new QWidget;
    QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setSpacing(10);
    
    for (const QString& action : actions) {
        QPushButton* btn = new QPushButton(action);
        QString buttonStyle;
        
        if (action.contains("转人工客服")) {
            buttonStyle = R"(
                QPushButton {
                    background-color: #FF9500;
                    color: white;
                    border: none;
                    border-radius: 8px;
                    padding: 10px 20px;
                    font-size: 14px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #E6860E;
                }
            )";
            connect(btn, &QPushButton::clicked, this, &ChatWidget::onTransferToHuman);
        } else {
            buttonStyle = R"(
                QPushButton {
                    background-color: #34C759;
                    color: white;
                    border: none;
                    border-radius: 8px;
                    padding: 10px 20px;
                    font-size: 14px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #30A855;
                }
            )";
            connect(btn, &QPushButton::clicked, this, &ChatWidget::onActionButtonClicked);
        }
        
        btn->setStyleSheet(buttonStyle);
        actionLayout->addWidget(btn);
    }
    
    actionLayout->addStretch();
    m_interactionLayout->addWidget(actionWidget);
    // m_interactionWidget->show();//modify
}

void ChatWidget::clearInteractionComponents()
{
    while (m_interactionLayout->count() > 0) {
        QLayoutItem* item = m_interactionLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_interactionWidget->hide();
}

// 其他槽函数的简单实现
void ChatWidget::onInputTextChanged()
{
    bool hasText = !m_messageInput->toPlainText().trimmed().isEmpty();
    m_btnSend->setEnabled(hasText && !m_isAITyping);
}

void ChatWidget::onActionButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString action = button->text();
        
        AIMessage actionMsg;
        actionMsg.content = "您选择了：" + action + "\n\n相关功能正在开发中。如需立即协助，建议转人工客服。";
        actionMsg.type = MessageType::System;
        actionMsg.timestamp = QDateTime::currentDateTime();
        actionMsg.sessionId = m_currentSessionId;
        
        addMessage(actionMsg);
        clearInteractionComponents();
    }
}

void ChatWidget::onClearChatClicked()
{
    int ret = QMessageBox::question(this, "确认清空", "确定要清空聊天记录吗？\n此操作不可恢复。");
    if (ret == QMessageBox::Yes) {
        // 清空显示
        while (m_chatLayout->count() > 1) {
            QLayoutItem* item = m_chatLayout->takeAt(0);
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        
        // 清空历史记录
        m_chatHistory.clear();
        m_messageCount = 0;
        
        // 重新发送欢迎消息
        QTimer::singleShot(300, [this]() {
            AIMessage welcomeMsg;
            welcomeMsg.content = "聊天记录已清空。我是公司智能HR助手，请问有什么可以帮助您的吗？";
            welcomeMsg.type = MessageType::Robot;
            welcomeMsg.timestamp = QDateTime::currentDateTime();
            welcomeMsg.sessionId = m_currentSessionId;
            addMessage(welcomeMsg);
        });
    }
}

void ChatWidget::onSaveChatClicked()
{
    QMessageBox::information(this, "保存成功", "聊天记录已保存。");
}

void ChatWidget::onSettingsClicked()
{
    QMessageBox::information(this, "设置", "设置功能正在开发中...");
}

// 数据库相关方法的简单实现
void ChatWidget::initDatabase()
{
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    
    m_database = QSqlDatabase::addDatabase("QSQLITE", "ai_chat_db");
    m_database.setDatabaseName(dbPath + "/ai_chat_history.db");
    
    if (m_database.open()) {
        QSqlQuery query(m_database);
        query.exec("CREATE TABLE IF NOT EXISTS ai_chat_messages ("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                  "session_id TEXT,"
                  "content TEXT,"
                  "message_type INTEGER,"
                  "timestamp TEXT)");
    }
}

void ChatWidget::saveChatHistory()
{
    if (!m_database.isOpen()) return;
    
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO ai_chat_messages (session_id, content, message_type, timestamp) "
                 "VALUES (?, ?, ?, ?)");
    
    for (const AIMessage& msg : m_chatHistory) {
        query.bindValue(0, msg.sessionId);
        query.bindValue(1, msg.content);
        query.bindValue(2, static_cast<int>(msg.type));
        query.bindValue(3, msg.timestamp.toString(Qt::ISODate));
        query.exec();
    }
}

QString ChatWidget::generateSessionId()
{
    return QUuid::createUuid().toString().remove("{").remove("}");
}

QString ChatWidget::formatTimestamp(const QDateTime& timestamp)
{
    return timestamp.toString("hh:mm");
}

// 其他未实现的方法
void ChatWidget::simulateTyping() { }
void ChatWidget::onDepartmentSelected() { }
void ChatWidget::loadChatHistory() { }
QString ChatWidget::extractKeywords(const QString& text) { return text; }
QStringList ChatWidget::getQualityKeywords(const QString& text) { return QStringList(); }
void ChatWidget::updateQuickButtons(const QStringList& suggestions) { }
void ChatWidget::addDepartmentSelector(const QStringList& departments) { }

void ChatWidget::onAIChatResponse(const AIAnalysisResult& result)
{
    // 创建AI回复消息
    AIMessage aiMsg;
    aiMsg.content = result.aiResponse;
    aiMsg.type = MessageType::Robot;
    aiMsg.timestamp = QDateTime::currentDateTime();
    aiMsg.sessionId = m_currentSessionId;
    
    addMessage(aiMsg);
    
    // 根据诊断结果添加交互组件
    QStringList actionButtons;
    
    if (result.fitnessLevel == "critical") {
        actionButtons << "🚨 立即急诊" << "📞 拨打120" << "👤 转人工客服";
    } else if (result.fitnessLevel == "high") {
        actionButtons << "🏥 尽快就医" << "📞 预约挂号" << "👤 转人工客服";
    } else if (!result.recommendedDepartment.isEmpty()) {
        actionButtons << QString("📅 预约%1").arg(result.recommendedDepartment) 
                      << "🔍 查看更多部门" << "👤 转人工客服";
    } else {
        actionButtons << "🔍 品质分析" << "📅 预约挂号" << "👤 转人工客服";
    }
    
    if (!actionButtons.isEmpty()) {
        addActionButtons(actionButtons);
    }
    
    qDebug() << "AIHR结果 - 部门:" << result.recommendedDepartment
             << "合适程度:" << result.fitnessLevel
             << "需要人工:" << result.needsHumanConsult;
}

void ChatWidget::onAIApiError(const QString& error)
{
    qDebug() << "AI API错误:" << error;
    
    // 显示错误消息并回退到本地逻辑
    AIMessage errorMsg;
    errorMsg.content = "抱歉，AI服务暂时不可用，为您提供基础HR建议：\n\n" + generateAIResponse(m_currentContext);
    errorMsg.type = MessageType::Robot;
    errorMsg.timestamp = QDateTime::currentDateTime();
    errorMsg.sessionId = m_currentSessionId;
    
    addMessage(errorMsg);
    
    // 添加基础交互按钮
    addActionButtons({"🔍 品质自查", "📅 预约挂号", "👤 转人工客服"});
} 
