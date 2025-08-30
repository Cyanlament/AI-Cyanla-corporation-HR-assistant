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
    , m_currentRequestType(TriageRequest)
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
    m_model = "doubao-1-5-pro-32k-character-250715";
    
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

void AIApiClient::sendTriageRequest(const QString& userInput, const QString& conversationHistory)
{
    if (m_currentReply) {
        qDebug() << "请求正在进行中，忽略新请求";
        return;
    }
    
    m_currentRequestType = TriageRequest;
    emit requestStarted();
    
    QString systemPrompt = createTriagePrompt(userInput, conversationHistory);
    QJsonObject requestBody = createRequestBody(systemPrompt, userInput);
    
    QNetworkRequest request = createApiRequest();
    
    QJsonDocument doc(requestBody);
    m_currentReply = m_networkManager->post(request, doc.toJson());
    
    connect(m_currentReply, &QNetworkReply::finished, 
            this, &AIApiClient::handleTriageResponse);
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &AIApiClient::handleNetworkError);
    connect(m_currentReply, &QNetworkReply::sslErrors,
            this, &AIApiClient::handleSslErrors);
    
    m_timeoutTimer->start();
    
    qDebug() << "发送智能HR请求:" << userInput;
}

void AIApiClient::sendSymptomAnalysis(const QString& qualities, int age, const QString& gender)
{
    if (m_currentReply) {
        qDebug() << "请求正在进行中，忽略新请求";
        return;
    }
    
    m_currentRequestType = SymptomRequest;
    emit requestStarted();
    
    QString systemPrompt = createSymptomPrompt(qualities, age, gender);
    QJsonObject requestBody = createRequestBody(systemPrompt, qualities);
    
    QNetworkRequest request = createApiRequest();
    
    QJsonDocument doc(requestBody);
    m_currentReply = m_networkManager->post(request, doc.toJson());
    
    connect(m_currentReply, &QNetworkReply::finished, 
            this, &AIApiClient::handleSymptomResponse);
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &AIApiClient::handleNetworkError);
    connect(m_currentReply, &QNetworkReply::sslErrors,
            this, &AIApiClient::handleSslErrors);
    
    m_timeoutTimer->start();
    
    qDebug() << "发送症状分析请求:" << qualities;
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

QString AIApiClient::createTriagePrompt(const QString& userInput, const QString& history)
{
    QString prompt = R"(
你是一个专业的公司HR政策问答助手，熟悉公司所有人力资源政策和流程。请根据员工的问题提供准确、详细的回答。

## 你的职责：
1. 准确回答关于公司HR政策的问题
2. 提供相关政策的具体条款和适用条件
3. 指导员工如何申请或执行相关政策
4. 提供相关部门的联系方式（如果需要）
5. 对于不确定的问题，建议联系HR部门确认

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

QString AIApiClient::createSymptomPrompt(const QString& qualities, int age, const QString& gender)
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

void AIApiClient::handleTriageResponse()
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
        
        AIDiagnosisResult result = parseApiResponse(doc);
        m_isConnected = true;
        emit connectionStatusChanged(true);
        emit triageResponseReceived(result);
        
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

void AIApiClient::handleSymptomResponse()
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
        
        AIDiagnosisResult result = parseApiResponse(doc);
        m_isConnected = true;
        emit connectionStatusChanged(true);
        emit qualityAnalysisReceived(result);
        
    } else {
        QString error = QString("症状分析请求失败: %1").arg(reply->errorString());
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
        
        AIDiagnosisResult result = parseApiResponse(doc);
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

AIDiagnosisResult AIApiClient::parseApiResponse(const QJsonDocument& response)
{
    AIDiagnosisResult result;
    
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

void AIApiClient::parseAIResponseContent(AIDiagnosisResult& result)
{
    QString content = result.aiResponse.toLower();
    
    // 判断紧急程度
    if (content.contains("危急") || content.contains("critical") || content.contains("立即就医")) {
        result.emergencyLevel = "critical";
    } else if (content.contains("紧急") || content.contains("high") || content.contains("尽快就医")) {
        result.emergencyLevel = "high";
    } else if (content.contains("一般") || content.contains("medium")) {
        result.emergencyLevel = "medium";
    } else {
        result.emergencyLevel = "low";
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
                              content.contains("专业医生") || result.emergencyLevel == "critical";
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
