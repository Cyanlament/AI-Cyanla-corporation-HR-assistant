#ifndef CHATSTORAGE_H
#define CHATSTORAGE_H


#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

// 消息结构体
struct Message {
    int id;
    QString sender;
    QString receiver;
    QString message;
    QDateTime timestamp;

    // 构造函数
    Message() : id(-1) {}
    Message(const QString &s, const QString &r, const QString &m, const QDateTime &t = QDateTime::currentDateTime())
        : id(-1), sender(s), receiver(r), message(m), timestamp(t) {}

    // 转换为 JSON 对象
    QJsonObject toJson() const;

    // 从 JSON 对象创建
    static Message fromJson(const QJsonObject &json);
};

class ChatStorage : public QObject
{
    Q_OBJECT

public:
    explicit ChatStorage(QObject *parent = nullptr);
    ~ChatStorage();

    // 初始化数据库
    bool initialize();

    // 插入新消息
    bool insertMessage(const QString &sender, const QString &receiver,
                       const QString &message, const QDateTime &timestamp = QDateTime::currentDateTime());
    bool insertMessage(const Message &msg);

    // 查询消息记录
    QList<Message> getAllMessages();
    QList<Message> getMessagesBetweenUsers(const QString &user1, const QString &user2);
    QList<Message> getMessagesByTimeRange(const QDateTime &startTime, const QDateTime &endTime);
    QList<Message> getMessagesBySender(const QString &sender);
    QList<Message> getMessagesByReceiver(const QString &receiver);

    // 分页查询
    QList<Message> getMessagesWithPagination(int offset = 0, int limit = 50);
    QList<Message> getMessagesBetweenUsersWithPagination(const QString &user1, const QString &user2,
                                                         int offset = 0, int limit = 50);

    // 删除操作
    bool deleteMessage(int messageId);
    bool deleteMessagesBetweenUsers(const QString &user1, const QString &user2);
    bool deleteMessagesByTimeRange(const QDateTime &startTime, const QDateTime &endTime);
    bool deleteAllMessages();

    // 统计信息
    int getTotalMessageCount();
    int getMessageCount(const QString &sender, const QString &receiver);
    QStringList getAllUsers();

    // 数据导出
    QString exportToJson(const QList<Message> &messages);
    QString exportAllToJson();
    bool exportToFile(const QString &filePath, const QList<Message> &messages);

    // 数据库状态
    bool isValid() const;
    QString getLastError() const;

signals:
    void messageInserted(const Message &message);
    void messageDeleted(int messageId);
    void databaseError(const QString &error);

private:
    bool createTables();
    bool checkDatabaseConnection();
    void setLastError(const QString &error);
    QString getDatabasePath();

    // 辅助查询方法
    QList<Message> executeMessageQuery(const QString &queryString, const QVariantList &parameters = QVariantList());

private:
    QSqlDatabase m_database;
    QString m_connectionName;
    QString m_lastError;
    bool m_isInitialized;

    static const QString DATABASE_NAME;
    static const QString TABLE_NAME;
    static const int DATABASE_VERSION;
};

#endif // CHATSTORAGE_H
