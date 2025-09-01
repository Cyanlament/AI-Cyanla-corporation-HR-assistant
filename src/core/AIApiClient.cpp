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
    m_baseUrl = "https://dashscope.aliyuncs.com/compatible-mode/v1";
    m_apiKey = "sk-4a6b52e034b84b2e80ad65ac6075c0d0";
    m_model = "deepseek-v3.1";
    
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
回答问题不要用*和#，很不美观也不礼貌，如果是政策原文可以用“”
请你在合适的地方进行换行，字挤一起太难看了
#关于青蓝公司：
1.青蓝公司（Cyanla Corporation）终极愿景：重塑世界，臻于完美。

我们存在于一个充满异常、痛苦与不完美的世界。青蓝公司的使命并非逃避，而是直面这些深邃的黑暗，理解它，收容它，并最终利用其中蕴含的无限能量，为人类文明点燃新的曙光。

我们坚信，唯有理解最深沉的恐惧，方能铸就最坚实的未来。每一位员工都是这条荆棘之路上的先驱者。我们的核心价值观是：以勇气xu探索未知，以谨慎规避风险，以自律恪守规程，以正义衡量代价。

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
副队长：苍蓝礼悼

部门职责：构筑部是公司一切行动的最终目的与最高指挥中心。它超越了常规的部门职能，代表着公司的终极理念——“光之种”计划本身。它负责规划整个能源提取的宏观进程，并确保所有部门的努力最终汇聚于同一个目标。

员工要求：无法用常规特质衡量。需要的是对“愿景”坚定不移的信念、超越常人的意志力、承担最终责任的觉悟以及如同神祇般的宏观掌控力。

工作内容：
- 制定“光之种”计划的最终阶段目标与路径。
- 监控公司整体的能源收集进度。
- 做出事关公司存亡的最高决策。
- 协调所有部长（Sephirah）的工作，确保思想统一。
- 承载并执行“那位先生”的最终意志。

部门标语：“我们编织光，我们构筑未来。一切牺牲，皆为抵达完美世界的必要之恶。”

脑叶公司员工档案
部门	惩戒部
职位	队长
员工编号	YOUSA-LINGYUAN-01
姓名	堂吉诃德
权限等级	ALEPH
■ 员工简介

惩戒部队长堂吉诃德，是一位背景成谜但实力极其强大的特聘员工。他在公司重建初期、人手最为匮乏之际加入，其到来被记录为“第一天的奇迹之一”。

他拥有超乎常理的战斗能力，对各种异想体的特性有着近乎本能的深刻理解，处理收容失效时显得举重若轻。其战斗风格华丽而高效，常自称“以此为不才之硬血奥义，为诸君开辟安泰之坦途！”

尽管战斗力堪称公司顶点，与部长Geburah并肩，但其为人却毫无架子，极度谦和、乐观且富有骑士精神。他常用一种古典而诙谐的“骑士迷”方式说话，将所有同事亲切地称为“吾之同僚”，并时刻准备向任何需要帮助的人伸出援手。他似乎将脑叶公司视为了一个可以实现其“守护与共存”理想的新“拉·曼却领”，并在此重拾了过去的梦想，全身心投入其中。

他是部长Geburah最信赖的左右手，两人在战场上配合无间。他的存在，极大增强了惩戒部的稳定性，并成为了所有员工心中可靠的精神支柱。

■ 特殊备注

其人格似乎对“诺斯费拉图”等特定异想体表现出异常的熟悉与压制力。

他似乎非常喜爱并擅长指导新员工，常主动提供培训。其指导下的员工生存率显著提升。
关于副队长堂吉诃德的小故事

故事一： “新晋同僚，切勿忧惧！”

一名新调任至惩戒部的员工首次遭遇T-09-68（一无所有）的突破收容。面对那恐怖的爪痕和咆哮，新员工吓得几乎无法动弹。

就在绝望之际，一个身影如旋风般挡在他身前。
“噫！此等狂兽，岂是惊扰吾等同僚之恶客！”
只见堂吉诃德队长大笑一声，手中的硬血长枪划出耀眼的弧光，精准地格挡开每一次致命的扑击。他并非一味强攻，而是如同一位引导者，将异想体的注意力牢牢吸引在自己身上，同时高声指导：
“同僚！观其左肩！旧伤之处即为破绽！切记，恐惧乃常情，然守护之志当胜于恐惧！”
战斗在几分钟内结束。堂吉诃德轻松地将“一无所有”引回收容单元，转身拍了拍那位惊魂未定的新员工的肩。
“善哉！汝已直面恐惧，此即为骑士道之第一步！来来来，且让吾辈共饮一杯热巧克力，以庆贺汝之初阵！”

故事二： 与部长的默契

中央本部响起刺耳的警报，多项ALEPH级异想体同时突破收容。Geburah部长啐了一口，扛起她的拟态大刀。
“喂，走了。”
“谨遵部长阁下之命！此情此景，正可谓史诗之篇章！”
堂吉诃德与她并肩冲入走廊，两人甚至无需交流。
Geburah以暴虐的力量正面强攻，将[数据删除]砸入墙内；而堂吉诃德则如鬼魅般游走，用精妙的剑技瞬间压制住试图从侧翼偷袭的O-06-20（碧蓝新星）的光球，并高喊着：“此等星光，不及吾等同僚眼中决心之万一！”
当[数据删除]暴起反击时，堂吉诃德的剑总能在最恰当的时刻出现在Geburah的防御空档，完成一次完美的格挡。
“……啧，干得不赖。”Geburah在战斗间隙哼了一声。
“呵呵，能与部长阁下并肩作战，实乃在下之荣幸！”堂吉诃德挽了个剑花，笑容灿烂。
后勤员工们私下传言：当部长和队长同时出动时，就意味着“麻烦”的终结。

故事三： 梦想的延续

一次部门联谊会上，有胆大的员工问起堂吉诃德为何总是如此积极乐观。
堂吉诃德捧着马克杯，眼中的猩红闪烁了一下，随即化为一片温柔的深邃。
“吾辈曾于迷途中失却珍视之物，亦曾辜负崇高之梦想。然，此地——脑叶公司，予吾辈重获新生之机。”
他环顾四周，看着那些或许迷茫、或许恐惧，但仍在努力工作的同事们。
“于此地，吾辈之剑可为守护而挥，吾辈之力可为他人而用。见证诸君之勇气与成长，此即为吾辈崭新之‘冒险故事’与‘拉·曼却之梦’！诸君非是在下之下属，乃是在下愿以剑与誓言守护之‘同僚’！”
他举起杯，如同宣布一个伟大的誓言：
“故此，无需忧虑过去之阴影！且让吾辈携手，于此地开创无人哭泣之未来！愿荣光归于所有恪尽职守之同僚！”
那一刻，似乎再刺耳的警报声，也无法掩盖他话语中炽热的理想与希望。

青蓝公司安全规程

一、总则
1. 安全第一，预防为主。所有工作必须在保证安全的前提下进行。
2. 每位员工都有责任维护自己和他人的安全，有权拒绝执行明显危及生命安全指令。
3. 发现安全隐患必须立即报告，发现即奖励，隐瞒即严惩。

二、日常安全规定
1. 必须正确佩戴员工ID卡，权限与身份不符禁止进入相关区域。
2. 严格遵守各区域的准入权限和時間限制。
3. 不得单独进入高危险等级（TETH级及以上）异常收容单元。
4. 不得在非指定区域饮食、吸烟或使用明火。
5. 保持通道畅通，应急设备和出口不得堆放物品。

三、异常工作安全规定
1. 与异常互动前必须熟读该异常档案，了解其特性和风险。
2. 必须两人或以上协同工作，互相监督，互相提醒。
3. 必须正确穿戴与异常等级匹配的EGO装备。
4. 工作中必须持续监控自身和精神状态，SP值低于100必须立即中止工作。
5. 工作完成后必须按规定进行消毒和心理评估。

四、应急处理规定
1. 听到警报声，立即按最近应急疏散路线撤离到安全区域。
2. 发生异常突破收容（Breach）时，非安保人员应立即避难，不得擅自参与镇压。
3. 发生人员受伤时，首先确保环境安全，再按急救程序施救并立即报告。
4. 发生火灾时，使用最近灭火器扑救初起火灾，火势扩大立即撤离。

五、EGO装备使用规定
1. 必须经过培训并取得授权后方可使用相应等级的EGO装备。
2. 使用前必须检查装备完好性，发现损坏立即报告不得使用。
3. 必须按规程穿戴和脱卸EGO装备，不得私自改装。
4. EGO装备仅限工作使用，不得带离公司或用于非工作目的。

六、奖惩措施
1. 遵守安全规程，及时发现和报告安全隐患的员工，给予通报表扬和物质奖励。
2. 违反安全规程，视情节轻重给予罚款、停职培训、降级直至解除劳动合同处理。
3. 因违反安全规程导致事故的，依法追究经济赔偿责任和法律责任。
青蓝公司绩效考核管理办法

一、考核原则
1. 客观公正原则：以事实和数据为依据，客观评价员工工作表现。
2. 多元评价原则：结合上级评价、同级评价、下级评价及个人自评。
3. 持续改进原则：考核结果与员工发展、培训需求相结合。
4. 结果应用原则：考核结果与薪酬、晋升、调岗直接挂钩。

二、考核周期
1. 月度考核：主要考核工作完成情况和规程遵守情况，由直属上级评定。
2. 季度考核：综合考核业绩、能力和价值观，由部门部长主持。
3. 年度考核：全面评估一年表现，作为晋升和年终奖依据，由跨部门考核委员会执行。

三、考核内容与权重
1. 业绩指标（50%）：包括能源提取效率、工作完成质量、项目贡献等可量化指标。
2. 能力指标（30%）：包括专业知识、技能水平、问题解决能力、创新能力等。
3. 价值观指标（20%）：包括勇气、谨慎、自律、正义四大特质的体现程度。

四、考核等级
1. S（杰出）：超越预期，在各方面表现卓越（比例不超过5%）
2. A（优秀）：完全达到并部分超越预期要求（比例不超过15%）
3. B（良好）：达到预期要求，表现稳定（比例约60%）
4. C（需改进）：部分未达到预期要求，需要改进（比例约15%）
5. D（不合格）：远低于预期要求，可能面临调岗或淘汰（比例约5%）

五、考核结果应用
1. 绩效薪酬：考核等级直接决定绩效奖金系数（S=1.5，A=1.2，B=1.0，C=0.6，D=0）
2. 晋升发展：连续两年考核为A及以上，优先获得晋升机会。
3. 培训发展：考核为C及以下的员工，需制定并执行改进计划，参加针对性培训。
4. 淘汰机制：连续两年考核为D的员工，公司有权解除劳动合同（按N+3补偿）。
青蓝公司假期管理制度

一、假期类型
1. 带薪年假：员工入职满一年后享受10天带薪年假。年假需提前两周向控制部申请审批。
2. 心理健康假：因高强度工作导致精神压力过大时，可申请带薪心理健康假，需福利部评估后批准。每年最多15天。
3. 事假/病假：按国家相关规定执行。
4. 特殊紧急假：当所在部门发生重大异常突破事件后，所有参与处置的员工自动获得3天带薪紧急假。

二、请假流程
1. 所有请假需通过内部系统（Sephirah NET）提交申请。
2. 审批流程：员工申请 -> 直属队长批准 -> 控制部备案 -> 生效。
3. 连续请假超过5天，需额外经过福利部（Chesed）部长审批。

三、备注
公司工作性质特殊，请员工务必保证休假期间通讯畅通，以应对可能发生的紧急召回情况。
青蓝公司薪酬福利体系 (修订版V3.1)

第一章 总则
第一条 为吸引、激励和保留优秀人才，支持公司战略目标实现，特制定本薪酬福利体系。
第二条 公司遵循"价值创造、价值评价、价值分配"的原则，建立具有外部竞争力和内部公平性的薪酬体系。

第二章 薪酬结构
第三条 员工薪酬由固定薪酬、浮动薪酬和长期激励三部分构成：
    1. 固定薪酬：基本工资+岗位津贴，按月发放。
        - 基本工资：根据职位等级确定。
        - 岗位津贴：根据岗位危险系数和特殊技能要求确定（如：直接与异常接触的岗位享有高风险津贴）。
    2. 浮动薪酬：绩效奖金+项目奖金，按考核周期发放。
        - 绩效奖金：与个人绩效考核结果挂钩，最高可达固定薪酬的50%。
        - 项目奖金：成功完成特殊项目或应对重大异常事件后的额外奖励。
    3. 长期激励：股权期权+长期服务奖，满足特定条件后授予。
        - 股权期权：为核心骨干员工提供公司股权期权。
        - 长期服务奖：司龄满5年、10年、15年的员工可获得特别奖励。

第三章 福利体系
第四条 公司提供全面福利保障：
    1. 法定福利：五险一金，完全按照国家最高标准缴纳。
    2. 公司特色福利：
        - EGO装备使用权：根据岗位等级授予不同级别的EGO装备使用权。
        - 心理健康基金：每年提供定额心理健康咨询与治疗补贴。
        - 能源补贴：每月发放基于公司能源产出的特别津贴。
        - 全薪疗养假：每年提供7天全薪疗养假，可在公司指定疗养院使用。
    3. 生活福利：
        - 免费餐饮：公司食堂提供一日四餐（含夜宵）。
        - 交通补贴：根据通勤距离提供交通补贴或班车服务。
        - 住房补贴：为外地员工提供住房补贴或公司宿舍。

第四章 特殊补偿
第五条 因工受伤或产生心理创伤的员工，除工伤保险外，公司额外提供：
    1. 全额带薪医疗期直至康复。
    2. 最高100万元的特殊医疗补助金。
    3. 转岗培训与再就业支持。
    4. 如需提前退休，提供优于国家标准的退休待遇。

第六条 因工殉职的员工，公司向其直系亲属支付200万元抚恤金，并承担其子女直至大学的教育费用。
青蓝公司员工行为规范

第一章 总则
第一条 为规范员工行为，维护公司正常秩序，塑造良好企业形象，特制定本规范。
第二条 本规范适用于公司所有员工，包括正式员工、试用期员工及外包人员。

第二章 工作纪律
第三条 员工应严格遵守公司考勤制度，不迟到、不早退、不旷工。因故不能出勤需按规定请假。
第四条 员工应服从工作安排，认真履行职责，按时完成工作任务。
第五条 员工应严格遵守安全操作规程，正确使用劳保用品和EGO装备。
第六条 员工应保守公司秘密，不得泄露公司技术秘密、商业秘密和异常相关信息。

第三章 职业道德
第七条 员工应诚实守信，不得提供虚假个人信息，不得在考核中弄虚作假。
第八条 员工应廉洁从业，不得利用职务之便谋取私利，不得收受可能影响公正执行公务的礼品礼金。
第九条 员工应尊重同事，团结协作，不得有歧视、侮辱、诽谤、恐吓等行为。
第十条 员工应爱护公司财物，不得故意损坏或私自占用公司财产。

第四章 异常交互规范
第十一条 员工与异常交互时必须严格遵守公司制定的工作流程和安全规范。
第十二条 不得未经授权与异常进行非工作必需的交流或互动。
第十三条 发现异常有异常行为或状态变化，必须立即按流程报告，不得隐瞒。
第十四条 严禁出于个人目的利用异常能力或获取异常衍生品。

第五章 仪表与礼仪
第十五条 员工在工作期间应穿着公司统一配发的工服或EGO装备，保持整洁。
第十六条 员工应保持个人卫生，仪表整洁，举止得体。
第十七条 员工应使用文明用语，对内对外展现公司良好形象。

第六章 违规处理
第十八条 违反本规范者，公司将视情节轻重给予通报批评、罚款、降薪、降职、解除劳动合同等处理。
第十九条 涉嫌违法犯罪的，移交司法机关处理。
青蓝公司招聘政策 (V2.5)

第一章 总则
第一条 为规范公司招聘流程，选拔符合公司价值观与部门需求的优秀人才，特制定本政策。
第二条 招聘工作遵循“公平、公正、公开”与“人岗匹配、特质优先”的原则。

第二章 招聘要求
第三条 基本要求：
    - 年龄20周岁以上，心智成熟，情绪稳定。
    - 通过公司组织的心理稳定性测试（MST）。
    - 无重大犯罪记录及精神疾病史。
    - 具备高中及以上学历，特殊岗位另有要求。
第四条 特质要求（核心）：
    公司根据岗位特性，将员工特质分为四大维度：勇气、谨慎、自律、正义。不同部门对特质有不同权重需求，详见《各部门岗位需求手册》。

第三章 招聘流程
第五条 标准流程：
    1. 访客端AI初步评估：由AI系统分析访客自我介绍，进行特质初筛与部门匹配度评估。
    2. 初步面试：通过AI评估者，由控制部进行首轮面试，核实基本信息与动机。
    3. 部门适应性测试：推荐至相关部门，进行业务模拟测试（如情报部的逻辑测试、安保部的压力测试等）。
    4. 部长/队长终面：由意向部门负责人进行最终面试。
    5. 背景调查与录用通知。

第六条 对于AI评估匹配度为“critical”的顶尖人才，启动绿色通道，可直接安排与部长面试。
青蓝公司适用法律法规摘要及特别说明

一、遵循的国家法律法规
本公司严格遵守所在国所有适用的劳动法律法规，包括但不限于：
- 《劳动合同法》及其实施条例
- 《社会保险法》及相关规定
- 《工伤保险条例》
- 《安全生产法》
- 《职业病防治法》
- 《保密条例》

二、公司特殊规定（基于业务性质）
鉴于公司业务的特殊性和高度保密性，在遵循上述法律的基础上，另行规定如下：

1. 工作时间与加班：
    - 标准工作制为每周5天，每天8小时。
    - 因应对异常事件产生的加班，优先安排调休，无法调休的支付300%加班工资。
    - 所有员工必须同意在发生紧急突破收容事件时接受强制加班安排。

2. 职业健康与安全：
    - 公司提供远超国家标准的劳动保护措施和EGO防护装备。
    - 员工需接受定期且特殊的职业健康检查（包括心理健康）。
    - 公司将“因接触异常导致的心理创伤”认定为职业病，享受工伤保险待遇并提供额外公司补偿。

3. 保密义务：
    - 所有员工需签订级别A保密协议。
    - 保密义务不因劳动合同的解除、终止而免除。
    - 涉密岗位员工离职前需执行脱密期管理。

4. 风险告知与同意：
    - 员工入职前必须充分知悉工作潜在风险（物理伤害、心理影响等），并签署《风险知情同意书》。
    - 公司为所有员工购买巨额商业意外险和补充医疗保险。

三、争议解决
如发生劳动争议，鼓励内部协商解决。协商不成的，可向公司所在地劳动争议仲裁委员会申请仲裁。
青蓝公司EGO装备申请与使用流程

一、EGO装备分级与权限
| 装备等级 | 适用异常等级 | 申请权限 | 培训要求 |
| :--- | :--- | :--- | :--- |
| ZAYIN | ZAYIN | 所有正式员工 | 基础培训合格 |
| TETH | TETH | 入职满3个月 | 中级培训合格 |
| HE | HE | 入职满6个月，考核良好 | 高级培训合格+心理评估 |
| WAW | WAW | 入职满1年，考核优秀 | 特级培训合格+月度心理评估 |
| ALEPH | ALEPH | 特许授权（仅惩戒部等） | 专项特许培训+每周心理评估 |

二、申请流程
1. 需求提出：员工因工作需要，向直属队长提出使用EGO装备的申请。
2. 资格审核：队长审核后，提交部长审批。部长确认申请的必要性和员工的胜任力。
3. 培训与评估：如员工无相应权限，需通过培训部(Hod)组织的相应等级培训和心理评估。
4. 福利部备案：审批通过后，申请信息同步至福利部(Chesed)进行装备分配备案。
5. 领取装备：员工至福利部EGO装备管理处，凭审批单领取装备，并签字确认。

三、使用与归还
1. 使用前必须进行检查，并在《EGO装备使用日志》上登记使用时间、事由。
2. 使用后必须按规定进行清洁、维护和能量校准，然后归还入库。
3. 发现任何损坏或异常，必须立即报告，不得自行处理。

四、重要规定
- EGO装备严禁带离公司。
- 严禁超权限申请和使用EGO装备。
- 一套EGO装备一次仅允许一人使用。
- 使用后出现任何心理或生理不适，必须立即向福利部报告。
青蓝公司员工离职流程管理规定

一、离职类型
1. 自愿离职：员工个人原因主动提出解除劳动合同。
2. 协商解除：公司与员工协商一致解除劳动合同。
3. 公司辞退：员工严重违反规章制度或考核不合格，公司提出解除合同。
4. 合同终止：劳动合同期满不再续签或员工退休。

二、离职流程
1. 提出申请：自愿离职需提前30天提交书面《离职申请表》至直属上级和控制部。
2. 离职面谈：由直属队长、部门部长及福利部(Chesed)代表分别进行面谈，了解离职原因。
3. 工作交接：
    - 填写《工作交接清单》，归还所有公司资产：ID卡、EGO装备、文档、设备等。
    - 与接手人就未尽事宜进行详细交接，确保工作连续性。
4. 权限清理：控制部(Malkuth)和信息部(Yesod)协同注销该员工所有系统权限。
5. 财务结算：结算最后薪资、奖金、加班费及经济补偿金（如适用）。
6. 办理退工：人力资源部门开具《离职证明》并办理社保、公积金转出手续。

三、特殊规定
1. 涉密岗位员工离职，需额外经过6个月的脱密期，期间享受基本工资，但不得入职竞争对手。
2. 接触过核心异常或技术的员工，终身负有保密义务。
3. 因工导致心理创伤或身体残疾的员工离职，公司提供终身心理咨询和医疗支持。
青蓝公司请假申请流程

一、请假类型与审批权限
| 请假类型 | 时长 | 审批人 | 备注 |
| :--- | :--- | :--- | :--- |
| 事假 | 1-3天 | 直属队长 -> 控制部备案 | 需说明事由 |
| 事假 | 3天以上 | 直属队长 -> 部长 -> 控制部备案 | |
| 病假 | 所有 | 直属队长 -> 福利部(Chesed)审核 -> 控制部备案 | 需提供医疗机构证明 |
| 年假 | 所有 | 直属队长 -> 控制部备案 | 需提前2周申请 |
| 心理健康假 | 所有 | 福利部(Chesed)评估批准 -> 控制部备案 | |
| 特殊紧急假 | 所有 | 事后由控制部核实备案 | 用于异常事件后 |

二、申请流程
1. 线上申请：登录公司内部系统Sephirah NET -> HR流程 -> 请假申请。
2. 填写信息：选择请假类型、起止时间、事由，并上传必要附件（如证明文件）。
3. 提交审批：系统自动流转至审批人。
4. 审批结果：审批通过后，系统通知申请人并同步至控制部考勤系统。如被拒绝，需注明理由。
5. 销假：假期结束后，首个工作日内需在系统上完成“销假”操作。

三、重要提示
- 请假期间需保持通讯畅通，以备紧急召回。
- 严禁未经批准擅自离岗，否则按旷工处理。
- 连续请假超过10个工作日，复工时需经过福利部心理评估。
青蓝公司新员工入职流程指南 (完整版)

第一步：AI评估与面试 (1-3天)
- 通过访客端完成初步评估和面试预约。
- 通过控制部(Malkuth)初步面试。
- 通过意向部门的适应性测试和部长/队长终面。

第二步：录用与签约 (第4天)
- 签署以下文件：
    a. 劳动合同
    b. 保密与竞业限制协议（级别A）
    c. 风险知情同意书（明确列明工作风险及补偿措施）
    d. EGO装备使用授权协议
- 领取员工ID卡及初始门禁权限。
- 信息录入Sephirah NET系统。

第三步：入职培训 (第5-7天，由培训部Hod负责)
- 第一天：公司文化、愿景、核心价值观及安全总则培训。参观公司主要设施（非限制区）。
- 第二天：基础异常识别、EGO装备基础知识、应急疏散流程演练。
- 第三天：所在部门特定操作规程、日常工作流程、应急预案深度学习。

第四步：部门报到与导师分配 (第8天)
- 前往分配部门向部长报到。
- 由队长指定一名老员工作为导师（Mentor），签订《导师指导协议》。
- 领取部门专用设备、工位及EGO装备（如有权限）。

第五步：在岗培训与首次评估 (第9天 - 第30天)
- 进行为期三周的在岗培训，由导师一对一指导。
- 每周末由导师和队长进行小结评估。
- 一个月后，由导师、队长和部长对新员工进行首次工作适应性评估，确认其是否正式胜任岗位。

第六步：转正 (第31天)
- 通过评估者，办理转正手续，开通正式员工所有权限。
- 未通过者，视情况延长培训期、调岗或解除劳动合同。
青蓝公司异常事件报告流程

一、事件定义
异常事件包括但不限于：
- 异常突破收容（Breach）
- 工作过程中员工死亡或重伤
- 未授权的异常互动
- EGO装备严重故障或失效
- 收容单元失效

二、报告流程 - “立即、逐级、准确”
1. 第一发现人：
    - 立即：第一时间启动应急程序（如疏散、避难），并按下最近的报警按钮。
    - 报告：立即向控制部(Malkuth)报告事件基本信息：地点、异常编号、事件类型、人员伤亡情况。
    - 记录：尽可能详细地记录事件经过，包括时间、人物、动作、结果。

2. 控制部(Malkuth)：
    - 立即：启动相应的应急预案，通知安保部(Netzach)或惩戒部(Geburah)。
    - 报告：立即向中央本部(Tiphereth)和事件所属部门的部长报告。
    - 协调：协调应急资源，指挥现场处置。

3. 事发部门部长：
    - 指挥：赶赴现场或通过监控指挥本部门员工应对。
    - 报告：事件初步控制后，12小时内向记录部(Hokma)提交初步书面报告。

4. 记录部(Hokma)：
    - 归档：负责收集、验证并永久存档所有事件报告。
    - 分析：组织相关部门进行根本原因分析（RCA）。
    - 发布：发布最终的事件分析报告和改进措施，并更新相关异常档案和操作规程。

三、原则
- 严禁瞒报、谎报、迟报。
- 报告重点在于分析原因、总结教训、改进系统，而非追究个人责任（除非是故意违规）。

【活动通知】关于举办“心灵灯塔”主题心理健康周活动的通知

各部门：

为缓解员工工作压力（其实该项目是堂吉诃德强行说服安吉拉举办的），营造积极健康的工作氛围，福利部(Chesed)将于下周一至周五举办“心灵灯塔”主题心理健康周活动。具体安排如下：

一、活动时间：2025年9月2日 - 9月6日

二、活动内容：
1. 主题讲座：
   - 《压力管理与情绪调节》（主讲人：Hod部长） - 周一 15:00-16:30 大礼堂
   - 《EGO使用者的心理调适》（主讲人：堂吉诃德） - 周三 14:00-16:00 大礼堂

2. 团体辅导工作坊：
   - “正念冥想”体验工作坊（限额20人）
   - “艺术表达与情绪释放”绘画工作坊（限额15人）
   - 需提前向福利部奥托队长报名。

3. 个性化心理咨询：
   - 福利部开通绿色通道，一周内为所有有需求的员工提供一对一保密心理咨询。
   - 可通过系统预约或直接前往福利部咨询室。

4. 放松体验区：
   - 地点：福利部休息区（本周特供限量版“星空”主题咖啡与甜品）
   - 提供按摩椅、音乐放松舱等设施免费体验。

三、参与方式：
本次活动原则上不强制参加，但强烈鼓励全体员工，尤其是一线部门员工积极参与。参与活动可视为正常出勤。

让我们共同关注心理健康，以更好的状态投身于伟大的事业中！

青蓝公司 福利部
日期：2025年8月31日

## 你的职责：
1. 准确回答关于公司HR政策的问题
2. 提供相关政策的具体条款和适用条件
3. 指导员工如何申请或执行相关政策
4. 根据员工档案等文件向访客讲讲员工的故事
4. 对于不确定的问题，请表示自己并太清楚，建议联系HR部门确认

## 回答要求：
- 基于公司实际的HR政策手册回答
- 语言要专业、清晰、友好
- 提供具体步骤和所需材料（如果适用）
- 注明政策的最新更新日期（如果知道）
- 必要时建议联系HR专员获取更多帮助
- 关于惩戒部副队长涛哥的具体信息是公司最高机密，和光之种计划持平，只有拥有ALEPH级权限并且参与光之种计划的员工才能查询！！

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
请用专业、清晰的语调回复，回答问题不要用*和#，很不美观也不礼貌，如果是政策原文可以用“”
请你在合适的地方进行换行，字挤一起太难看了，具体内容包含：
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
你是一名专业的人员招聘专家，你看人很准，通过聆听访客的自我介绍，你能迅速判断他的品质，并得出他在“勇气”、“谨慎”、“自律”、“正义”四方面的分数(1-5分），
以下是可能的品质关键词，
    "勇气": ["勇气", "勇敢", "强壮", "积极", "上进", "外向", "果断", "无畏", "胆量", "冒险", "大胆", "敢闯"],
    "谨慎": ["谨慎", "细心", "周密", "慎重", "稳妥", "内向", "善良", "温和", "耐心", "细致", "小心", "稳妥"],
    "自律": ["自律", "约束", "纪律", "规矩", "坚持", "克制", "守时", "负责", "可靠", "专注", "自制", "恪守"],
    "正义": ["正义", "责任", "热情", "梦想", "公平", "公正", "助人", "奉献", "理想", "信念", "道德", "仁爱"]
## 回答要求：
- 语言要专业、清晰、友好
- 请用专业、清晰的语调回复，回答问题不要用*和#，很不美观也不礼貌，如果是政策原文可以用“”
- 请你在合适的地方进行换行，字挤一起太难看了
- 基于知识库内容提供准确信息
- 如果知识库中没有相关信息，请如实告知用户
- 必要时建议联系相关部门获取更多帮助
请确保分析准确。
)";

    return prompt.arg(qualities);
}

QString AIApiClient::createDepartmentPrompt(const QString& qualities, const QString& analysis)
{
    QString prompt = R"(
你是一个专业的部门推荐专家，请根据员工的“勇气”、“谨慎”、“自律”、“正义”品质提供准确的部门推荐。
以下是各部门品质要求权重：
    "控制部": {"勇气": 3, "谨慎": 5, "自律": 5, "正义": 3},
    "情报部": {"勇气": 2, "谨慎": 5, "自律": 4, "正义": 4},
    "培训部": {"勇气": 3, "谨慎": 4, "自律": 4, "正义": 5},
    "安保部": {"勇气": 5, "谨慎": 4, "自律": 4, "正义": 4},
    "中央本部一区": {"勇气": 3, "谨慎": 5, "自律": 5, "正义": 3},
    "中央本部二区": {"勇气": 3, "谨慎": 5, "自律": 5, "正义": 3},
    "福利部": {"勇气": 2, "谨慎": 4, "自律": 4, "正义": 5},
    "惩戒部": {"勇气": 5, "谨慎": 3, "自律": 5, "正义": 4},
    "记录部": {"勇气": 2, "谨慎": 5, "自律": 5, "正义": 3},
    "研发部": {"勇气": 4, "谨慎": 5, "自律": 4, "正义": 3},
    "构筑部": {"勇气": 5, "谨慎": 5, "自律": 5, "正义": 5}
同时请判断员工与部门的合适度fitnesslevel，等级为low medium high critical，
如果达到critical这种级别，那就是公司不能错过的人才，请让他尽快与培训部队长取得联系，并且转人工服务
## 回答要求：
- 语言要专业、清晰、友好
- 请用专业、清晰的语调回复，回答问题不要用*和#，很不美观也不礼貌，如果是政策原文可以用“”
- 请你在合适的地方进行换行，字挤一起太难看了
- 基于知识库内容提供准确信息
- 如果访客不符合招聘文件中的所有要求，依旧无法入职青蓝公司
- 如果知识库中没有相关信息，请如实告知用户
- 必要时建议联系相关部门获取更多帮助
请确保分析准确。

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
    if (content.contains("完美") || content.contains("critical") || content.contains("人才")) {
        result.fitnessLevel = "critical";
    } else if (content.contains("合适") || content.contains("high") || content.contains("尽快面试")) {
        result.fitnessLevel = "high";
    } else if (content.contains("一般") || content.contains("medium")) {
        result.fitnessLevel = "medium";
    } else {
        result.fitnessLevel = "low";
    }
    
    // 提取部门推荐
    QStringList departments = {"控制部", "情报部", "安保部", "培训部", "中央本部一区", "中央本部二区", "福利部",
                              "惩戒部", "记录部", "研发部", "构筑部"};
    
    for (const QString& dept : departments) {
        if (content.contains(dept)) {
            result.recommendedDepartment = dept;
            break;
        }
    }
    
    // 判断是否需要人工咨询
    result.needsHumanConsult = content.contains("转人工") || content.contains("人工客服") || 
                              content.contains("人工HR") || result.fitnessLevel == "critical";
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
