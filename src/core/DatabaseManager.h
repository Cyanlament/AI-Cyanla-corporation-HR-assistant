#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QDateTime>
#include <QCryptographicHash>

struct UserInfo {
    int id;
    QString username;
    QString email;
    QString phone;
    QString role;
    QString realName;
    QDateTime createdAt;
    QDateTime lastLogin;
    int status;
    QString avatarPath;
};

// 聊天会话信息
struct ChatSession {
    int id;
    int visitorId;
    int staffId;
    QString visitorName;
    QString staffName;
    QDateTime createdAt;
    QDateTime lastMessageAt;
    int status; // 0-已结束, 1-进行中, 2-等待中
    QString lastMessage;
};

// 聊天消息信息
struct ChatMessage {
    int id;
    int sessionId;
    int senderId;
    QString senderName;
    QString senderRole;
    QString content;
    QDateTime timestamp;
    int messageType; // 0-普通消息, 1-系统消息, 2-图片, 3-文件
    int isRead; // 0-未读, 1-已读
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* instance();

    // 数据库初始化
    bool initDatabase();
    void closeDatabase();

    // 用户管理
    bool registerUser(const QString& username, const QString& password,
                      const QString& email, const QString& phone,
                      const QString& role, const QString& realName = "");
    bool loginUser(const QString& username, const QString& password, UserInfo& userInfo);
    bool updateLastLogin(int userId);
    bool isUsernameExists(const QString& username);
    bool isEmailExists(const QString& email);

    // 密码处理
    QString hashPassword(const QString& password);
    bool verifyPassword(const QString& password, const QString& hash);

    // 用户信息
    UserInfo getUserInfo(int userId);
    bool updateUserInfo(const UserInfo& userInfo);
    bool changePassword(int userId, const QString& oldPassword, const QString& newPassword);

    // 聊天会话管理
    int createChatSession(int visitorId, int staffId = 0);
    bool updateChatSession(int sessionId, int staffId);
    bool closeChatSession(int sessionId);
    QList<ChatSession> getActiveSessions();
    QList<ChatSession> getvisitorSessions(int visitorId);
    QList<ChatSession> getStaffSessions(int staffId);
    ChatSession getChatSession(int sessionId);

    // 聊天消息管理
    int sendMessage(int sessionId, int senderId, const QString& content, int messageType = 0);
    QList<ChatMessage> getChatMessages(int sessionId, int limit = 50);
    QList<ChatMessage> getUnreadMessages(int userId);
    bool markMessageAsRead(int messageId);
    bool markSessionAsRead(int sessionId, int userId);

    // 在线状态管理
    bool updateUserOnlineStatus(int userId, bool isOnline);
    QList<UserInfo> getOnlineStaff();

    // 获取用户列表
    QList<UserInfo> getAllUsers();
    QList<UserInfo> getUsersByRole(const QString& role);

signals:
    // 聊天相关信号
    void newMessageReceived(const ChatMessage& message);
    void sessionCreated(const ChatSession& session);
    void sessionUpdated(const ChatSession& session);
    void userOnlineStatusChanged(int userId, bool isOnline);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool createTables();
    QString getDbPath();

    static DatabaseManager* m_instance;
    QSqlDatabase m_database;
};

#endif // DATABASEMANAGER_H
