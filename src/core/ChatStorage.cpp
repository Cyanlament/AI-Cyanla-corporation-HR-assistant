#include "ChatStorage.h"
#include <QUuid>
#include <QFile>
#include <QTextStream>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

// 静态常量定义
const QString ChatStorage::DATABASE_NAME = "chat_history.db";
const QString ChatStorage::TABLE_NAME = "chat_messages";
const int ChatStorage::DATABASE_VERSION = 1;

// Message 结构体实现
QJsonObject Message::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["sender"] = sender;
    json["receiver"] = receiver;
    json["message"] = message;
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    return json;
}

Message Message::fromJson(const QJsonObject &json)
{
    Message msg;
    msg.id = json["id"].toInt();
    msg.sender = json["sender"].toString();
    msg.receiver = json["receiver"].toString();
    msg.message = json["message"].toString();
    msg.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    return msg;
}

// ChatStorage 实现
ChatStorage::ChatStorage(QObject *parent)
    : QObject(parent)
    , m_connectionName(QString("ChatStorage_%1").arg(QUuid::createUuid().toString()))
    , m_isInitialized(false)
{
    qDebug() << "ChatStorage: 初始化聊天存储模块";
}

ChatStorage::~ChatStorage()
{
    if (m_database.isOpen()) {
        qDebug() << "ChatStorage: 关闭数据库连接";
        m_database.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool ChatStorage::initialize()
{
    if (m_isInitialized) {
        qDebug() << "ChatStorage: 数据库已初始化";
        return true;
    }

    QString dbPath = getDatabasePath();
    qDebug() << "ChatStorage: 数据库路径:" << dbPath;

    // 确保数据库目录存在
    QDir dbDir = QFileInfo(dbPath).absoluteDir();
    if (!dbDir.exists()) {
        if (!dbDir.mkpath(".")) {
            setLastError("无法创建数据库目录: " + dbDir.absolutePath());
            return false;
        }
    }

    // 创建数据库连接
    m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_database.setDatabaseName(dbPath);

    if (!m_database.open()) {
        setLastError("无法打开数据库: " + m_database.lastError().text());
        return false;
    }

    qDebug() << "ChatStorage: 数据库连接成功";

    // 创建表结构
    if (!createTables()) {
        return false;
    }

    m_isInitialized = true;
    qDebug() << "ChatStorage: 初始化完成";
    return true;
}

bool ChatStorage::createTables()
{
    QSqlQuery query(m_database);

    // 创建聊天消息表
    QString createTableSQL = QString(
                                 "CREATE TABLE IF NOT EXISTS %1 ("
                                 "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                 "sender TEXT NOT NULL, "
                                 "receiver TEXT NOT NULL, "
                                 "message TEXT NOT NULL, "
                                 "timestamp DATETIME NOT NULL, "
                                 "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                                 ")"
                                 ).arg(TABLE_NAME);

    if (!query.exec(createTableSQL)) {
        setLastError("创建表失败: " + query.lastError().text());
        return false;
    }

    // 创建索引以提高查询性能
    QStringList indexes = {
        QString("CREATE INDEX IF NOT EXISTS idx_sender ON %1(sender)").arg(TABLE_NAME),
        QString("CREATE INDEX IF NOT EXISTS idx_receiver ON %1(receiver)").arg(TABLE_NAME),
        QString("CREATE INDEX IF NOT EXISTS idx_timestamp ON %1(timestamp)").arg(TABLE_NAME),
        QString("CREATE INDEX IF NOT EXISTS idx_sender_receiver ON %1(sender, receiver)").arg(TABLE_NAME)
    };

    for (const QString &indexSQL : indexes) {
        if (!query.exec(indexSQL)) {
            qWarning() << "ChatStorage: 创建索引警告:" << query.lastError().text();
            // 索引创建失败不影响主要功能，只记录警告
        }
    }

    qDebug() << "ChatStorage: 表结构创建完成";
    return true;
}

QString ChatStorage::getDatabasePath()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataPath.isEmpty()) {
        dataPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        dataPath += "/HospAI";
    }

    QDir().mkpath(dataPath);
    return dataPath + "/" + DATABASE_NAME;
}

bool ChatStorage::insertMessage(const QString &sender, const QString &receiver,
                                const QString &message, const QDateTime &timestamp)
{
    if (!checkDatabaseConnection()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
                      "INSERT INTO %1 (sender, receiver, message, timestamp) "
                      "VALUES (?, ?, ?, ?)"
                      ).arg(TABLE_NAME));

    query.addBindValue(sender);
    query.addBindValue(receiver);
    query.addBindValue(message);
    query.addBindValue(timestamp);

    if (!query.exec()) {
        setLastError("插入消息失败: " + query.lastError().text());
        return false;
    }

    // 获取插入的记录ID
    int messageId = query.lastInsertId().toInt();

    // 创建消息对象并发送信号
    Message msg(sender, receiver, message, timestamp);
    msg.id = messageId;

    qDebug() << QString("ChatStorage: 插入消息成功 [ID: %1, %2 -> %3]")
                    .arg(messageId).arg(sender).arg(receiver);

    emit messageInserted(msg);
    return true;
}

bool ChatStorage::insertMessage(const Message &msg)
{
    return insertMessage(msg.sender, msg.receiver, msg.message, msg.timestamp);
}

QList<Message> ChatStorage::getAllMessages()
{
    QString queryString = QString("SELECT id, sender, receiver, message, timestamp FROM %1 ORDER BY timestamp ASC").arg(TABLE_NAME);
    return executeMessageQuery(queryString);
}

QList<Message> ChatStorage::getMessagesBetweenUsers(const QString &user1, const QString &user2)
{
    QString queryString = QString(
                              "SELECT id, sender, receiver, message, timestamp FROM %1 "
                              "WHERE (sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?) "
                              "ORDER BY timestamp ASC"
                              ).arg(TABLE_NAME);

    return executeMessageQuery(queryString, {user1, user2, user2, user1});
}

QList<Message> ChatStorage::getMessagesByTimeRange(const QDateTime &startTime, const QDateTime &endTime)
{
    QString queryString = QString(
                              "SELECT id, sender, receiver, message, timestamp FROM %1 "
                              "WHERE timestamp BETWEEN ? AND ? "
                              "ORDER BY timestamp ASC"
                              ).arg(TABLE_NAME);

    return executeMessageQuery(queryString, {startTime, endTime});
}

QList<Message> ChatStorage::getMessagesBySender(const QString &sender)
{
    QString queryString = QString(
                              "SELECT id, sender, receiver, message, timestamp FROM %1 "
                              "WHERE sender = ? ORDER BY timestamp ASC"
                              ).arg(TABLE_NAME);

    return executeMessageQuery(queryString, {sender});
}

QList<Message> ChatStorage::getMessagesByReceiver(const QString &receiver)
{
    QString queryString = QString(
                              "SELECT id, sender, receiver, message, timestamp FROM %1 "
                              "WHERE receiver = ? ORDER BY timestamp ASC"
                              ).arg(TABLE_NAME);

    return executeMessageQuery(queryString, {receiver});
}

QList<Message> ChatStorage::getMessagesWithPagination(int offset, int limit)
{
    QString queryString = QString(
                              "SELECT id, sender, receiver, message, timestamp FROM %1 "
                              "ORDER BY timestamp DESC LIMIT ? OFFSET ?"
                              ).arg(TABLE_NAME);

    return executeMessageQuery(queryString, {limit, offset});
}

QList<Message> ChatStorage::getMessagesBetweenUsersWithPagination(const QString &user1, const QString &user2,
                                                                  int offset, int limit)
{
    QString queryString = QString(
                              "SELECT id, sender, receiver, message, timestamp FROM %1 "
                              "WHERE (sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?) "
                              "ORDER BY timestamp DESC LIMIT ? OFFSET ?"
                              ).arg(TABLE_NAME);

    return executeMessageQuery(queryString, {user1, user2, user2, user1, limit, offset});
}

bool ChatStorage::deleteMessage(int messageId)
{
    if (!checkDatabaseConnection()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM %1 WHERE id = ?").arg(TABLE_NAME));
    query.addBindValue(messageId);

    if (!query.exec()) {
        setLastError("删除消息失败: " + query.lastError().text());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    if (rowsAffected > 0) {
        qDebug() << "ChatStorage: 删除消息成功, ID:" << messageId;
        emit messageDeleted(messageId);
        return true;
    } else {
        setLastError("未找到要删除的消息");
        return false;
    }
}

bool ChatStorage::deleteMessagesBetweenUsers(const QString &user1, const QString &user2)
{
    if (!checkDatabaseConnection()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
                      "DELETE FROM %1 WHERE (sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?)"
                      ).arg(TABLE_NAME));

    query.addBindValue(user1);
    query.addBindValue(user2);
    query.addBindValue(user2);
    query.addBindValue(user1);

    if (!query.exec()) {
        setLastError("删除用户间消息失败: " + query.lastError().text());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    qDebug() << QString("ChatStorage: 删除用户间消息成功, %1 <-> %2, 删除 %3 条记录")
                    .arg(user1).arg(user2).arg(rowsAffected);
    return true;
}

bool ChatStorage::deleteMessagesByTimeRange(const QDateTime &startTime, const QDateTime &endTime)
{
    if (!checkDatabaseConnection()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM %1 WHERE timestamp BETWEEN ? AND ?").arg(TABLE_NAME));
    query.addBindValue(startTime);
    query.addBindValue(endTime);

    if (!query.exec()) {
        setLastError("按时间范围删除消息失败: " + query.lastError().text());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    qDebug() << QString("ChatStorage: 按时间删除消息成功, 删除 %1 条记录").arg(rowsAffected);
    return true;
}

bool ChatStorage::deleteAllMessages()
{
    if (!checkDatabaseConnection()) {
        return false;
    }

    QSqlQuery query(m_database);
    if (!query.exec(QString("DELETE FROM %1").arg(TABLE_NAME))) {
        setLastError("清空所有消息失败: " + query.lastError().text());
        return false;
    }

    qDebug() << "ChatStorage: 清空所有消息成功";
    return true;
}

int ChatStorage::getTotalMessageCount()
{
    if (!checkDatabaseConnection()) {
        return -1;
    }

    QSqlQuery query(m_database);
    if (!query.exec(QString("SELECT COUNT(*) FROM %1").arg(TABLE_NAME))) {
        setLastError("获取消息总数失败: " + query.lastError().text());
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int ChatStorage::getMessageCount(const QString &sender, const QString &receiver)
{
    if (!checkDatabaseConnection()) {
        return -1;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
                      "SELECT COUNT(*) FROM %1 WHERE (sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?)"
                      ).arg(TABLE_NAME));

    query.addBindValue(sender);
    query.addBindValue(receiver);
    query.addBindValue(receiver);
    query.addBindValue(sender);

    if (!query.exec()) {
        setLastError("获取用户间消息数失败: " + query.lastError().text());
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QStringList ChatStorage::getAllUsers()
{
    if (!checkDatabaseConnection()) {
        return QStringList();
    }

    QSqlQuery query(m_database);
    if (!query.exec(QString("SELECT DISTINCT sender FROM %1 UNION SELECT DISTINCT receiver FROM %1").arg(TABLE_NAME))) {
        setLastError("获取用户列表失败: " + query.lastError().text());
        return QStringList();
    }

    QStringList users;
    while (query.next()) {
        users.append(query.value(0).toString());
    }
    return users;
}

QString ChatStorage::exportToJson(const QList<Message> &messages)
{
    QJsonArray jsonArray;
    for (const Message &msg : messages) {
        jsonArray.append(msg.toJson());
    }

    QJsonObject rootObject;
    rootObject["export_timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    rootObject["message_count"] = messages.size();
    rootObject["messages"] = jsonArray;

    QJsonDocument doc(rootObject);
    return doc.toJson();
}

QString ChatStorage::exportAllToJson()
{
    return exportToJson(getAllMessages());
}

bool ChatStorage::exportToFile(const QString &filePath, const QList<Message> &messages)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setLastError("无法打开导出文件: " + filePath);
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif
    out << exportToJson(messages);

    qDebug() << QString("ChatStorage: 导出 %1 条消息到文件: %2").arg(messages.size()).arg(filePath);
    return true;
}

bool ChatStorage::isValid() const
{
    return m_isInitialized && m_database.isOpen();
}

QString ChatStorage::getLastError() const
{
    return m_lastError;
}

bool ChatStorage::checkDatabaseConnection()
{
    if (!m_isInitialized) {
        setLastError("数据库未初始化");
        return false;
    }

    if (!m_database.isOpen()) {
        setLastError("数据库连接已断开");
        return false;
    }

    return true;
}

void ChatStorage::setLastError(const QString &error)
{
    m_lastError = error;
    qWarning() << "ChatStorage 错误:" << error;
    emit databaseError(error);
}

QList<Message> ChatStorage::executeMessageQuery(const QString &queryString, const QVariantList &parameters)
{
    QList<Message> messages;

    if (!checkDatabaseConnection()) {
        return messages;
    }

    QSqlQuery query(m_database);
    query.prepare(queryString);

    for (const QVariant &param : parameters) {
        query.addBindValue(param);
    }

    if (!query.exec()) {
        setLastError("查询消息失败: " + query.lastError().text());
        return messages;
    }

    while (query.next()) {
        Message msg;
        msg.id = query.value(0).toInt();
        msg.sender = query.value(1).toString();
        msg.receiver = query.value(2).toString();
        msg.message = query.value(3).toString();
        msg.timestamp = query.value(4).toDateTime();
        messages.append(msg);
    }

    qDebug() << QString("ChatStorage: 查询返回 %1 条消息").arg(messages.size());
    return messages;
}
