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
    , m_networkManager(nullptr)
    , m_audioSource(nullptr)
    , m_audioBuffer(nullptr)
    , m_isRecording(false)
{
    initDatabase();
    setupUI();
    setupQuickButtons();
    
    m_currentSessionId = generateSessionId();
    m_isInitialized = true;
    m_specialResponses["堂吉诃德是个什么样的人"] = "D:/qtprogram/Don.png";
    m_networkManager = new QNetworkAccessManager(this);

    // 华为云配置 - 这些应该从配置文件或设置中读取
    m_projectId = "c234d9997e794b09b55d19cc0c5abd58";
    m_region = "cn-east-3";
    m_ak = "HPUARBBTTI7X7ERUIFEG";
    m_sk = "EcMUrW6zEbz6sIGgdQZUNo80ic5PLvWGMRHEHqzb";

    // 获取IAM Token
    getIAMToken();

    // 连接网络请求完成信号
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &ChatWidget::onTokenReceived);
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
        welcomeMsg.content = "您好！我是青蓝公司智能HR助手\n\n我可以帮助您：\n• 如果您想入职，请自我介绍，我会进行分析\n• 解答公司相关问题\n•  识别您与公司及部门合适度，还能帮您安排面试 \n• 转接人工客服\n\n请描述您的问题开始咨询！";
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
    if (m_audioSource) {
        m_audioSource->stop();
        delete m_audioSource;
    }

    if (m_audioBuffer) {
        m_audioBuffer->close();
        delete m_audioBuffer;
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
    // 显示语音按钮
    m_btnVoice->show();

    m_btnVoice->setStyleSheet(iconButtonStyle);
    m_btnVoice->setCheckable(true);

    // 连接语音按钮点击信号
    connect(m_btnVoice, &QPushButton::clicked, [this](bool checked) {
        if (checked) {
            startRecording();
        } else {
            stopRecording();
        }
    });
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
                     "构筑部"
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
// 开始录音
void ChatWidget::startRecording()
{
    if (m_isRecording) return;

    // 检查麦克风权限
    QMediaDevices devices;
    if (devices.audioInputs().isEmpty()) {
        QMessageBox::warning(this, "录音失败", "未找到可用的麦克风设备");
        m_btnVoice->setChecked(false);
        return;
    }

    // 设置音频格式 - 华为云要求pcm16k16bit
    QAudioFormat format;
    format.setSampleRate(16000); // 16kHz
    format.setChannelCount(1);   // 单声道
    format.setSampleFormat(QAudioFormat::Int16); // 16bit

    // 获取默认音频输入设备
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (!inputDevice.isFormatSupported(format)) {
        QMessageBox::warning(this, "录音失败", "音频格式不支持");
        m_btnVoice->setChecked(false);
        return;
    }

    // 创建音频输入和缓冲区
    m_audioSource = new QAudioSource(inputDevice, format, this);
    m_audioBuffer = new QBuffer(this);
    m_audioBuffer->open(QIODevice::WriteOnly);

    // 开始录音
    m_audioSource->start(m_audioBuffer);
    m_isRecording = true;

    // 更新UI
    m_btnVoice->setText("🔴");
    m_statusLabel->setText("录音中...");
}
// 停止录音并识别
void ChatWidget::stopRecording()
{
    if (!m_isRecording) return;

    // 停止录音
    m_audioSource->stop();
    m_isRecording = false;

    // 获取录音数据
    QByteArray audioData = m_audioBuffer->data();
    m_audioBuffer->close();

    // 清理资源
    delete m_audioSource;
    delete m_audioBuffer;
    m_audioSource = nullptr;
    m_audioBuffer = nullptr;

    // 更新UI
    m_btnVoice->setText("🎤");
    m_statusLabel->setText("识别中...");

    // 发送到华为云进行识别
    speechRecognize(audioData);
}

// 获取IAM Token
void ChatWidget::getIAMToken()
{
    if (m_ak.isEmpty() || m_sk.isEmpty()) {
        qWarning() << "AK/SK未配置，无法获取Token";
        return;
    }

    QUrl url("https://iam.myhuaweicloud.com/v3/auth/tokens");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 构建请求体
    QJsonObject auth;
    QJsonObject identity;
    QJsonArray methods;
    methods.append("password");

    QJsonObject password;
    QJsonObject user;
    user.insert("name", m_ak);
    user.insert("password", m_sk);
    user.insert("domain", QJsonObject{{"name", m_ak}});

    password.insert("user", user);
    identity.insert("methods", methods);
    identity.insert("password", password);

    QJsonObject scope;
    QJsonObject project;
    project.insert("name", m_region);
    scope.insert("project", project);

    auth.insert("identity", identity);
    auth.insert("scope", scope);

    QJsonObject root;
    root.insert("auth", auth);

    QJsonDocument doc(root);
    QByteArray data = doc.toJson();

    // 发送请求
    m_networkManager->post(request, data);
}

// 处理Token响应
void ChatWidget::onTokenReceived(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "获取Token失败:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    // 从响应头中获取Token
    QByteArray token = reply->rawHeader("X-Subject-Token");
    if (!token.isEmpty()) {
        m_token = token;
        qDebug() << "获取Token成功";
    } else {
        qWarning() << "未找到Token";
    }

    reply->deleteLater();

    // 重新连接网络管理器到语音识别结果处理
    disconnect(m_networkManager, &QNetworkAccessManager::finished,
               this, &ChatWidget::onTokenReceived);
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &ChatWidget::onSpeechRecognitionResult);
}

// 语音识别
void ChatWidget::speechRecognize(const QByteArray& audioData)
{
    if (m_token.isEmpty()) {
        QMessageBox::warning(this, "很遗憾", "Token用完了，想用就去充钱罢");
        return;
    }

    // 构建请求URL
    QString urlStr = QString("https://sis-ext.%1.myhuaweicloud.com/v1/%2/asr/short-audio")
                         .arg(m_region).arg(m_projectId);

    QUrl url(urlStr);
    QNetworkRequest request(url);

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("X-Auth-Token", m_token.toUtf8());

    // 构建请求体
    QJsonObject config;
    config.insert("audio_format", "pcm16k16bit");
    config.insert("property", "chinese_16k_common");

    QJsonObject data;
    data.insert("data", QString(audioData.toBase64()));

    QJsonObject root;
    root.insert("config", config);
    root.insert("data", data);

    QJsonDocument doc(root);
    QByteArray postData = doc.toJson();

    // 发送请求
    m_networkManager->post(request, postData);
}

// 处理语音识别结果
void ChatWidget::onSpeechRecognitionResult(QNetworkReply* reply)
{
    m_statusLabel->setText("智能HR助手");

    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, "识别失败", reply->errorString());
        reply->deleteLater();
        return;
    }

    // 解析响应
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject json = doc.object();

    if (json.contains("result")) {
        QJsonObject result = json["result"].toObject();
        if (result.contains("text")) {
            QString recognizedText = result["text"].toString();

            // 将识别结果填入输入框
            if (!recognizedText.isEmpty()) {
                m_messageInput->setPlainText(recognizedText);

                // 可选：自动发送识别结果
                // onSendMessage();
            }
        }
    } else if (json.contains("error_msg")) {
        QString errorMsg = json["error_msg"].toString();
        QMessageBox::warning(this, "识别错误", errorMsg);
    }

    reply->deleteLater();
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
    
    // 检查特殊响应 - 堂吉诃德问题
    if (text == "堂吉诃德长什么样？") {
        QString imagePath = "D:/qtprogram/Don.png";
        displayImageMessage(imagePath, "堂吉诃德先生");
        return;
    }

    // 检查是否直接触发转人工关键词
    QString lowerText = text.toLower();
    if (lowerText.contains("转人工") || lowerText.contains("换人工") || lowerText.contains("要人工") || 
        lowerText.contains("人工客服") || lowerText.contains("真人客服") || lowerText.contains("真人HR")) {
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
// 添加显示图片的函数
void ChatWidget::addImageMessage(const QString& imagePath, const QString& altText)
{
    // 检查文件是否存在
    QFile file(imagePath);
    if (!file.exists()) {
        AIMessage errorMsg;
        errorMsg.content = "抱歉，图片未找到: " + imagePath;
        errorMsg.type = MessageType::System;
        errorMsg.timestamp = QDateTime::currentDateTime();
        errorMsg.sessionId = m_currentSessionId;
        addMessage(errorMsg);
        return;
    }

    // 创建图片消息
    AIMessage imageMsg;
    imageMsg.content = QString("<div style='text-align: center;'><img src='%1' alt='%2' style='max-width: 300px; max-height: 300px; border-radius: 8px;'/><p style='color: #666; margin-top: 8px;'>%2</p></div>")
                           .arg(imagePath).arg(altText);
    imageMsg.type = MessageType::Robot;
    imageMsg.timestamp = QDateTime::currentDateTime();
    imageMsg.sessionId = m_currentSessionId;

    addMessage(imageMsg);
}

// 添加显示图片消息的函数
void ChatWidget::displayImageMessage(const QString& imagePath, const QString& caption)
{
    QWidget* messageWidget = new QWidget;
    QHBoxLayout* messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(0, 0, 0, 0);

    // 机器人头像
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

    // 图片容器
    QWidget* imageContainer = new QWidget;
    QVBoxLayout* imageLayout = new QVBoxLayout(imageContainer);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->setSpacing(5);

    // 图片标签
    QLabel* imageLabel = new QLabel;
    QPixmap pixmap(imagePath);

    if (pixmap.isNull()) {
        // 图片加载失败
        imageLabel->setText("图片加载失败: " + imagePath);
        imageLabel->setStyleSheet(R"(
            QLabel {
                background-color: #F2F2F7;
                color: #FF3B30;
                border-radius: 12px;
                padding: 12px 16px;
                margin-right: 60px;
                font-size: 14px;
            }
        )");
    } else {
        // 缩放图片以适应显示
        QPixmap scaledPixmap = pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(scaledPixmap);
        imageLabel->setStyleSheet(R"(
            QLabel {
                background-color: #F2F2F7;
                border-radius: 12px;
                padding: 10px;
            }
        )");
        imageLabel->setAlignment(Qt::AlignCenter);
    }

    // 图片说明
    QLabel* captionLabel = new QLabel(caption);
    captionLabel->setStyleSheet(R"(
        QLabel {
            color: #8E8E93;
            font-size: 12px;
            padding: 0 5px;
        }
    )");
    captionLabel->setAlignment(Qt::AlignCenter);

    // 时间戳
    QLabel* timeLabel = new QLabel(formatTimestamp(QDateTime::currentDateTime()));
    timeLabel->setStyleSheet("color: #8E8E93; font-size: 11px;");

    imageLayout->addWidget(imageLabel);
    imageLayout->addWidget(captionLabel);
    imageLayout->addWidget(timeLabel);
    timeLabel->setAlignment(Qt::AlignLeft);

    // 添加到消息布局
    messageLayout->addWidget(avatarLabel);
    messageLayout->addWidget(imageContainer);
    messageLayout->addStretch();

    // 添加到聊天区域
    m_chatLayout->removeItem(m_chatLayout->itemAt(m_chatLayout->count() - 1));
    m_chatLayout->addWidget(messageWidget);
    m_chatLayout->addStretch();

    // 滚动到底部
    QTimer::singleShot(100, this, &ChatWidget::scrollToBottom);

    // 添加到聊天历史
    AIMessage imageMsg;
    imageMsg.content = "[图片] " + caption;
    imageMsg.type = MessageType::Robot;
    imageMsg.timestamp = QDateTime::currentDateTime();
    imageMsg.sessionId = m_currentSessionId;
    m_chatHistory.append(imageMsg);
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
            return "⚠️ 根据您描述的特点，您可能是我们公司在找的人才！！建议您立即前往惩戒部寻找堂吉诃德先生参与面试与培训！\n\n惩戒部位置：公司6楼\n惩戒部电话：114514";
        }
    }
    
    // 转人工相关 - 直接触发转人工
    if (input.contains("转人工") || input.contains("换人工") || input.contains("要人工") || 
        input.contains("人工客服") || input.contains("真人客服") || input.contains("联系客服")) {
        // 直接触发转人工
        QTimer::singleShot(500, this, &ChatWidget::onTransferToHuman);
        return "好的，正在为您转接人工客服，请稍候...";
    }
    
    // 一般人工咨询提示
    if (input.contains("人工") || input.contains("客服") || input.contains("部长")) {
        return "如果您希望与部长或队长直接沟通，您可以点击界面左侧与人工客服沟通！";
    }
    
    // 预约相关
    if (input.contains("预约") || input.contains("面试")||input.contains("面试")) {
        return "如果您想预约面试，可以在界面左侧找到预约选项进行预约！";
    }
    
    // 默认响应
    return "嘟嘟嘟...\nworkworkwork......";
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
    
    // 检查是否是图片消息
    bool isImageMessage = message.content.contains("<img src=");

    // 创建消息气泡
    QLabel* bubbleLabel = new QLabel;
    bubbleLabel->setText(message.content);
    bubbleLabel->setWordWrap(true);
    bubbleLabel->setTextFormat(Qt::RichText);
    bubbleLabel->setOpenExternalLinks(true);

    // 如果是图片消息，调整大小策略
    if (isImageMessage) {
        bubbleLabel->setMinimumWidth(320);
        bubbleLabel->setMinimumHeight(340);
    }


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
    if (input.contains("爱世人") ) {
        advice.department = "福利部";
        advice.reason = "看来您和福利部的那位妖精小姐很聊的来呢🎶";
        advice.needAppointment = true;

    }

    
    return advice;
}

void ChatWidget::processChatAdvice(const ChatAdvice& advice)
{
    if (advice.fitness) {
        // 合适情况
        addActionButtons({"转人工客服"});
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
        actionButtons << "🚨 立即入职" << "📞 联系部长" << "👤 转人工客服";
    } else if (result.fitnessLevel == "high") {
        actionButtons << "📅 预约面试" << "📞 联系队长" << "👤 转人工客服";
    } else if (!result.recommendedDepartment.isEmpty()) {
        actionButtons << QString("📅 预约%1").arg(result.recommendedDepartment) 
                      << "🔍 查看更多部门" << "👤 转人工客服";
    } else {
        actionButtons << "🔍 品质分析" << "📅 预约" << "👤 转人工客服";
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
    addActionButtons({"🔍 品质自查", "📅 预约面试", "👤 转人工客服"});
} 
