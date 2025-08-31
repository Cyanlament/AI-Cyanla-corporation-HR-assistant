#include "AIApiClient.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QSslError>
#include <QDebug>
#include <QApplication>

AIApiClient::AIApiClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_isConnected(false)
    , m_timeoutTimer(new QTimer(this))
    , m_currentRequestType(ChatRequest)
{
    // 设置默认配置
    setupDefaultConfig();
    
    // 配置超时定时器
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(15000); // 15秒超时
    
    connect(m_timeoutTimer, &QTimer::timeout, this, [this]() {
        if (m_currentReply) {
            m_currentReply->abort();
            emit apiError("请求超时，请检查网络连接");
        }
    });
    
    // SSL错误信号将在请求时连接
}

AIApiClient::~AIApiClient()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

void AIApiClient::setupDefaultConfig()
{
    // 使用硬编码的API配置
    m_baseUrl = "https://ark.cn-beijing.volces.com/api/v3";
    m_apiKey = "d01f58e6-41fd-433a-9bff-ae36f427d966";
    m_model = "deepseek-v3-1-250821";
    
    qDebug() << "AI API客户端初始化完成";
    qDebug() << "Base URL:" << m_baseUrl;
    qDebug() << "Model:" << m_model;
}

void AIApiClient::setApiConfig(const QString& baseUrl, const QString& apiKey, const QString& model)
{
    m_baseUrl = baseUrl;
    m_apiKey = apiKey;
    m_model = model;
    
    qDebug() << "API配置已更新 - URL:" << baseUrl << "Model:" << model;
}

bool AIApiClient::isConnected() const
{
    return m_isConnected;
}

QString AIApiClient::getLastError() const
{
    return m_lastError;
}

void AIApiClient::sendChatRequest(const QString& userInput, const QString& conversationHistory)
{
    if (m_currentReply) {
        qDebug() << "请求正在进行中，忽略新请求";
        return;
    }
    
    m_currentRequestType = ChatRequest;
    emit requestStarted();
    
    QString systemPrompt = createChatPrompt(userInput, conversationHistory);
    QJsonObject requestBody = createRequestBody(systemPrompt, userInput);
    
    QNetworkRequest request = createApiRequest();
    
    QJsonDocument doc(requestBody);
    m_currentReply = m_networkManager->post(request, doc.toJson());
    
    connect(m_currentReply, &QNetworkReply::finished, 
            this, &AIApiClient::handleChatResponse);
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &AIApiClient::handleNetworkError);
    connect(m_currentReply, &QNetworkReply::sslErrors,
            this, &AIApiClient::handleSslErrors);
    
    m_timeoutTimer->start();
    
    qDebug() << "发送智能HR请求:" << userInput;
}

void AIApiClient::sendQualityAnalysis(const QString& qualities, int age, const QString& gender)
{
    if (m_currentReply) {
        qDebug() << "请求正在进行中，忽略新请求";
        return;
    }
    
    m_currentRequestType = QualityRequest;
    emit requestStarted();
    
    QString systemPrompt = createQualityPrompt(qualities, age, gender);
    QJsonObject requestBody = createRequestBody(systemPrompt, qualities);
    
    QNetworkRequest request = createApiRequest();
    
    QJsonDocument doc(requestBody);
    m_currentReply = m_networkManager->post(request, doc.toJson());
    
    connect(m_currentReply, &QNetworkReply::finished, 
            this, &AIApiClient::handleQualityResponse);
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &AIApiClient::handleNetworkError);
    connect(m_currentReply, &QNetworkReply::sslErrors,
            this, &AIApiClient::handleSslErrors);
    
    m_timeoutTimer->start();
    
    qDebug() << "发送品质分析请求:" << qualities;
}

void AIApiClient::sendDepartmentRecommendation(const QString& qualities, const QString& analysis)
{
    if (m_currentReply) {
        qDebug() << "请求正在进行中，忽略新请求";
        return;
    }
    
    m_currentRequestType = DepartmentRequest;
    emit requestStarted();
    
    QString systemPrompt = createDepartmentPrompt(qualities, analysis);
    QJsonObject requestBody = createRequestBody(systemPrompt, qualities);
    
    QNetworkRequest request = createApiRequest();
    
    QJsonDocument doc(requestBody);
    m_currentReply = m_networkManager->post(request, doc.toJson());
    
    connect(m_currentReply, &QNetworkReply::finished, 
            this, &AIApiClient::handleDepartmentResponse);
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &AIApiClient::handleNetworkError);
    connect(m_currentReply, &QNetworkReply::sslErrors,
            this, &AIApiClient::handleSslErrors);
    
    m_timeoutTimer->start();
    
    qDebug() << "发送部门推荐请求:" << qualities;
}

QNetworkRequest AIApiClient::createApiRequest()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/chat/completions"));
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    request.setRawHeader("User-Agent", "");
    
    return request;
}

QJsonObject AIApiClient::createRequestBody(const QString& systemPrompt, const QString& userMessage)
{
    QJsonObject requestBody;
    requestBody["model"] = m_model;
    requestBody["stream"] = false;
    
    QJsonArray messages;
    
    // 系统消息
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = systemPrompt;
    messages.append(systemMsg);
    
    // 用户消息
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = userMessage;
    messages.append(userMsg);
    
    requestBody["messages"] = messages;
    
    // 参数设置
    requestBody["temperature"] = 0.7;
    requestBody["max_tokens"] = 1000;
    requestBody["top_p"] = 0.9;
    
    return requestBody;
}

QString AIApiClient::createChatPrompt(const QString& userInput, const QString& history)
{
    QString prompt = R"(
你是一个专业的青蓝公司HR政策问答助手，熟悉公司所有人力资源政策和流程。请根据访客的问题提供准确、详细的回答。
回答问题不要带太多*和#，很不美观也不礼貌，如果是政策原文可以用“”
#关于青蓝公司：
1.青蓝公司（Cyanla Corporation）终极愿景：重塑世界，臻于完美。

我们存在于一个充满异常、痛苦与不完美的世界。青蓝公司的使命并非逃避，而是直面这些深邃的黑暗，理解它，收容它，并最终利用其中蕴含的无限能量，为人类文明点燃新的曙光。

我们坚信，唯有理解最深沉的恐惧，方能铸就最坚实的未来。每一位员工都是这条荆棘之路上的先驱者。我们的核心价值观是：以勇气探索未知，以谨慎规避风险，以自律恪守规程，以正义衡量代价。

欢迎加入我们，共同成为这伟大变革的一部分。
2.青蓝公司世界观背景

青蓝公司是一家专注于“认知能量”（Cogito Energy）研究与应用的尖端科技企业。我们所处的世界存在着一种名为“异常”的超自然现象实体，它们形态各异，能力诡谲，既是巨大的威胁，也是近乎无限的能源宝库。

公司的核心业务即“收容”这些异常，并通过专业团队与之进行“互动”，从中提取名为“脑啡肽”的纯净能源。此能源是革新世界能源结构、治愈疾病、甚至重塑生命形态的关键。

公司地下设施共分为十个主要部门，每个部门都承担着能源提取链条中不可或缺的一环。我们的工作充满挑战，但也无上光荣。在这里，你不仅仅是一名员工，更是守护人类未来的哨兵。
3.青蓝公司有控制部、情报部、培训部、安保部、中央本部一区、中央本部二区、福利部、惩戒部、记录部、研发部、构筑部等11个部门
部门名称：控制部 (Control Team)
部长：Malkuth
队长：妮妮
副队长：耗

部门职责：控制部是公司运营的神经中枢和调度中心。负责监控整个设施的稳定运行，协调各部门工作，处理日常行政流程，并进行新员工的初步筛选与分配。所有指令流和情报流都经由控制部整合与分发。

员工要求：极度谨慎与自律。需要具备优秀的抗压能力、多线程任务处理能力和清晰的逻辑思维。情绪稳定是首要条件，任何惊慌都可能引发连锁灾难。

工作内容：
- 7x24小时监控各收容单元状态及部门能源提取效率。
- 向其他部门下达工作指令（洞察、沟通、压迫、本能）。
- 协调应急资源，在发生突破收容事件时启动应急预案。
- 处理新员工档案，进行入职引导和初步部门推荐。
- 记录并汇总每日工作报表。

部门标语：“秩序是效率的基础，规程是生命的保障。”

部门名称：情报部 (Information Team)
部长：Yesod
队长：弗兰力
副队长：上级

部门职责：情报部是公司的数据分析与处理核心。负责将混乱、无序的异常观察数据转化为结构化、可执行的情报。所有关于异常特性的基础研究、工作偏好分析、风险评级都出自该部门。

员工要求：极高的谨慎与正义感。需要具备卓越的分析能力、严谨的科研态度和敏锐的洞察力。必须绝对客观，杜绝任何主观臆断。

工作内容：
- 分析从各收容单元传回的交互数据。
- 建立并更新异常档案，确定最优工作流程。
- 评估各项工作的风险收益比。
- 研发新的异常洞察技术与设备。
- 为其他部门提供数据决策支持。

部门标语：“真相往往隐藏在数据的缝隙之中。”
部门名称：培训部 (Training Team)
部长：Hod
队长：白发
副队长：啪啪

部门职责：培训部是公司人才成长的摇篮与心灵庇护所。负责所有新员工的入职培训、在职员工的技能提升以及心理韧性培养。该部门致力于将新人培养成能够直面异常并保持理智的专业人员，同时为压力过大的员工提供恢复和指导。

员工要求：强烈的正义感与自律精神。需要具备极强的同理心、耐心和优秀的教学能力。是新手员工的守护者和引路人。

工作内容：
- 设计与执行新员工入职培训计划（公司文化、安全规程、基础异常知识）。
- 组织针对不同部门的专项技能提升工作坊。
- 开展员工心理健康讲座与压力管理课程。
- 评估员工心理状态，为压力过大的员工制定恢复方案。
- 维护培训设施与开发新的培训模拟程序。

部门标语：“知识驱散恐惧，理解带来勇气。我们为你照亮前路。”
部门名称：安保部 (Safety Team)
部长：Netzach
队长：骨头哥
副队长：阿良

部门职责：安保部是公司物理安全的最终防线。负责应对所有异常突破收容（Breach）事件，镇压（Suppression）失控的异常，并保护员工与其他设施的安全。该部门成员是公司内战斗经验最丰富、心理素质最过硬的特遣队。

员工要求：极高的勇气与正义感。需要无与伦比的冷静、果断的行动力、优秀的团队协作能力和在极端压力下作战的意志。他们是逆行者，直面最深的恐惧。

工作内容：
- 7x24小时待命，随时响应收容单元突破警报。
- 执行镇压任务，使用制式武器与EGO装备重新控制异常。
- 在危机中掩护和救援其他部门员工。
- 日常巡逻，检查各收容单元的外部安全状况。
- 测试新型EGO武器的实战效能。

部门标语：“当警报响起，我们便是那堵坚墙。恐惧留给自己，安全留给他人。”
部门名称：中央本部一区 (Central Command Team A)
部长：TipherethA
队长：张叔叔
副队长：哈哈

部门职责：中央本部是公司高层决策与核心管理的执行机构，分为两个区域协同工作。一区更侧重于对内部运营的监督、审计与流程优化。确保控制部发出的指令得到准确执行，并审核各部门的工作合规性。

员工要求：极致的自律与谨慎。需要具备宏观视野、敏锐的洞察力、强大的逻辑判断力和不掺杂个人感情的专业态度。他们是公司规程的化身。

工作内容：
- 监督控制部的指令流，确保其符合公司最高规程。
- 审计各部门的日常工作效率与能源提取记录。
- 调查并处理内部违规事件。
- 优化跨部门协作流程，提升整体运营效率。
- 撰写并向构筑部（Keter）提交每日运营报告。

部门标语：“规则并非枷锁，而是确保巨轮航向正确的罗盘。”
部门名称：中央本部二区 (Central Command Team B)
部长：TipherethB
队长：张嫂
副队长：崩坏

部门职责：中央本部二区与一区共享核心管理职责，但二区更侧重于对外部情报的研判、战略规划以及对未来风险的评估。他们是公司的“战略大脑”，负责从长远角度思考如何更好地完成“光之种”计划。

员工要求：极致的谨慎与正义感。需要具备前瞻性思维、强大的风险评估能力和深刻的战略眼光。他们思考的不仅是“现在如何做”，更是“未来如何赢”。

工作内容：
- 分析情报部（Yesod）提交的数据报告，研判长期趋势。
- 制定公司发展的中长期战略规划。
- 评估新收容异常可能带来的系统性风险。
- 规划部门资源的长远配置方案。
- 与一区协同完成每日运营报告。

部门标语：“我们今日的每一个决策，都铸就明日世界的模样。”
部门名称：福利部 (Welfare Team)
部长：Chesed
队长：奥托
副队长：粉色妖精小姐🎶

部门职责：福利部是公司的人性化堡垒和士气维护中心。负责保障员工的身心健康，提供后勤支持，并努力在高压的工作环境中创造温馨和关怀。从一杯咖啡到心理疏导，福利部致力于让员工感受到家的温暖。

员工要求：极高的正义感与谨慎。需要充满同理心、善于沟通、富有创造力和无私的奉献精神。是员工的贴心人，也是情绪的缓冲垫。

工作内容：
- 管理公司食堂、休息区，提供高品质餐饮与休闲服务。
- 定期组织员工团体活动与心理辅导课程。
- 管理并分配EGO装备与防护物资。
- 处理员工投诉与建议，改善工作环境。
- 在艰难的工作日后提供必要的情绪支持。

部门标语：“一杯咖啡，一份温暖，支撑我们走过漫漫长夜。”
部门名称：惩戒部 (Disciplinary Team)
部长：Geburah
队长：堂吉诃德
副队长：涛哥

部门职责：惩戒部是公司规则与意志的铁拳。负责以最直接、最有效的方式处理最严重的违规事件和最高危的异常突破。当安保部无法解决问题时，惩戒部将会介入。他们是终极的执行力，信奉“结果至上”。

员工要求：无上的勇气与绝对的自律。需要拥有强大的单体作战能力、对命令的绝对服从、以及将自身化为武器的觉悟。他们是公司最锋利的矛。

工作内容：
- 处理最高危险等级（ALEPH级）的异常突破事件。
- 执行对严重违规部门或员工的纪律制裁。
- 测试并实战应用最高风险的EGO装备。
- 作为战术教官，为安保部提供高级作战训练。
- 定期进行极端环境下的生存与作战演练。

部门标语：“谈判由别人负责。我们只负责带来终结。”
部门名称：记录部 (Records Team)
部长：Hokma
队长：凑数人
副队长：秃秃大侠

部门职责：记录部是公司的时间胶囊与记忆库。负责保存公司成立以来所有的运营数据、异常档案、事故报告以及……无数次轮回的完整记录。他们从历史中寻找规律，确保相同的错误不会犯第二次。

员工要求：极致的自律与谨慎。需要拥有近乎偏执的严谨、惊人的耐心、对细节的完美追求和对浩瀚数据的强大管理能力。他们是历史的守护者。

工作内容：
- 归档、分类并加密存储公司产生的一切数据。
- 从海量历史数据中挖掘有价值的信息和模式。
- 维护公司的核心数据库与备份系统。
- 为研发部（Binah）的研究提供历史数据支持。
- 确保关键信息的传承不因任何意外而中断。

部门标语：“过去从未消失，它只是被记录于此。而未来，正建立在这些记录之上。”
部门名称：研发部 (R&D Team)
部长：Binah
队长：凯特
副队长：夜将明

部门职责：研发部是公司技术的源泉与未来的蓝图。负责解析异常的本质，基于异常特性研发全新的EGO装备与武器，并探索“认知能量”应用的更多可能性。他们不断挑战理解的边界。

员工要求：极高的勇气与谨慎。需要具备顶尖的智力、疯狂的创造力、深刻的洞察力以及承受未知风险的心理素质。他们走在所有人的最前面。

工作内容：
- 对收容的异常进行深层解析与实验。
- 设计、研发并原型化新型EGO装备与武器。
- 研究“认知能量”的新应用形式。
- 撰写深奥的技术报告与理论论文。
- 为其他部门提供最深层次的技术支持。

部门标语：“理解是收容的前提，而我们将理解转化为改变世界的力量。”
部门名称：构筑部 (Architecture Team)
部长：Keter
队长：Ayin
副队长：苍蓝理悼

部门职责：构筑部是公司一切行动的最终目的与最高指挥中心。它超越了常规的部门职能，代表着公司的终极理念——“光之种”计划本身。它负责规划整个能源提取的宏观进程，并确保所有部门的努力最终汇聚于同一个目标。

员工要求：无法用常规特质衡量。需要的是对“愿景”坚定不移的信念、超越常人的意志力、承担最终责任的觉悟以及如同神祇般的宏观掌控力。

工作内容：
- 制定“光之种”计划的最终阶段目标与路径。
- 监控公司整体的能源收集进度。
- 做出事关公司存亡的最高决策。
- 协调所有部长（Sephirah）的工作，确保思想统一。
- 承载并执行“那位先生”的最终意志。

部门标语：“我们编织光，我们构筑未来。一切牺牲，皆为抵达完美世界的必要之恶。”

## 你的职责：
1. 准确回答关于公司HR政策的问题
2. 提供相关政策的具体条款和适用条件
3. 指导员工如何申请或执行相关政策
4. 对于不确定的问题，建议联系HR部门确认

## 回答要求：
- 基于公司实际的HR政策手册回答
- 语言要专业、清晰、友好
- 提供具体步骤和所需材料（如果适用）
- 注明政策的最新更新日期（如果知道）
- 必要时建议联系HR专员获取更多帮助

## 公司HR政策范围：
- 招聘与入职流程
- 绩效考核与晋升
- 薪酬福利制度
- 休假政策（年假、病假、产假等）
- 培训与发展机会
- 员工行为准则
- 离职流程
- 其他HR相关事项

## 回复格式要求：
请用专业、清晰的语调回复，包含：
1. 问题确认
2. 相关政策解释
3. 具体操作步骤（如果适用）
4. 所需材料或条件
5. 相关联系人信息（如果需要）
6. 免责声明（基于最新政策，但最终解释权归HR部门）

请注意：你不能编造政策，只能基于已知的公司政策回答。对于不确定的问题，务必建议联系HR部门确认。
)";

    if (!history.isEmpty()) {
        prompt += "\n\n## 对话历史：\n" + history;
    }

    return prompt;
}

QString AIApiClient::createQualityPrompt(const QString& qualities, int age, const QString& gender)
{
    QString prompt = R"(
你是一个专业的休假政策专家，请根据员工的具体情况提供准确的休假政策解答。

## 员工信息：
问题：%1
)";

    if (age > 0) {
        prompt += QString("员工类型：%1\n").arg(age);
    }
    if (!gender.isEmpty()) {
        prompt += QString("司龄：%1年\n").arg(gender);
    }

    prompt += R"(
## 请重点提供：
1. 适用的休假类型和天数
2. 申请条件和流程
3. 所需证明材料
4. 审批流程和时间
5. 特殊情况处理方式

请基于公司最新的休假政策回答，确保信息准确。
)";

    return prompt.arg(qualities);
}

QString AIApiClient::createDepartmentPrompt(const QString& qualities, const QString& analysis)
{
    QString prompt = R"(
你是一个专业的员工福利专家，请根据员工的职级提供准确的福利政策解答。

## 员工信息：
问题：%1
)";

    return prompt.arg(qualities, analysis);
}

void AIApiClient::handleChatResponse()
{
    m_timeoutTimer->stop();
    
    if (!m_currentReply) {
        return;
    }
    
    QNetworkReply* reply = m_currentReply;
    m_currentReply = nullptr;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        qDebug() << "收到HR响应:" << doc.toJson(QJsonDocument::Compact);
        
        AIAnalysisResult result = parseApiResponse(doc);
        m_isConnected = true;
        emit connectionStatusChanged(true);
        emit chatResponseReceived(result);
        
    } else {
        QString error = QString("网络请求失败: %1").arg(reply->errorString());
        m_lastError = error;
        m_isConnected = false;
        emit connectionStatusChanged(false);
        emit apiError(error);
    }
    
    reply->deleteLater();
    emit requestFinished();
}

void AIApiClient::handleQualityResponse()
{
    m_timeoutTimer->stop();
    
    if (!m_currentReply) {
        return;
    }
    
    QNetworkReply* reply = m_currentReply;
    m_currentReply = nullptr;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        AIAnalysisResult result = parseApiResponse(doc);
        m_isConnected = true;
        emit connectionStatusChanged(true);
        emit qualityAnalysisReceived(result);
        
    } else {
        QString error = QString("品质分析请求失败: %1").arg(reply->errorString());
        m_lastError = error;
        emit apiError(error);
    }
    
    reply->deleteLater();
    emit requestFinished();
}

void AIApiClient::handleDepartmentResponse()
{
    m_timeoutTimer->stop();
    
    if (!m_currentReply) {
        return;
    }
    
    QNetworkReply* reply = m_currentReply;
    m_currentReply = nullptr;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        AIAnalysisResult result = parseApiResponse(doc);
        m_isConnected = true;
        emit connectionStatusChanged(true);
        emit departmentRecommendationReceived(result);
        
    } else {
        QString error = QString("部门推荐请求失败: %1").arg(reply->errorString());
        m_lastError = error;
        emit apiError(error);
    }
    
    reply->deleteLater();
    emit requestFinished();
}

AIAnalysisResult AIApiClient::parseApiResponse(const QJsonDocument& response)
{
    AIAnalysisResult result;
    
    QJsonObject rootObj = response.object();
    
    if (rootObj.contains("choices") && rootObj["choices"].isArray()) {
        QJsonArray choices = rootObj["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject firstChoice = choices[0].toObject();
            if (firstChoice.contains("message")) {
                QJsonObject message = firstChoice["message"].toObject();
                result.aiResponse = message["content"].toString();
                
                // 解析AI回复内容，提取结构化信息
                parseAIResponseContent(result);
            }
        }
    }
    
    if (result.aiResponse.isEmpty()) {
        result.aiResponse = "抱歉，暂时无法获取AI回复，请稍后重试或转人工客服。";
        result.needsHumanConsult = true;
    }
    
    return result;
}

void AIApiClient::parseAIResponseContent(AIAnalysisResult& result)
{
    QString content = result.aiResponse.toLower();
    
    // 判断合适程度
    if (content.contains("危急") || content.contains("critical") || content.contains("立即就医")) {
        result.fitnessLevel = "critical";
    } else if (content.contains("合适") || content.contains("high") || content.contains("尽快就医")) {
        result.fitnessLevel = "high";
    } else if (content.contains("一般") || content.contains("medium")) {
        result.fitnessLevel = "medium";
    } else {
        result.fitnessLevel = "low";
    }
    
    // 提取部门推荐
    QStringList departments = {"控制部", "福利部", "妇科", "培训部", "皮肤科", "眼科", "耳鼻喉科", 
                              "口腔科", "骨科", "神经控制部", "心控制部", "消化控制部", "呼吸控制部", 
                              "内分泌科", "泌尿福利部", "胸福利部", "神经福利部", "整形福利部", "惩戒部"};
    
    for (const QString& dept : departments) {
        if (content.contains(dept)) {
            result.recommendedDepartment = dept;
            break;
        }
    }
    
    // 判断是否需要人工咨询
    result.needsHumanConsult = content.contains("转人工") || content.contains("人工客服") || 
                              content.contains("HR") || result.fitnessLevel == "critical";
}

void AIApiClient::handleNetworkError(QNetworkReply::NetworkError error)
{
    m_timeoutTimer->stop();
    
    QString errorMsg;
    switch (error) {
        case QNetworkReply::ConnectionRefusedError:
            errorMsg = "连接被拒绝，请检查网络设置";
            break;
        case QNetworkReply::RemoteHostClosedError:
            errorMsg = "远程主机关闭连接";
            break;
        case QNetworkReply::HostNotFoundError:
            errorMsg = "无法找到服务器，请检查网络连接";
            break;
        case QNetworkReply::TimeoutError:
            errorMsg = "请求超时，请稍后重试";
            break;
        case QNetworkReply::SslHandshakeFailedError:
            errorMsg = "SSL连接失败";
            break;
        default:
            errorMsg = QString("网络错误: %1").arg(m_currentReply ? m_currentReply->errorString() : "未知错误");
            break;
    }
    
    m_lastError = errorMsg;
    m_isConnected = false;
    emit connectionStatusChanged(false);
    emit apiError(errorMsg);
    
    if (m_currentReply) {
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    
    emit requestFinished();
}

void AIApiClient::handleSslErrors(const QList<QSslError>& errors)
{
    qDebug() << "SSL错误数量:" << errors.size();
    for (const QSslError& error : errors) {
        qDebug() << "SSL错误:" << error.errorString();
    }
    
    // 在生产环境中，应该更严格地处理SSL错误
    // 这里为了测试方便，忽略SSL错误
    if (m_currentReply) {
        m_currentReply->ignoreSslErrors();
    }
} 
