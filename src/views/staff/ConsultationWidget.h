#ifndef CONSULTATIONWIDGET_H
#define CONSULTATIONWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <QSplitter>

class ConsultationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConsultationWidget(QWidget *parent = nullptr);

private:
    void setupUI();
    
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    QListWidget* m_chatList;
    QWidget* m_chatArea;
    QVBoxLayout* m_chatLayout;
    QTextEdit* m_messageArea;
    QHBoxLayout* m_inputLayout;
    QTextEdit* m_inputEdit;
    QPushButton* m_sendButton;
};

#endif // CONSULTATIONWIDGET_H 