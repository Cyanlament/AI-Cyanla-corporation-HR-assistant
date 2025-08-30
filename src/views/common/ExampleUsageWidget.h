#ifndef EXAMPLEUSAGEWIDGET_H
#define EXAMPLEUSAGEWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QGroupBox>
#include <QFrame>

#include "HospitalNavigationWidget.h"
#include "../../core/ChatStorage.h"
#include "UIStyleManager.h"

class ExampleUsageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExampleUsageWidget(QWidget *parent = nullptr);
    ~ExampleUsageWidget();

private slots:
    void sendTestMessage();
    void loadChatHistory();
    void onMessageInserted(const Message &message);
    void clearChatHistory();
    void exportChatData();

private:
    void setupUI();
    void setupNavigationTab();
    void setupChatTab();
    void setupStyleDemoTab();
    
    void updateMessagesList();
    void showDemoButtons();

private:
    QTabWidget *m_tabWidget;
    
    // 导航系统相关
    HospitalNavigationWidget *m_navigationWidget;
    
    // 聊天存储相关
    ChatStorage *m_chatStorage;
    QTextEdit *m_messageDisplay;
    QLineEdit *m_senderInput;
    QLineEdit *m_receiverInput;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;
    QPushButton *m_loadButton;
    QPushButton *m_clearButton;
    QPushButton *m_exportButton;
    QListWidget *m_messagesList;
    QLabel *m_statusLabel;
    
    // 样式演示相关
    QWidget *m_styleDemoWidget;
};

#endif // EXAMPLEUSAGEWIDGET_H 