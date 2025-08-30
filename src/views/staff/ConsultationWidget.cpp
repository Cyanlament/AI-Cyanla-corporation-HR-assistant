#include "ConsultationWidget.h"

ConsultationWidget::ConsultationWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ConsultationWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    m_splitter = new QSplitter(Qt::Horizontal);
    
    // 聊天列表
    m_chatList = new QListWidget;
    m_chatList->setMaximumWidth(250);
    m_chatList->addItem("访客001 - 咨询挂号");
    m_chatList->addItem("访客002 - 检查报告");
    m_chatList->addItem("访客003 - 用药咨询");
    
    // 聊天区域
    m_chatArea = new QWidget;
    m_chatLayout = new QVBoxLayout(m_chatArea);
    
    m_messageArea = new QTextEdit;
    m_messageArea->setReadOnly(true);
    m_messageArea->setPlaceholderText("选择访客开始咨询...");
    
    m_inputLayout = new QHBoxLayout;
    m_inputEdit = new QTextEdit;
    m_inputEdit->setMaximumHeight(80);
    m_inputEdit->setPlaceholderText("输入回复消息...");
    
    m_sendButton = new QPushButton("发送");
    m_sendButton->setFixedSize(80, 30);
    
    m_inputLayout->addWidget(m_inputEdit);
    m_inputLayout->addWidget(m_sendButton);
    
    m_chatLayout->addWidget(m_messageArea);
    m_chatLayout->addLayout(m_inputLayout);
    
    m_splitter->addWidget(m_chatList);
    m_splitter->addWidget(m_chatArea);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    
    m_mainLayout->addWidget(m_splitter);
} 