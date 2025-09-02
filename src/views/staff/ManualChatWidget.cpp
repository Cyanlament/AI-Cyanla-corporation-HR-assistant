#include "ManualChatWidget.h"
#include <QListWidgetItem>
#include <QMessageBox>

ManualChatWidget::ManualChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_contentLayout(nullptr)
    , m_userGroup(nullptr)
    , m_userList(nullptr)
    , m_chatGroup(nullptr)
    , m_chatDisplay(nullptr)
    , m_messageInput(nullptr)
    , m_btnSend(nullptr)
    , m_btnTransfer(nullptr)
{
    setupUI();
    loadUsers();
}

void ManualChatWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    m_contentLayout = new QHBoxLayout;
    
    // 用户列表
    m_userGroup = new QGroupBox("等待人工服务的用户");
    m_userGroup->setMaximumWidth(250);
    QVBoxLayout* userLayout = new QVBoxLayout(m_userGroup);
    
    m_userList = new QListWidget;
    connect(m_userList, &QListWidget::itemClicked, this, &ManualChatWidget::onUserSelected);
    userLayout->addWidget(m_userList);
    
    // 聊天区域
    m_chatGroup = new QGroupBox("客服对话");
    QVBoxLayout* chatLayout = new QVBoxLayout(m_chatGroup);
    
    m_chatDisplay = new QTextEdit;
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setPlaceholderText("选择用户开始人工服务...");
    
    QHBoxLayout* inputLayout = new QHBoxLayout;
    m_messageInput = new QLineEdit;
    m_messageInput->setPlaceholderText("输入回复内容...");
    m_btnSend = new QPushButton("发送");
    m_btnTransfer = new QPushButton("转回AI");
    
    connect(m_btnSend, &QPushButton::clicked, this, &ManualChatWidget::onSendMessage);
    connect(m_btnTransfer, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "转移", "已将用户转回AI处理");
    });
    
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_btnSend);
    inputLayout->addWidget(m_btnTransfer);
    
    chatLayout->addWidget(m_chatDisplay);
    chatLayout->addLayout(inputLayout);
    
    m_contentLayout->addWidget(m_userGroup);
    m_contentLayout->addWidget(m_chatGroup);
    m_contentLayout->setStretch(0, 1);
    m_contentLayout->setStretch(1, 3);
    
    m_mainLayout->addLayout(m_contentLayout);
    
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #E5E5EA;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #F1F3F4;
        }
        QListWidget::item:selected {
            background-color: #FF9500;
            color: white;
        }
    )");
}

void ManualChatWidget::loadUsers()
{
    QStringList users = {"hanser - 面试预约", "otto - 福利咨询", "血狼破军 - 停职结果"};
    for (const QString& user : users) {
        QListWidgetItem* item = new QListWidgetItem("🔴 " + user);
        m_userList->addItem(item);
    }
}

void ManualChatWidget::onSendMessage()
{
    QString message = m_messageInput->text().trimmed();
    if (!message.isEmpty()) {
        m_chatDisplay->append(QString("<div style='text-align: right; color: #007AFF;'><b>客服:</b> %1</div><br>").arg(message));
        m_messageInput->clear();
    }
}

void ManualChatWidget::onUserSelected()
{
    QListWidgetItem* item = m_userList->currentItem();
    if (item) {
        QString userName = item->text().mid(2); // 去掉🔴符号
        m_chatDisplay->setHtml(QString("<h3>正在为 %1 提供人工服务</h3><hr>").arg(userName));
    }
} 
