#ifndef AIAPICLIENT_H
#define AIAPICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QString>
#include <QTimer>

// AI诊断结果结构
struct AIAnalysisResult {
    QString qualityAnalysis;      // 品质分析
    QString recommendedDepartment; // 推荐部门
    QString fitnessLevel;       // 合适程度 (low/medium/high/critical)
    QStringList possibleCauses;   // 可能原因 //对于建议你去的部门
    QStringList suggestions;      // 未来工作建议
    bool needsHumanConsult;       // 是否需要人工咨询
    QString aiResponse;           // AI完整回复
};

class AIApiClient : public QObject
{
    Q_OBJECT

public:
    explicit AIApiClient(QObject *parent = nullptr);
    ~AIApiClient();
    
    // 发送智能HR请求
    void sendChatRequest(const QString& userInput, const QString& conversationHistory = "");
    
    // 发送品质分析请求
    void sendQualityAnalysis(const QString& qualities, int age = 0, const QString& gender = "");
    
    // 发送部门推荐请求
    void sendDepartmentRecommendation(const QString& qualities, const QString& analysis = "");
    
    // 设置API配置
    void setApiConfig(const QString& baseUrl, const QString& apiKey, const QString& model);
    
    // 检查API连接状态
    bool isConnected() const;
    
    // 获取错误信息
    QString getLastError() const;

signals:
    // AI响应信号
    void chatResponseReceived(const AIAnalysisResult& result);
    void qualityAnalysisReceived(const AIAnalysisResult& result);
    void departmentRecommendationReceived(const AIAnalysisResult& result);
    
    // 错误和状态信号
    void apiError(const QString& error);
    void connectionStatusChanged(bool connected);
    void requestStarted();
    void requestFinished();

private slots:
    void handleChatResponse();
    void handleQualityResponse();
    void handleDepartmentResponse();
    void handleNetworkError(QNetworkReply::NetworkError error);
    void handleSslErrors(const QList<QSslError>& errors);

private:
    // 网络管理
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply;
    
    // API配置
    QString m_baseUrl;
    QString m_apiKey;
    QString m_model;
    
    // 状态管理
    bool m_isConnected;
    QString m_lastError;
    QTimer* m_timeoutTimer;
    
    // 请求类型枚举
    enum RequestType {
        ChatRequest,
        QualityRequest,
        DepartmentRequest
    };
    RequestType m_currentRequestType;
    
    // 私有方法
    QNetworkRequest createApiRequest();
    QJsonObject createRequestBody(const QString& systemPrompt, const QString& userMessage);
    AIAnalysisResult parseApiResponse(const QJsonDocument& response);
    void parseAIResponseContent(AIAnalysisResult& result);
    void setupDefaultConfig();
    QString createChatPrompt(const QString& userInput, const QString& history);
    QString createQualityPrompt(const QString& qualities, int age, const QString& gender);
    QString createDepartmentPrompt(const QString& qualities, const QString& analysis);
};

#endif // AIAPICLIENT_H 
