#include "FAQWidget.h"
#include <QListWidgetItem>
#include <QMessageBox>

FAQWidget::FAQWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_searchLayout(nullptr)
    , m_contentLayout(nullptr)
    , m_searchEdit(nullptr)
    , m_btnSearch(nullptr)
    , m_categoryGroup(nullptr)
    , m_categoryList(nullptr)
    , m_faqGroup(nullptr)
    , m_faqList(nullptr)
    , m_answerGroup(nullptr)
    , m_answerDisplay(nullptr)
    , m_btnHelpful(nullptr)
    , m_btnNotHelpful(nullptr)
    , m_currentCategory("全部")
{
    setupUI();
    loadFAQData();
}

void FAQWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);
    
    // 搜索区域
    m_searchLayout = new QHBoxLayout;
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("搜索问题关键词...");
    m_searchEdit->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #E5E5EA;
            border-radius: 8px;
            padding: 12px 16px;
            font-size: 16px;
        }
        QLineEdit:focus {
            border-color: #007AFF;
        }
    )");
    
    m_btnSearch = new QPushButton("搜索");
    m_btnSearch->setStyleSheet(R"(
        QPushButton {
            background-color: #007AFF;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0056CC;
        }
    )");
    
    connect(m_searchEdit, &QLineEdit::textChanged, this, &FAQWidget::onSearchTextChanged);
    connect(m_btnSearch, &QPushButton::clicked, [this]() {
        filterFAQs(m_searchEdit->text());
    });
    
    m_searchLayout->addWidget(m_searchEdit);
    m_searchLayout->addWidget(m_btnSearch);
    m_searchEdit->hide();
    m_btnSearch->hide();
    
    // 内容区域
    m_contentLayout = new QHBoxLayout;
    
    // 左侧：分类和问题列表
    QWidget* leftWidget = new QWidget;
    leftWidget->setMaximumWidth(400);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    
    // 分类列表
    m_categoryGroup = new QGroupBox("问题分类");
    QVBoxLayout* categoryLayout = new QVBoxLayout(m_categoryGroup);
    m_categoryList = new QListWidget;
    m_categoryList->setMaximumHeight(150);
    connect(m_categoryList, &QListWidget::itemClicked, this, &FAQWidget::onCategoryChanged);
    categoryLayout->addWidget(m_categoryList);
    
    // 问题列表
    m_faqGroup = new QGroupBox("问题列表");
    QVBoxLayout* faqLayout = new QVBoxLayout(m_faqGroup);
    m_faqList = new QListWidget;
    connect(m_faqList, &QListWidget::itemClicked, this, &FAQWidget::onFAQItemClicked);
    faqLayout->addWidget(m_faqList);
    
    leftLayout->addWidget(m_categoryGroup);
    leftLayout->addWidget(m_faqGroup);
    m_categoryGroup->hide();
    
    // 右侧：答案显示
    m_answerGroup = new QGroupBox("详细解答");
    QVBoxLayout* answerLayout = new QVBoxLayout(m_answerGroup);
    
    m_answerDisplay = new QTextEdit;
    m_answerDisplay->setReadOnly(true);
    m_answerDisplay->setPlaceholderText("请从左侧选择问题查看详细解答...");
    
    // 反馈按钮
    QHBoxLayout* feedbackLayout = new QHBoxLayout;
    QLabel* feedbackLabel = new QLabel("这个答案对您有帮助吗？");
    m_btnHelpful = new QPushButton("有帮助");
    m_btnNotHelpful = new QPushButton("没帮助");
    m_btnHelpful->hide();
    m_btnNotHelpful->hide();
    feedbackLabel->hide();
    
    m_btnHelpful->setStyleSheet(R"(
        QPushButton {
            background-color: #34C759;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: #30B24A;
        }
    )");
    
    m_btnNotHelpful->setStyleSheet(R"(
        QPushButton {
            background-color: #FF3B30;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: #E5292F;
        }
    )");
    
    connect(m_btnHelpful, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "感谢反馈", "感谢您的反馈，我们会继续改进服务质量！");
    });
    
    connect(m_btnNotHelpful, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "感谢反馈", "抱歉没能帮到您，您可以尝试联系在线客服获得更多帮助。");
    });
    
    feedbackLayout->addWidget(feedbackLabel);
    feedbackLayout->addStretch();
    feedbackLayout->addWidget(m_btnHelpful);
    feedbackLayout->addWidget(m_btnNotHelpful);
    
    answerLayout->addWidget(m_answerDisplay);
    answerLayout->addLayout(feedbackLayout);
    
    // 添加到内容布局
    m_contentLayout->addWidget(leftWidget);
    m_contentLayout->addWidget(m_answerGroup);
    m_contentLayout->setStretch(0, 1);
    m_contentLayout->setStretch(1, 2);
    
    // 添加到主布局
    m_mainLayout->addLayout(m_searchLayout);
    m_mainLayout->addLayout(m_contentLayout);
    
    // 样式设置
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #E5E5EA;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 10px 0 10px;
        }
        QListWidget {
            border: 1px solid #CED4DA;
            border-radius: 6px;
            background-color: white;
        }
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #F1F3F4;
        }
        QListWidget::item:selected {
            background-color: #007AFF;
            color: white;
        }
        QListWidget::item:hover {
            background-color: #F2F2F7;
        }
        QTextEdit {
            border: 1px solid #CED4DA;
            border-radius: 6px;
            background-color: white;
            padding: 15px;
            font-size: 14px;
            line-height: 1.6;
        }
    )");
}

void FAQWidget::loadFAQData()
{
    // 添加分类
    QStringList categories = {"全部", "挂号预约", "就诊流程", "部门介绍", "检查检验", "医保报销", "其他服务"};
    for (const QString& category : categories) {
        QListWidgetItem* item = new QListWidgetItem(category);
        m_categoryList->addItem(item);
    }
    m_categoryList->setCurrentRow(0);
    
    // 添加FAQ数据
    m_faqData.clear();
    
    // 挂号预约
    m_faqData.append({"挂号预约", "如何网上预约挂号？", 
        "您可以通过以下方式进行网上预约：\n\n1. 关注公司官方微信公众号\n2. 下载公司官方APP\n3. 登录公司官网\n4. 拨打预约电话\n\n预约时需要提供您的身份证号码和手机号码。", 95});
    
    m_faqData.append({"挂号预约", "预约挂号需要提前多长时间？", 
        "我院预约挂号服务时间安排：\n\n• 普通门诊：可预约7天内号源\n• 专家门诊：可预约14天内号源\n• 特需门诊：可预约30天内号源\n\n建议您提前1-3天预约，以确保有合适的时间段。", 87});
    
    // 就诊流程
    m_faqData.append({"就诊流程", "初次就诊需要办理什么手续？", 
        "初次就诊需要办理以下手续：\n\n1. 携带有效身份证件\n2. 在一楼大厅办理就诊卡\n3. 如有医保请带医保卡\n4. 到相应部门取号就诊\n\n办卡时间：周一至周日 7:00-17:00", 112});
    
    m_faqData.append({"就诊流程", "就诊当天的流程是什么？", 
        "就诊当天流程：\n\n1. 到院取号（如已预约）\n2. 部门候诊\n3. 医生诊疗\n4. 缴费取药/检查\n5. 复诊预约（如需要）\n\n请提前30分钟到达公司，留出足够时间。", 98});
    
    // 部门介绍
    m_faqData.append({"部门介绍", "各部门的位置在哪里？", 
        "部门分布：\n\n• 1楼：挂号收费、安保部、惩戒部\n• 2楼：控制部、福利部门诊\n• 3楼：妇产科、培训部\n• 4楼：眼科、耳鼻喉科\n• 5楼：检验科、影像科\n\n详细位置请参考院内导向标识。", 76});
    
    // 检查检验
    m_faqData.append({"检查检验", "检查前需要注意什么？", 
        "不同检查的注意事项：\n\n• 血常规：无需空腹\n• 生化检查：需空腹8-12小时\n• B超检查：根据部位确定是否需要憋尿\n• CT/MRI：请提前告知是否有金属植入物\n\n具体要求请咨询开单医生。", 89});
    
    // 医保报销
    m_faqData.append({"医保报销", "医保报销比例是多少？", 
        "医保报销比例：\n\n• 职工医保：门诊70-90%，住院85-95%\n• 居民医保：门诊50-70%，住院70-85%\n• 新农合：门诊50-60%，住院60-80%\n\n具体比例因地区和政策而异，请咨询医保窗口。", 134});
    
    // 其他服务
    m_faqData.append({"其他服务", "公司有停车场吗？", 
        "公司停车信息：\n\n• 地下停车场：300个车位\n• 地面停车场：150个车位\n• 收费标准：前30分钟免费，之后每小时5元\n• 开放时间：24小时\n\n建议使用公共交通工具。", 67});
    
    m_filteredData = m_faqData;
    filterFAQs("");
}

void FAQWidget::onSearchTextChanged(const QString& text)
{
    if (text.length() >= 2) {
        filterFAQs(text);
    } else if (text.isEmpty()) {
        filterFAQs("");
    }
}

void FAQWidget::onCategoryChanged()
{
    QListWidgetItem* item = m_categoryList->currentItem();
    if (item) {
        m_currentCategory = item->text();
        filterFAQs(m_searchEdit->text());
    }
}

void FAQWidget::onFAQItemClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    int index = m_faqList->row(item);
    if (index >= 0 && index < m_filteredData.size()) {
        const FAQItem& faq = m_filteredData[index];
        
        QString formattedAnswer = faq.answer;
        formattedAnswer.replace("\n", "<br>");
        
        // QString displayText = QString("<h3>%1</h3><hr><p>%2</p><br><small><i>👍 %3 人觉得有帮助</i></small>")
        //                      .arg(faq.question)
        //                      .arg(formattedAnswer)
        //                      .arg(faq.helpfulCount);

        QString displayText = QString("<h3>%1</h3><hr><p>%2</p><br><small></small>")
                                  .arg(faq.question)
                                  .arg(formattedAnswer);

        
        m_answerDisplay->setHtml(displayText);
    }
}

void FAQWidget::filterFAQs(const QString& keyword)
{
    m_filteredData.clear();
    m_faqList->clear();
    
    for (const FAQItem& faq : m_faqData) {
        bool matchCategory = (m_currentCategory == "全部" || faq.category == m_currentCategory);
        bool matchKeyword = keyword.isEmpty() || 
                           faq.question.contains(keyword, Qt::CaseInsensitive) ||
                           faq.answer.contains(keyword, Qt::CaseInsensitive);
        
        if (matchCategory && matchKeyword) {
            m_filteredData.append(faq);
            
            QString itemText = QString("%1").arg(faq.question);
            QListWidgetItem* item = new QListWidgetItem(itemText);
            m_faqList->addItem(item);
        }
    }
    
    // 清空答案显示
    m_answerDisplay->clear();
} 
