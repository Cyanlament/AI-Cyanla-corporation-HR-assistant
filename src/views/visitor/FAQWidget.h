#ifndef FAQWIDGET_H
#define FAQWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QGroupBox>
#include <QLabel>

class FAQWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FAQWidget(QWidget *parent = nullptr);

private slots:
    void onSearchTextChanged(const QString& text);
    void onFAQItemClicked(QListWidgetItem* item);
    void onCategoryChanged();

private:
    void setupUI();
    void loadFAQData();
    void filterFAQs(const QString& keyword);

private:
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_searchLayout;
    QHBoxLayout* m_contentLayout;
    
    // 搜索区域
    QLineEdit* m_searchEdit;
    QPushButton* m_btnSearch;
    
    // 左侧分类和问题列表
    QGroupBox* m_categoryGroup;
    QListWidget* m_categoryList;
    QGroupBox* m_faqGroup;
    QListWidget* m_faqList;
    
    // 右侧答案显示
    QGroupBox* m_answerGroup;
    QTextEdit* m_answerDisplay;
    QPushButton* m_btnHelpful;
    QPushButton* m_btnNotHelpful;
    
    // 数据
    struct FAQItem {
        QString category;
        QString question;
        QString answer;
        int helpfulCount;
    };
    
    QList<FAQItem> m_faqData;
    QList<FAQItem> m_filteredData;
    QString m_currentCategory;
};

#endif // FAQWIDGET_H 