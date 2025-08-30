#ifndef KNOWLEDGEBASEWIDGET_H
#define KNOWLEDGEBASEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QSplitter>

class KnowledgeBaseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KnowledgeBaseWidget(QWidget *parent = nullptr);

private:
    void setupUI();
    void loadKnowledgeBase();
    
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_searchLayout;
    QLineEdit* m_searchEdit;
    QPushButton* m_searchButton;
    QSplitter* m_splitter;
    QTreeWidget* m_categoryTree;
    QTextEdit* m_contentArea;
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
};

#endif // KNOWLEDGEBASEWIDGET_H 