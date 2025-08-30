#include "ExampleUsageWidget.h"
#include "src/core/ChatStorage.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QGridLayout>
#include <QSplitter>

ExampleUsageWidget::ExampleUsageWidget(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(nullptr)
    , m_navigationWidget(nullptr)
    , m_chatStorage(nullptr)
    , m_messageDisplay(nullptr)
    , m_senderInput(nullptr)
    , m_receiverInput(nullptr)
    , m_messageInput(nullptr)
    , m_sendButton(nullptr)
    , m_loadButton(nullptr)
    , m_clearButton(nullptr)
    , m_exportButton(nullptr)
    , m_messagesList(nullptr)
    , m_statusLabel(nullptr)
    , m_styleDemoWidget(nullptr)
{
    // 初始化聊天存储
    m_chatStorage = new ChatStorage(this);
    if (!m_chatStorage->initialize()) {
        QMessageBox::warning(this, "数据库错误", 
                           "无法初始化聊天数据库: " + m_chatStorage->getLastError());
    }
    
    setupUI();
    
    // 连接聊天存储信号
    connect(m_chatStorage, &ChatStorage::messageInserted,
            this, &ExampleUsageWidget::onMessageInserted);
}

ExampleUsageWidget::~ExampleUsageWidget()
{
}

void ExampleUsageWidget::setupUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    UIStyleManager::applyContainerSpacing(this);
    
    // 标题
    QLabel *titleLabel = new QLabel("医院智慧客服系统功能演示", this);
    UIStyleManager::applyLabelStyle(titleLabel, "title");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(titleLabel->styleSheet() + 
        QString("QLabel {"
               "background-color: %1;"
               "border-radius: 12px;"
               "padding: 20px;"
               "margin-bottom: 20px;"
               "}")
        .arg(UIStyleManager::colors.surface));
    titleLabel->setGraphicsEffect(UIStyleManager::createShadowEffect(titleLabel));
    mainLayout->addWidget(titleLabel);
    
    // 创建选项卡控件
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet(m_tabWidget->styleSheet() + 
        QString("QTabWidget::pane {"
               "border: 2px solid %1;"
               "border-radius: 8px;"
               "background-color: %2;"
               "margin-top: 5px;"
               "}")
        .arg(UIStyleManager::colors.border)
        .arg(UIStyleManager::colors.surface));
    
    setupNavigationTab();
    setupChatTab();
    setupStyleDemoTab();
    
    mainLayout->addWidget(m_tabWidget);
}

void ExampleUsageWidget::setupNavigationTab()
{
    // 创建医院导航选项卡
    m_navigationWidget = new HospitalNavigationWidget();
    m_tabWidget->addTab(m_navigationWidget, "🗺️ 医院导航");
}

void ExampleUsageWidget::setupChatTab()
{
    QWidget *chatTab = new QWidget();
    QHBoxLayout *chatLayout = new QHBoxLayout(chatTab);
    chatLayout->setContentsMargins(15, 15, 15, 15);
    chatLayout->setSpacing(20);
    
    // 左侧：控制面板
    QFrame *controlFrame = new QFrame();
    UIStyleManager::applyFrameStyle(controlFrame, true);
    controlFrame->setFixedWidth(350);
    
    QVBoxLayout *controlLayout = new QVBoxLayout(controlFrame);
    controlLayout->setContentsMargins(15, 15, 15, 15);
    controlLayout->setSpacing(15);
    
    // 控制面板标题
    QLabel *controlTitle = new QLabel("💬 聊天记录管理", controlFrame);
    UIStyleManager::applyLabelStyle(controlTitle, "subtitle");
    controlLayout->addWidget(controlTitle);
    
    // 消息输入分组
    QGroupBox *inputGroup = new QGroupBox("发送消息", controlFrame);
    UIStyleManager::applyGroupBoxStyle(inputGroup);
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
    
    m_senderInput = new QLineEdit(inputGroup);
    m_senderInput->setPlaceholderText("发送者");
    m_senderInput->setText("患者001");
    UIStyleManager::applyLineEditStyle(m_senderInput);
    
    m_receiverInput = new QLineEdit(inputGroup);
    m_receiverInput->setPlaceholderText("接收者");
    m_receiverInput->setText("客服001");
    UIStyleManager::applyLineEditStyle(m_receiverInput);
    
    m_messageInput = new QLineEdit(inputGroup);
    m_messageInput->setPlaceholderText("消息内容");
    m_messageInput->setText("你好，我想咨询一下挂号的问题");
    UIStyleManager::applyLineEditStyle(m_messageInput);
    
    m_sendButton = new QPushButton("📤 发送消息", inputGroup);
    UIStyleManager::applyButtonStyle(m_sendButton, "primary");
    
    inputLayout->addWidget(new QLabel("发送者:"));
    inputLayout->addWidget(m_senderInput);
    inputLayout->addWidget(new QLabel("接收者:"));
    inputLayout->addWidget(m_receiverInput);
    inputLayout->addWidget(new QLabel("消息:"));
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    
    controlLayout->addWidget(inputGroup);
    
    // 操作按钮分组
    QGroupBox *actionGroup = new QGroupBox("数据操作", controlFrame);
    UIStyleManager::applyGroupBoxStyle(actionGroup);
    QVBoxLayout *actionLayout = new QVBoxLayout(actionGroup);
    
    m_loadButton = new QPushButton("📋 加载历史记录", actionGroup);
    UIStyleManager::applyButtonStyle(m_loadButton, "secondary");
    
    m_exportButton = new QPushButton("💾 导出数据", actionGroup);
    UIStyleManager::applyButtonStyle(m_exportButton, "secondary");
    
    m_clearButton = new QPushButton("🗑️ 清空记录", actionGroup);
    UIStyleManager::applyButtonStyle(m_clearButton, "error");
    
    actionLayout->addWidget(m_loadButton);
    actionLayout->addWidget(m_exportButton);
    actionLayout->addWidget(m_clearButton);
    
    controlLayout->addWidget(actionGroup);
    
    // 状态标签
    m_statusLabel = new QLabel("系统就绪", controlFrame);
    UIStyleManager::applyLabelStyle(m_statusLabel, "caption");
    m_statusLabel->setStyleSheet(m_statusLabel->styleSheet() + 
        QString("QLabel {"
               "background-color: %1;"
               "border-radius: 6px;"
               "padding: 8px;"
               "}")
        .arg(UIStyleManager::colors.background));
    controlLayout->addWidget(m_statusLabel);
    
    controlLayout->addStretch();
    
    // 右侧：消息显示
    QFrame *displayFrame = new QFrame();
    UIStyleManager::applyFrameStyle(displayFrame, true);
    
    QVBoxLayout *displayLayout = new QVBoxLayout(displayFrame);
    displayLayout->setContentsMargins(15, 15, 15, 15);
    displayLayout->setSpacing(15);
    
    QLabel *displayTitle = new QLabel("📖 聊天记录", displayFrame);
    UIStyleManager::applyLabelStyle(displayTitle, "subtitle");
    displayLayout->addWidget(displayTitle);
    
    m_messagesList = new QListWidget(displayFrame);
    m_messagesList->setAlternatingRowColors(true);
    displayLayout->addWidget(m_messagesList);
    
    chatLayout->addWidget(controlFrame);
    chatLayout->addWidget(displayFrame);
    
    m_tabWidget->addTab(chatTab, "💬 聊天存储");
    
    // 连接信号
    connect(m_sendButton, &QPushButton::clicked, this, &ExampleUsageWidget::sendTestMessage);
    connect(m_loadButton, &QPushButton::clicked, this, &ExampleUsageWidget::loadChatHistory);
    connect(m_clearButton, &QPushButton::clicked, this, &ExampleUsageWidget::clearChatHistory);
    connect(m_exportButton, &QPushButton::clicked, this, &ExampleUsageWidget::exportChatData);
    
    // 按回车发送消息
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ExampleUsageWidget::sendTestMessage);
}

void ExampleUsageWidget::setupStyleDemoTab()
{
    QWidget *styleTab = new QWidget();
    QVBoxLayout *styleLayout = new QVBoxLayout(styleTab);
    UIStyleManager::applyContainerSpacing(styleTab);
    
    // 标题
    QLabel *styleTitle = new QLabel("🎨 UI样式演示", styleTab);
    UIStyleManager::applyLabelStyle(styleTitle, "title");
    styleTitle->setAlignment(Qt::AlignCenter);
    styleLayout->addWidget(styleTitle);
    
    // 按钮样式演示
    QGroupBox *buttonGroup = new QGroupBox("按钮样式", styleTab);
    UIStyleManager::applyGroupBoxStyle(buttonGroup);
    QGridLayout *buttonLayout = new QGridLayout(buttonGroup);
    
    QPushButton *primaryBtn = new QPushButton("主要按钮", buttonGroup);
    UIStyleManager::applyButtonStyle(primaryBtn, "primary");
    
    QPushButton *secondaryBtn = new QPushButton("次要按钮", buttonGroup);
    UIStyleManager::applyButtonStyle(secondaryBtn, "secondary");
    
    QPushButton *successBtn = new QPushButton("成功按钮", buttonGroup);
    UIStyleManager::applyButtonStyle(successBtn, "success");
    
    QPushButton *warningBtn = new QPushButton("警告按钮", buttonGroup);
    UIStyleManager::applyButtonStyle(warningBtn, "warning");
    
    QPushButton *errorBtn = new QPushButton("错误按钮", buttonGroup);
    UIStyleManager::applyButtonStyle(errorBtn, "error");
    
    QPushButton *disabledBtn = new QPushButton("禁用按钮", buttonGroup);
    UIStyleManager::applyButtonStyle(disabledBtn, "primary");
    disabledBtn->setEnabled(false);
    
    buttonLayout->addWidget(primaryBtn, 0, 0);
    buttonLayout->addWidget(secondaryBtn, 0, 1);
    buttonLayout->addWidget(successBtn, 1, 0);
    buttonLayout->addWidget(warningBtn, 1, 1);
    buttonLayout->addWidget(errorBtn, 2, 0);
    buttonLayout->addWidget(disabledBtn, 2, 1);
    
    styleLayout->addWidget(buttonGroup);
    
    // 文本样式演示
    QGroupBox *textGroup = new QGroupBox("文本样式", styleTab);
    UIStyleManager::applyGroupBoxStyle(textGroup);
    QVBoxLayout *textLayout = new QVBoxLayout(textGroup);
    
    QLabel *titleDemo = new QLabel("这是标题文本", textGroup);
    UIStyleManager::applyLabelStyle(titleDemo, "title");
    
    QLabel *subtitleDemo = new QLabel("这是副标题文本", textGroup);
    UIStyleManager::applyLabelStyle(subtitleDemo, "subtitle");
    
    QLabel *normalDemo = new QLabel("这是普通文本", textGroup);
    UIStyleManager::applyLabelStyle(normalDemo, "normal");
    
    QLabel *captionDemo = new QLabel("这是说明文本", textGroup);
    UIStyleManager::applyLabelStyle(captionDemo, "caption");
    
    QLabel *successDemo = new QLabel("这是成功状态文本", textGroup);
    UIStyleManager::applyLabelStyle(successDemo, "success");
    
    QLabel *warningDemo = new QLabel("这是警告状态文本", textGroup);
    UIStyleManager::applyLabelStyle(warningDemo, "warning");
    
    QLabel *errorDemo = new QLabel("这是错误状态文本", textGroup);
    UIStyleManager::applyLabelStyle(errorDemo, "error");
    
    textLayout->addWidget(titleDemo);
    textLayout->addWidget(subtitleDemo);
    textLayout->addWidget(normalDemo);
    textLayout->addWidget(captionDemo);
    textLayout->addWidget(successDemo);
    textLayout->addWidget(warningDemo);
    textLayout->addWidget(errorDemo);
    
    styleLayout->addWidget(textGroup);
    
    // 输入框演示
    QGroupBox *inputGroup = new QGroupBox("输入控件", styleTab);
    UIStyleManager::applyGroupBoxStyle(inputGroup);
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
    
    QLineEdit *lineEdit = new QLineEdit(inputGroup);
    lineEdit->setPlaceholderText("这是一个输入框");
    UIStyleManager::applyLineEditStyle(lineEdit);
    
    QTextEdit *textEdit = new QTextEdit(inputGroup);
    textEdit->setPlaceholderText("这是一个多行文本编辑器...");
    textEdit->setMaximumHeight(100);
    UIStyleManager::applyTextEditStyle(textEdit);
    
    inputLayout->addWidget(new QLabel("单行输入框:"));
    inputLayout->addWidget(lineEdit);
    inputLayout->addWidget(new QLabel("多行文本框:"));
    inputLayout->addWidget(textEdit);
    
    styleLayout->addWidget(inputGroup);
    
    styleLayout->addStretch();
    
    m_tabWidget->addTab(styleTab, "🎨 样式演示");
}

void ExampleUsageWidget::sendTestMessage()
{
    QString sender = m_senderInput->text().trimmed();
    QString receiver = m_receiverInput->text().trimmed();
    QString message = m_messageInput->text().trimmed();
    
    if (sender.isEmpty() || receiver.isEmpty() || message.isEmpty()) {
        m_statusLabel->setText("❌ 请填写完整的消息信息");
        UIStyleManager::applyLabelStyle(m_statusLabel, "error");
        return;
    }
    
    if (m_chatStorage->insertMessage(sender, receiver, message)) {
        m_statusLabel->setText("✅ 消息发送成功");
        UIStyleManager::applyLabelStyle(m_statusLabel, "success");
        m_messageInput->clear();
        m_messageInput->setFocus();
    } else {
        m_statusLabel->setText("❌ 发送失败: " + m_chatStorage->getLastError());
        UIStyleManager::applyLabelStyle(m_statusLabel, "error");
    }
}

void ExampleUsageWidget::loadChatHistory()
{
    updateMessagesList();
    m_statusLabel->setText("📋 历史记录已更新");
    UIStyleManager::applyLabelStyle(m_statusLabel, "normal");
}

void ExampleUsageWidget::onMessageInserted(const Message &message)
{
    updateMessagesList();
}

void ExampleUsageWidget::clearChatHistory()
{
    int ret = QMessageBox::question(this, "确认删除", 
                                   "确定要清空所有聊天记录吗？此操作不可恢复！",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_chatStorage->deleteAllMessages()) {
            m_statusLabel->setText("🗑️ 所有记录已清空");
            UIStyleManager::applyLabelStyle(m_statusLabel, "warning");
            updateMessagesList();
        } else {
            m_statusLabel->setText("❌ 清空失败: " + m_chatStorage->getLastError());
            UIStyleManager::applyLabelStyle(m_statusLabel, "error");
        }
    }
}

void ExampleUsageWidget::exportChatData()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                   "导出聊天数据",
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + 
                                                   "/chat_export_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".json",
                                                   "JSON files (*.json)");
    
    if (!fileName.isEmpty()) {
        QList<Message> messages = m_chatStorage->getAllMessages();
        if (m_chatStorage->exportToFile(fileName, messages)) {
            m_statusLabel->setText(QString("💾 已导出 %1 条记录").arg(messages.size()));
            UIStyleManager::applyLabelStyle(m_statusLabel, "success");
        } else {
            m_statusLabel->setText("❌ 导出失败: " + m_chatStorage->getLastError());
            UIStyleManager::applyLabelStyle(m_statusLabel, "error");
        }
    }
}

void ExampleUsageWidget::updateMessagesList()
{
    m_messagesList->clear();
    
    QList<Message> messages = m_chatStorage->getAllMessages();
    for (const Message &msg : messages) {
        QString itemText = QString("[%1] %2 → %3: %4")
                          .arg(msg.timestamp.toString("MM-dd hh:mm"))
                          .arg(msg.sender)
                          .arg(msg.receiver)
                          .arg(msg.message);
        
        QListWidgetItem *item = new QListWidgetItem(itemText);
        
        // 根据发送者设置不同颜色
        if (msg.sender.contains("患者")) {
            item->setForeground(QColor(UIStyleManager::colors.primary));
        } else if (msg.sender.contains("客服")) {
            item->setForeground(QColor(UIStyleManager::colors.success));
        }
        
        m_messagesList->addItem(item);
    }
    
    // 滚动到底部
    m_messagesList->scrollToBottom();
} 
