#include "RegisterDialog.h"
#include <QPainter>
#include <QLinearGradient>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTimer>

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("医院智慧客服系统- 注册账号");
    setFixedSize(600, 700);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    setupUI();
    setupStyles();
    
    // 设置默认焦点
    m_usernameEdit->setFocus();
}

void RegisterDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 滚动区域
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setObjectName("scrollArea");
    
    // 主框架
    m_registerFrame = new QFrame;
    m_registerFrame->setObjectName("registerFrame");
    m_frameLayout = new QVBoxLayout(m_registerFrame);
    m_frameLayout->setContentsMargins(20, 20, 20, 20);
    m_frameLayout->setSpacing(20);
    
    // 标题区域
    m_titleLabel = new QLabel("创建账号");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setObjectName("titleLabel");
    
    m_subtitleLabel = new QLabel("请填写以下信息完成注册");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_subtitleLabel->setObjectName("subtitleLabel");
    
    // 输入区域
    m_inputFrame = new QFrame;
    m_inputFrame->setObjectName("inputFrame");
    m_inputLayout = new QGridLayout(m_inputFrame);
    m_inputLayout->setSpacing(12);
    m_inputLayout->setContentsMargins(25, 25, 25, 25);
    
    int row = 0;
    
    // 角色选择
    m_roleLabel = new QLabel("注册身份:");
    m_roleLabel->setObjectName("inputLabel");
    m_roleCombo = new QComboBox;
    m_roleCombo->addItem("👨‍⚕️ 患者", "患者");
    // m_roleCombo->addItem("👩‍💼 客服", "客服");
    // m_roleCombo->addItem("👨‍💻 管理员", "管理员");
    m_roleCombo->setObjectName("roleCombo");
    connect(m_roleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &RegisterDialog::onRoleChanged);
    
    m_inputLayout->addWidget(m_roleLabel, row, 0);
    m_inputLayout->addWidget(m_roleCombo, row, 1);
    row++;
    
    // 用户名
    m_usernameLabel = new QLabel("用户名:");
    m_usernameLabel->setObjectName("inputLabel");
    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_usernameEdit->setObjectName("inputEdit");
    connect(m_usernameEdit, &QLineEdit::textChanged, this, &RegisterDialog::onUsernameChanged);
    
    m_usernameHint = new QLabel("3-20个字符，支持字母、数字、下划线");
    m_usernameHint->setObjectName("hintLabel");
    
    m_inputLayout->addWidget(m_usernameLabel, row, 0);
    m_inputLayout->addWidget(m_usernameEdit, row, 1);
    row++;
    m_inputLayout->addWidget(m_usernameHint, row, 1);
    row++;
    
    // 真实姓名
    m_realNameLabel = new QLabel("真实姓名:");
    m_realNameLabel->setObjectName("inputLabel");
    m_realNameEdit = new QLineEdit;
    m_realNameEdit->setPlaceholderText("请输入真实姓名");
    m_realNameEdit->setObjectName("inputEdit");
    connect(m_realNameEdit, &QLineEdit::textChanged, [this]() { updateRegisterButtonState(); });
    
    m_inputLayout->addWidget(m_realNameLabel, row, 0);
    m_inputLayout->addWidget(m_realNameEdit, row, 1);
    row++;
    
    // 邮箱
    m_emailLabel = new QLabel("邮箱:");
    m_emailLabel->setObjectName("inputLabel");
    m_emailEdit = new QLineEdit;
    m_emailEdit->setPlaceholderText("请输入邮箱地址");
    m_emailEdit->setObjectName("inputEdit");
    connect(m_emailEdit, &QLineEdit::textChanged, this, &RegisterDialog::onEmailChanged);
    
    m_emailHint = new QLabel("用于找回密码和接收通知");
    m_emailHint->setObjectName("hintLabel");
    
    m_inputLayout->addWidget(m_emailLabel, row, 0);
    m_inputLayout->addWidget(m_emailEdit, row, 1);
    row++;
    m_inputLayout->addWidget(m_emailHint, row, 1);
    row++;
    
    // 手机号
    m_phoneLabel = new QLabel("手机号:");
    m_phoneLabel->setObjectName("inputLabel");
    m_phoneEdit = new QLineEdit;
    m_phoneEdit->setPlaceholderText("请输入手机号（可选）");
    m_phoneEdit->setObjectName("inputEdit");
    
    m_inputLayout->addWidget(m_phoneLabel, row, 0);
    m_inputLayout->addWidget(m_phoneEdit, row, 1);
    row++;
    
    // 密码
    m_passwordLabel = new QLabel("密码:");
    m_passwordLabel->setObjectName("inputLabel");
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setObjectName("inputEdit");
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &RegisterDialog::onPasswordChanged);
    
    m_passwordStrength = new QProgressBar;
    m_passwordStrength->setRange(0, 100);
    m_passwordStrength->setValue(0);
    m_passwordStrength->setObjectName("passwordStrength");
    m_passwordStrength->setFixedHeight(8);
    
    m_passwordHint = new QLabel("至少6个字符，建议包含大小写字母、数字和特殊字符");
    m_passwordHint->setObjectName("hintLabel");
    
    m_inputLayout->addWidget(m_passwordLabel, row, 0);
    m_inputLayout->addWidget(m_passwordEdit, row, 1);
    row++;
    m_inputLayout->addWidget(m_passwordStrength, row, 1);
    row++;
    m_inputLayout->addWidget(m_passwordHint, row, 1);
    row++;
    
    // 确认密码
    m_confirmPasswordLabel = new QLabel("确认密码:");
    m_confirmPasswordLabel->setObjectName("inputLabel");
    m_confirmPasswordEdit = new QLineEdit;
    m_confirmPasswordEdit->setPlaceholderText("请再次输入密码");
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setObjectName("inputEdit");
    connect(m_confirmPasswordEdit, &QLineEdit::textChanged, this, &RegisterDialog::onConfirmPasswordChanged);
    
    m_confirmPasswordHint = new QLabel;
    m_confirmPasswordHint->setObjectName("hintLabel");
    
    m_inputLayout->addWidget(m_confirmPasswordLabel, row, 0);
    m_inputLayout->addWidget(m_confirmPasswordEdit, row, 1);
    row++;
    m_inputLayout->addWidget(m_confirmPasswordHint, row, 1);
    row++;
    
    // 按钮区域
    m_buttonFrame = new QFrame;
    m_buttonFrame->setObjectName("buttonFrame");
    m_buttonLayout = new QHBoxLayout(m_buttonFrame);
    m_buttonLayout->setSpacing(15);
    
    m_cancelButton = new QPushButton("取消");
    m_cancelButton->setObjectName("cancelButton");
    connect(m_cancelButton, &QPushButton::clicked, this, &RegisterDialog::onCancelClicked);
    
    m_registerButton = new QPushButton("注册");
    m_registerButton->setObjectName("registerButton");
    m_registerButton->setEnabled(false);
    connect(m_registerButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_registerButton);
    
    // 消息标签
    m_messageLabel = new QLabel;
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setObjectName("messageLabel");
    m_messageLabel->setWordWrap(true);
    m_messageLabel->hide();
    
    // 组装布局
    m_frameLayout->addWidget(m_titleLabel);
    m_frameLayout->addWidget(m_subtitleLabel);
    m_frameLayout->addSpacing(10);
    m_frameLayout->addWidget(m_inputFrame);
    m_frameLayout->addWidget(m_buttonFrame);
    m_frameLayout->addWidget(m_messageLabel);
    
    m_scrollArea->setWidget(m_registerFrame);
    m_mainLayout->addWidget(m_scrollArea);
}

void RegisterDialog::setupStyles()
{
    setStyleSheet(R"(
        RegisterDialog {
            background: transparent;
        }
        
        QScrollArea#scrollArea {
            background: transparent;
            border: none;
        }
        
        QFrame#registerFrame {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #ffffff, stop:1 #f8f9fa);
            border: 1px solid #e9ecef;
            border-radius: 20px;
        }
        
        QLabel#titleLabel {
            font-size: 28px;
            font-weight: bold;
            color: #2c3e50;
            margin: 10px;
        }
        
        QLabel#subtitleLabel {
            font-size: 14px;
            color: #6c757d;
            margin: 5px;
        }
        
        QFrame#inputFrame {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #f8f9fa, stop:1 #ffffff);
            border: 1px solid #dee2e6;
            border-radius: 15px;
        }
        
        QLabel#inputLabel {
            font-size: 14px;
            font-weight: bold;
            color: #495057;
            min-width: 80px;
        }
        
        QLabel#hintLabel {
            font-size: 12px;
            color: #6c757d;
        }
        
        QLineEdit#inputEdit {
            padding: 12px 16px;
            border: 2px solid #e9ecef;
            border-radius: 10px;
            font-size: 14px;
            background: white;
            selection-background-color: #007bff;
        }
        
        QLineEdit#inputEdit:focus {
            border-color: #007bff;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #ffffff, stop:1 #f8f9ff);
        }
        
        QComboBox#roleCombo {
            padding: 12px 16px;
            border: 2px solid #e9ecef;
            border-radius: 10px;
            font-size: 14px;
            background: white;
            min-width: 200px;
        }
        
        QComboBox#roleCombo:focus {
            border-color: #007bff;
        }
        
        QProgressBar#passwordStrength {
            border: 1px solid #dee2e6;
            border-radius: 4px;
            text-align: center;
            background: #f8f9fa;
        }
        
        QProgressBar#passwordStrength::chunk {
            border-radius: 3px;
        }
        
        QPushButton#registerButton {
            padding: 12px 24px;
            font-size: 14px;
            font-weight: bold;
            color: white;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #28a745, stop:1 #1e7e34);
            border: none;
            border-radius: 10px;
            min-width: 100px;
        }
        
        QPushButton#registerButton:hover:enabled {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #1e7e34, stop:1 #155724);
        }
        
        QPushButton#registerButton:disabled {
            background: #e9ecef;
            color: #6c757d;
        }
        
        QPushButton#cancelButton {
            padding: 12px 24px;
            font-size: 14px;
            font-weight: bold;
            color: #6c757d;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #f8f9fa, stop:1 #e9ecef);
            border: 2px solid #dee2e6;
            border-radius: 10px;
            min-width: 100px;
        }
        
        QPushButton#cancelButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #e9ecef, stop:1 #dee2e6);
            border-color: #adb5bd;
        }
        
        QLabel#messageLabel {
            padding: 10px 15px;
            border-radius: 8px;
            font-size: 13px;
            font-weight: bold;
        }
        
        QLabel#messageLabel[error="true"] {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        
        QLabel#messageLabel[error="false"] {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
    )");
}

void RegisterDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景渐变
    QLinearGradient gradient(rect().topLeft(), rect().bottomRight());
    gradient.setColorAt(0, QColor(40, 167, 69, 200));
    gradient.setColorAt(1, QColor(25, 135, 84, 200));
    
    painter.fillRect(rect(), gradient);
    
    QDialog::paintEvent(event);
}

void RegisterDialog::setSelectedRole(const QString& role)
{
    for (int i = 0; i < m_roleCombo->count(); ++i) {
        if (m_roleCombo->itemData(i).toString() == role) {
            m_roleCombo->setCurrentIndex(i);
            break;
        }
    }
}

void RegisterDialog::onRegisterClicked()
{
    if (!validateInput()) {
        return;
    }
    
    clearMessage();
    
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    QString email = m_emailEdit->text().trimmed();
    QString phone = m_phoneEdit->text().trimmed();
    QString role = m_roleCombo->currentData().toString();
    QString realName = m_realNameEdit->text().trimmed();
    
    // 尝试注册
    DatabaseManager* db = DatabaseManager::instance();
    if (db->registerUser(username, password, email, phone, role, realName)) {
        // 获取注册成功的用户信息
        if (db->loginUser(username, password, m_registeredUser)) {
            showMessage("注册成功！", false);
            QTimer::singleShot(1000, this, &QDialog::accept);
        } else {
            showMessage("注册成功，但获取用户信息失败！", true);
        }
    } else {
        showMessage("注册失败，可能用户名或邮箱已存在！", true);
    }
}

void RegisterDialog::onCancelClicked()
{
    reject();
}

void RegisterDialog::onUsernameChanged()
{
    QString username = m_usernameEdit->text().trimmed();
    
    if (username.isEmpty()) {
        m_usernameHint->setText("3-20个字符，支持字母、数字、下划线");
        m_usernameHint->setStyleSheet("color: #6c757d;");
    } else if (username.length() < 3) {
        m_usernameHint->setText("用户名太短");
        m_usernameHint->setStyleSheet("color: #dc3545;");
    } else if (username.length() > 20) {
        m_usernameHint->setText("用户名太长");
        m_usernameHint->setStyleSheet("color: #dc3545;");
    } else {
        QRegularExpression regex("^[a-zA-Z0-9_]+$");
        if (!regex.match(username).hasMatch()) {
            m_usernameHint->setText("只能包含字母、数字、下划线");
            m_usernameHint->setStyleSheet("color: #dc3545;");
        } else {
            // 检查用户名是否已存在
            DatabaseManager* db = DatabaseManager::instance();
            if (db->isUsernameExists(username)) {
                m_usernameHint->setText("用户名已存在");
                m_usernameHint->setStyleSheet("color: #dc3545;");
            } else {
                m_usernameHint->setText("✓ 用户名可用");
                m_usernameHint->setStyleSheet("color: #28a745;");
            }
        }
    }
    
    updateRegisterButtonState();
}

void RegisterDialog::onPasswordChanged()
{
    updatePasswordStrength();
    onConfirmPasswordChanged(); // 重新验证确认密码
    updateRegisterButtonState();
}

void RegisterDialog::onConfirmPasswordChanged()
{
    QString password = m_passwordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();
    
    if (confirmPassword.isEmpty()) {
        m_confirmPasswordHint->setText("");
    } else if (password == confirmPassword) {
        m_confirmPasswordHint->setText("✓ 密码匹配");
        m_confirmPasswordHint->setStyleSheet("color: #28a745;");
    } else {
        m_confirmPasswordHint->setText("密码不匹配");
        m_confirmPasswordHint->setStyleSheet("color: #dc3545;");
    }
    
    updateRegisterButtonState();
}

void RegisterDialog::onEmailChanged()
{
    QString email = m_emailEdit->text().trimmed();
    
    if (email.isEmpty()) {
        m_emailHint->setText("用于找回密码和接收通知");
        m_emailHint->setStyleSheet("color: #6c757d;");
    } else {
        QRegularExpression regex("^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$");
        if (!regex.match(email).hasMatch()) {
            m_emailHint->setText("邮箱格式不正确");
            m_emailHint->setStyleSheet("color: #dc3545;");
        } else {
            // 检查邮箱是否已存在
            DatabaseManager* db = DatabaseManager::instance();
            if (db->isEmailExists(email)) {
                m_emailHint->setText("邮箱已被注册");
                m_emailHint->setStyleSheet("color: #dc3545;");
            } else {
                m_emailHint->setText("✓ 邮箱可用");
                m_emailHint->setStyleSheet("color: #28a745;");
            }
        }
    }
    
    updateRegisterButtonState();
}

void RegisterDialog::onRoleChanged()
{
    clearMessage();
}

void RegisterDialog::updatePasswordStrength()
{
    QString password = m_passwordEdit->text();
    int strength = calculatePasswordStrength(password);
    
    m_passwordStrength->setValue(strength);
    
    // 设置进度条颜色
    QString color;
    QString text;
    
    if (strength < 30) {
        color = "#dc3545"; // 红色
        text = "弱";
    } else if (strength < 60) {
        color = "#ffc107"; // 黄色
        text = "中等";
    } else if (strength < 80) {
        color = "#fd7e14"; // 橙色
        text = "强";
    } else {
        color = "#28a745"; // 绿色
        text = "很强";
    }
    
    m_passwordStrength->setStyleSheet(QString(R"(
        QProgressBar#passwordStrength::chunk {
            background: %1;
            border-radius: 3px;
        }
    )").arg(color));
    
    if (password.isEmpty()) {
        m_passwordHint->setText("至少6个字符，建议包含大小写字母、数字和特殊字符");
        m_passwordHint->setStyleSheet("color: #6c757d;");
    } else {
        m_passwordHint->setText(QString("密码强度: %1").arg(text));
        m_passwordHint->setStyleSheet(QString("color: %1;").arg(color));
    }
}

int RegisterDialog::calculatePasswordStrength(const QString& password)
{
    if (password.isEmpty()) return 0;
    
    int score = 0;
    
    // 长度评分
    if (password.length() >= 6) score += 20;
    if (password.length() >= 8) score += 10;
    if (password.length() >= 12) score += 10;
    
    // 字符类型评分
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    
    for (const QChar& ch : password) {
        if (ch.isLower()) hasLower = true;
        else if (ch.isUpper()) hasUpper = true;
        else if (ch.isDigit()) hasDigit = true;
        else hasSpecial = true;
    }
    
    if (hasLower) score += 15;
    if (hasUpper) score += 15;
    if (hasDigit) score += 15;
    if (hasSpecial) score += 15;
    
    return qMin(score, 100);
}

void RegisterDialog::updateRegisterButtonState()
{
    // 检查所有必填字段
    bool canRegister = !m_usernameEdit->text().trimmed().isEmpty() &&
                      !m_passwordEdit->text().isEmpty() &&
                      !m_confirmPasswordEdit->text().isEmpty() &&
                      !m_emailEdit->text().trimmed().isEmpty() &&
                      !m_realNameEdit->text().trimmed().isEmpty() &&
                      m_passwordEdit->text() == m_confirmPasswordEdit->text() &&
                      m_passwordEdit->text().length() >= 6;
    
    m_registerButton->setEnabled(canRegister);
}

bool RegisterDialog::validateInput()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();
    QString email = m_emailEdit->text().trimmed();
    QString realName = m_realNameEdit->text().trimmed();
    
    // 验证用户名
    if (username.isEmpty()) {
        showMessage("请输入用户名！", true);
        m_usernameEdit->setFocus();
        return false;
    }
    
    if (username.length() < 3 || username.length() > 20) {
        showMessage("用户名长度应为3-20个字符！", true);
        m_usernameEdit->setFocus();
        return false;
    }
    
    QRegularExpression usernameRegex("^[a-zA-Z0-9_]+$");
    if (!usernameRegex.match(username).hasMatch()) {
        showMessage("用户名只能包含字母、数字、下划线！", true);
        m_usernameEdit->setFocus();
        return false;
    }
    
    // 验证真实姓名
    if (realName.isEmpty()) {
        showMessage("请输入真实姓名！", true);
        m_realNameEdit->setFocus();
        return false;
    }
    
    // 验证邮箱
    if (email.isEmpty()) {
        showMessage("请输入邮箱地址！", true);
        m_emailEdit->setFocus();
        return false;
    }
    
    QRegularExpression emailRegex("^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        showMessage("邮箱格式不正确！", true);
        m_emailEdit->setFocus();
        return false;
    }
    
    // 验证密码
    if (password.isEmpty()) {
        showMessage("请输入密码！", true);
        m_passwordEdit->setFocus();
        return false;
    }
    
    if (password.length() < 6) {
        showMessage("密码至少需要6个字符！", true);
        m_passwordEdit->setFocus();
        return false;
    }
    
    // 验证确认密码
    if (password != confirmPassword) {
        showMessage("两次输入的密码不一致！", true);
        m_confirmPasswordEdit->setFocus();
        return false;
    }
    
    return true;
}

void RegisterDialog::showMessage(const QString& message, bool isError)
{
    m_messageLabel->setText(message);
    m_messageLabel->setProperty("error", isError);
    m_messageLabel->style()->unpolish(m_messageLabel);
    m_messageLabel->style()->polish(m_messageLabel);
    m_messageLabel->show();
}

void RegisterDialog::clearMessage()
{
    m_messageLabel->hide();
} 
