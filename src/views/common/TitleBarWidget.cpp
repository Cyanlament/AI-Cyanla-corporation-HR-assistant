#include "TitleBarWidget.h"
#include <QTimer>
#include <QDateTime>

TitleBarWidget::TitleBarWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_titleLabel(nullptr)
    , m_userLabel(nullptr)
    , m_timeLabel(nullptr)
    , m_currentTitle("欢迎使用")
{
    setupUI();
    
    // 启动时间更新定时器
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TitleBarWidget::updateTime);
    timer->start(1000); // 每秒更新一次
    updateTime(); // 立即更新一次
}

void TitleBarWidget::setupUI()
{
    setFixedHeight(60);
    setStyleSheet(R"(
        TitleBarWidget {
            background-color: white;
            border-bottom: 1px solid #E5E5EA;
        }
        
        QLabel {
            color: #1D1D1F;
            font-weight: 500;
        }
        
        #titleLabel {
            font-size: 24px;
            font-weight: 600;
            color: #1D1D1F;
        }
        
        #userLabel {
            font-size: 14px;
            color: #6D6D70;
        }
        
        #timeLabel {
            font-size: 14px;
            color: #6D6D70;
        }
    )");
    
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(30, 10, 30, 10);
    
    // 标题标签
    m_titleLabel = new QLabel(m_currentTitle);
    m_titleLabel->setObjectName("titleLabel");
    
    // 弹性空间
    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    // 用户信息标签
    m_userLabel = new QLabel("用户: 未登录");
    m_userLabel->setObjectName("userLabel");
    
    // 时间标签
    m_timeLabel = new QLabel;
    m_timeLabel->setObjectName("timeLabel");
    
    // 添加到布局
    m_layout->addWidget(m_titleLabel);
    m_layout->addWidget(spacer);
    m_layout->addWidget(m_userLabel);
    m_layout->addWidget(m_timeLabel);
}

void TitleBarWidget::setTitle(const QString& title)
{
    m_currentTitle = title;
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

void TitleBarWidget::setUserInfo(const QString& username, const QString& role)
{
    if (m_userLabel) {
        QString userText = QString("用户: %1 (%2)").arg(username, role);
        m_userLabel->setText(userText);
    }
}

void TitleBarWidget::updateTime()
{
    if (m_timeLabel) {
        QDateTime current = QDateTime::currentDateTime();
        QString timeText = current.toString("yyyy-MM-dd hh:mm:ss");
        m_timeLabel->setText(timeText);
    }
} 