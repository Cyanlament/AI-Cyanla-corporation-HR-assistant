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
    QStringList categories = {"全部", "一般类", "部门相关", "福利政策"};
    for (const QString& category : categories) {
        QListWidgetItem* item = new QListWidgetItem(category);
        m_categoryList->addItem(item);
    }
    m_categoryList->setCurrentRow(0);
    
    // 添加FAQ数据
    m_faqData.clear();
    

    m_faqData.append({"一般类", "青蓝公司到底是做什么的？听起来很神秘。",
        "我们是一家专注于前沿能源研发与应用的科技企业。\n我们的核心业务涉及对特殊能源形态的收容、研究与利用，旨在为解决全球能源危机提供革命性的解决方案。\n具体技术细节因商业机密不便透露，但可以保证我们所有的研究都致力于创造一个更美好的未来。", 95});
    
    m_faqData.append({"一般类", "公司的名字“青蓝”有什么含义？",
        "“青”取意于天空与海洋，代表无限的可能与深邃的未知；\n“蓝”则象征着理性、冷静与科技。\n青蓝合一，代表了我们公司探索未知、以科技造福人类的核心理念。", 87});
    

    m_faqData.append({"一般类", "工作环境安全吗？听说有风险。",
        "安全是我们最优先考虑的事项。\n公司为所有员工提供了行业顶尖的防护装备（我们称为EGO装备）、严格的操作规程和全面的应急培训。\n只要严格遵守规程，风险是完全可以控制的。\n同时，公司为员工提供了远超行业标准的保险和福利保障。", 112});
    
    m_faqData.append({"一般类", "公司的上下班时间是怎样的？需要加班吗？",
                      "公司实行标准工时制，每周工作5天，每天8小时。\n由于研发工作的特殊性，部分岗位可能需要根据项目进度进行弹性工作或轮班。\n所有加班都会严格按照国家规定支付加班工资或安排调休。\n在发生紧急突发事件时，公司可能会启动应急响应机制。", 95});

    m_faqData.append({"一般类", "对员工的着装有什么要求吗？",
                      "在日常办公区，公司要求穿着统一的工服，保持专业形象。\n在进入特定研发或实验区域时，必须根据安全规定穿戴相应的防护服或EGO装备。\n具体要求和配备由所在部门负责。", 87});
    m_faqData.append({"一般类", "公司有食堂吗？伙食怎么样？",
                      "公司设有员工食堂，由福利部精心管理。\n我们提供一日四餐（含夜宵），菜品丰富、营养均衡，并会定期征集员工意见进行改良。\n食堂是员工放松和交流的好去处。", 95});

    m_faqData.append({"一般类", "新人刚入职会有人带吗？",
                      "当然。我们有一套完善的“导师制”（Mentorship Program）。\n每一位新员工都会由一位经验丰富的老员工作为导师，进行一对一的指导和帮助，直到你能够完全独立胜任工作。", 87});
    m_faqData.append({"一般类", "完整的招聘流程是怎样的？",
                      "我们的标准流程是：\n1. 访客端AI初步评估 -> 2. 控制部初步面试 -> 3. 意向部门适应性测试 -> 4. 部长/队长终面 -> 5. 背景调查 -> 6. 发放录用通知 -> 7. 入职体检与签约。\n整个过程通常需要2-4周。", 95});

    m_faqData.append({"一般类", "AI初步评估会评估什么？",
                      "AI系统主要通过分析你的自我介绍文本，来评估你的核心性格特质（勇气、谨慎、自律、正义）与我们公司文化的匹配度，并为你初步推荐可能适合的部门。\n请尽可能真诚地展示你自己。", 87});
    m_faqData.append({"一般类", "面试时会问哪些问题？",
                      "问题因人而异，因部门而异。\n但通常会包括：你对公司的了解、你的过往经历、你如何处理压力、你对团队协作的看法，以及一些情景模拟题来考察你的应变能力和特质。", 95});

    m_faqData.append({"一般类", "我没有相关经验，可以申请吗？",
                      "可以！对于许多岗位，我们更看重你的潜力和核心特质，而非特定经验。\n公司拥有完善的培训体系（培训部Hod负责），能够将新人培养成合格的员工。\n你的学习能力、适应性和价值观对我们更重要。", 87});
    m_faqData.append({"一般类", "面试后多久能收到结果？",
                      "通常在每一轮面试后的3-5个工作日内，你会收到是否进入下一轮的通知。\n最终面试结果会在所有候选人面试结束后的一周内由控制部统一发出。", 95});

    m_faqData.append({"一般类", "如果面试失败了，还可以再申请吗？",
                      "可以。\n我们鼓励候选人不断提升自己。\n如果一次面试未通过，我们建议你间隔6个月后再尝试申请。\n你可以向AI助手询问（在不涉及保密信息的前提下）如何改进，以便下次准备得更充分。", 87});

    m_faqData.append({"部门相关", "公司里哪个部门最核心？",
                      "每个部门都是公司不可或缺的一部分，如同精密仪器的齿轮，共同维持着公司的运转。\n控制部是大脑，情报部是眼睛，培训部是摇篮，安保部是盾牌……\n缺少任何一个环节，公司都无法实现我们的终极目标。", 112});
    m_faqData.append({"部门相关", "我想申请安保部，需要什么条件？",
                      "安保部需要员工具备超凡的勇气、冷静的头脑和强烈的责任感。\n通常优先考虑有相关经验（如军事、警务、安保）或体育特长的人员。\n你需要通过严格的体能测试、心理压力测试和团队协作评估。\n最重要的是，你必须深刻理解“守护”的含义。", 95});

    m_faqData.append({"部门相关", "情报部的工作是不是就是看监控？",
                      "这是一个常见的误解。情报部的职责远不止于此。\n他们需要从海量的、看似无序的交互数据中提炼出有价值的模式和规律，建立预测模型，为其他部门的工作提供科学依据。\n这是一项需要极强逻辑分析能力和耐心的脑力工作。", 87});

    m_faqData.append({"部门相关", "福利部除了发福利还做什么？",
                      "福利部是公司的“暖心管家”。他们的工作至关重要，包括：\n维护员工心理健康、管理后勤保障（食堂、休息区）、组织文化活动、\n提供EGO装备支持、以及在员工遭遇创伤后提供恢复性帮助。\n他们是公司士气的维护者。", 95});

    m_faqData.append({"部门相关", "惩戒部听起来很可怕，他们是做什么的？",
                      "惩戒部是公司规则的坚定捍卫者和最终执行者。\n他们主要负责处理最极端的情况，比如最高危险等级的异常失控事件或内部严重违规行为。\n他们并非为了惩罚而存在，而是为了维护整个系统的秩序和安全，\n确保大多数人的努力不会因少数情况而付诸东流。", 87});

    m_faqData.append({"部门相关", "研发部（Binah）和构筑部（Keter）有什么区别？",
                      "研发部（R&D）专注于“技术”本身，负责解析异常、研发新装备和新技术。\n而构筑部（Architecture）则着眼于“宏观战略”，负责规划整个能源提取计划的蓝图和最终目标，\n确保所有部门的工作方向一致。\n简单说，研发部思考“如何实现”，而构筑部定义“要实现什么”。", 95});

    m_faqData.append({"部门相关", "如何知道哪个部门最适合我？",
                      "你可以在访客端的AI助手中进行自我介绍，\nAI系统会基于我们建立的模型，分析你的性格特质（勇气、谨慎、自律、正义），\n并为你推荐匹配度最高的部门。\n当然，最终的选择还需要通过该部门的面试和测试。", 87});

    m_faqData.append({"福利政策类", "公司的薪酬待遇有竞争力吗？",
                      "绝对有。青蓝公司为员工提供极具竞争力的薪酬 package，\n它由固定薪酬、浮动绩效和长期激励构成。\n除此之外，我们提供的特色福利（如EGO装备使用权、心理健康基金、能源补贴）\n其价值远超现金报酬，是其他公司无法提供的。", 95});

    m_faqData.append({"福利政策类", "五险一金是按什么标准缴纳的？",
                      "公司严格按照国家规定的最高缴费基数和比例为员工缴纳\n养老保险、医疗保险、失业保险、工伤保险、生育保险和住房公积金。", 87});

    m_faqData.append({"福利政策类", "年假有多少天？怎么计算？",
                      "员工入职满一年后，即可享受10天带薪年假。\n司龄每增加一年，年假增加1天，最长不超过20天。\n年假需要提前两周申请，以便部门协调工作安排。", 95});

    m_faqData.append({"福利政策类", "什么是“心理健康假”？怎么申请？",
                      "鉴于我们工作的特殊性，公司特别设立了带薪“心理健康假”。\n当你感到压力过大、需要调整时，可以向福利部提出申请。\n福利部的专业人员会进行评估，批准后即可休假。\n每年最多可享受15天。", 87});

    m_faqData.append({"福利政策类", "如果工作中受伤了怎么办？",
                      "公司有完善的工伤保险和额外的商业保险。\n一旦发生工伤，请立即报告你的直属上级和控制部。\n公司不仅会承担全部医疗费用，还会提供额外的特殊医疗补助和带薪疗养期，\n确保你得到最好的治疗和恢复。", 95});

    m_faqData.append({"福利政策类", "EGO装备是什么？员工可以使用吗？",
                      "EGO装备是公司基于核心技术研发的专用防护与工作装备。\n员工在完成相应培训并通过考核后，会根据其岗位等级和权限，\n被授予不同级别EGO装备的使用权。\n这既是工作的必要工具，也是一项特殊的员工福利。", 87});

    m_faqData.append({"福利政策类", "公司有班车或者交通补贴吗？",
                      "公司为员工提供多条线路的免费班车服务。\n如果你选择自行通勤，公司也会根据你的通勤距离提供额外的交通补贴。", 95});
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
