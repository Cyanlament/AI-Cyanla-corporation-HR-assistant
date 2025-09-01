#include "DatabaseManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSqlRecord>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance(){

    if(!m_instance){
        m_instance = new DatabaseManager;
    }
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent):
    QObject(parent){

}

DatabaseManager::~DatabaseManager(){
    closeDatabase();
}

bool DatabaseManager::initDatabase()
{
    // 获取数据库路径
    QString dbPath = getDbPath();

    // 创建数据库连接
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);

    if (!m_database.open()) {
        qDebug() << "数据库打开失败:" << m_database.lastError().text();
        return false;
    }

    // 创建表
    if (!createTables()) {
        qDebug() << "创建数据表失败";
        return false;
    }

    qDebug() << "数据库初始化成功:" << dbPath;
    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

QString DatabaseManager::getDbPath()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(dataPath);
    }
    return dataPath + "/Cyan.db";
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    // 创建用户表
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(64) NOT NULL,
            email VARCHAR(100) UNIQUE,
            phone VARCHAR(20),
            role VARCHAR(20) NOT NULL,
            real_name VARCHAR(50),
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_login DATETIME,
            status INTEGER DEFAULT 1,
            avatar_path VARCHAR(255)
        )
    )";

    if (!query.exec(createUsersTable)) {
        qDebug() << "创建用户表失败:" << query.lastError().text();
        return false;
    }

    // 创建聊天会话表
    QString createSessionsTable = R"(
        CREATE TABLE IF NOT EXISTS chat_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            visitor_id INTEGER NOT NULL,
            staff_id INTEGER DEFAULT 0,
            visitor_name VARCHAR(50),
            staff_name VARCHAR(50),
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_message_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            status INTEGER DEFAULT 2,
            last_message TEXT,
            FOREIGN KEY (visitor_id) REFERENCES users(id),
            FOREIGN KEY (staff_id) REFERENCES users(id)
        )
    )";

    if (!query.exec(createSessionsTable)) {
        qDebug() << "创建聊天会话表失败:" << query.lastError().text();
        return false;
    }

    // 创建聊天消息表
    QString createMessagesTable = R"(
        CREATE TABLE IF NOT EXISTS chat_messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            session_id INTEGER NOT NULL,
            sender_id INTEGER NOT NULL,
            sender_name VARCHAR(50),
            sender_role VARCHAR(20),
            content TEXT NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            message_type INTEGER DEFAULT 0,
            is_read INTEGER DEFAULT 0,
            FOREIGN KEY (session_id) REFERENCES chat_sessions(id),
            FOREIGN KEY (sender_id) REFERENCES users(id)
        )
    )";

    if (!query.exec(createMessagesTable)) {
        qDebug() << "创建聊天消息表失败:" << query.lastError().text();
        return false;
    }

    // 在用户表中添加在线状态字段（如果不存在）
    query.exec("ALTER TABLE users ADD COLUMN is_online INTEGER DEFAULT 0");

    // 创建默认管理员账户（如果不存在）
    if (!isUsernameExists("admin")) {
        registerUser("admin", "admin123", "admin@Cyan.com", "", "管理员", "Ayin");
    }

    // 创建测试HR客服账户
    if (!isUsernameExists("staff1")) {
        registerUser("staff1", "123456", "staff1@Cyan.com", "", "客服小祥", "丰川祥子");
    }
    if (!isUsernameExists("staff2")) {
        registerUser("staff2", "123456", "staff2@Cyan.com", "", "客服小唐", "桑丘");
    }

    return true;
}

bool DatabaseManager::registerUser(const QString& username, const QString& password,
                                   const QString& email, const QString& phone,
                                   const QString& role, const QString& realName)
{
    // 检查用户名是否已存在
    if (isUsernameExists(username)) {
        return false;
    }

    // 检查邮箱是否已存在
    if (!email.isEmpty() && isEmailExists(email)) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO users (username, password_hash, email, phone, role, real_name)
        VALUES (?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(username);
    query.addBindValue(hashPassword(password));
    query.addBindValue(email);
    query.addBindValue(phone);
    query.addBindValue(role);
    query.addBindValue(realName);

    if (!query.exec()) {
        qDebug() << "用户注册失败:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::loginUser(const QString& username, const QString& password, UserInfo& userInfo)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, username, email, phone, role, real_name, created_at, last_login, status, avatar_path, password_hash
        FROM users
        WHERE username = ? AND status = 1
    )");

    query.addBindValue(username);

    if (!query.exec()) {
        qDebug() << "登录查询失败:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        QString storedHash = query.value("password_hash").toString();

        // 验证密码
        if (!verifyPassword(password, storedHash)) {
            return false;
        }

        // 填充用户信息
        userInfo.id = query.value("id").toInt();
        userInfo.username = query.value("username").toString();
        userInfo.email = query.value("email").toString();
        userInfo.phone = query.value("phone").toString();
        userInfo.role = query.value("role").toString();
        userInfo.realName = query.value("real_name").toString();
        userInfo.createdAt = query.value("created_at").toDateTime();
        userInfo.lastLogin = query.value("last_login").toDateTime();
        userInfo.status = query.value("status").toInt();
        userInfo.avatarPath = query.value("avatar_path").toString();

        // 更新最后登录时间
        updateLastLogin(userInfo.id);

        return true;
    }

    return false;
}

bool DatabaseManager::updateLastLogin(int userId)
{
    QSqlQuery query(m_database);
    query.prepare("UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(userId);

    return query.exec();
}

// ========== 聊天会话管理 ==========

int DatabaseManager::createChatSession(int visitorId, int staffId)
{
    UserInfo visitor = getUserInfo(visitorId);
    QString visitorName = visitor.realName.isEmpty() ? visitor.username : visitor.realName;

    QString staffName = "";
    if (staffId > 0) {
        UserInfo staff = getUserInfo(staffId);
        staffName = staff.realName.isEmpty() ? staff.username : staff.realName;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO chat_sessions (visitor_id, staff_id, visitor_name, staff_name, status)
        VALUES (?, ?, ?, ?, ?)
    )");

    query.addBindValue(visitorId);
    query.addBindValue(staffId > 0 ? staffId : QVariant());
    query.addBindValue(visitorName);
    query.addBindValue(staffName);
    query.addBindValue(staffId > 0 ? 1 : 2); // 1-进行中, 2-等待中

    if (query.exec()) {
        int sessionId = query.lastInsertId().toInt();

        // 发送系统消息
        QString systemMsg = staffId > 0 ?
                                QString("客服 %1 已接入对话").arg(staffName) :
                                "您好！请描述您的问题，我们会尽快为您安排HR客服。";
        sendMessage(sessionId, 0, systemMsg, 1); // 系统消息

        // 发送信号
        ChatSession session = getChatSession(sessionId);
        emit sessionCreated(session);

        return sessionId;
    }

    return -1;
}

bool DatabaseManager::updateChatSession(int sessionId, int staffId)
{
    UserInfo staff = getUserInfo(staffId);
    QString staffName = staff.realName.isEmpty() ? staff.username : staff.realName;

    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE chat_sessions
        SET staff_id = ?, staff_name = ?, status = 1, last_message_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )");

    query.addBindValue(staffId);
    query.addBindValue(staffName);
    query.addBindValue(sessionId);

    if (query.exec()) {
        // 发送系统消息
        sendMessage(sessionId, 0, QString("客服 %1 已接入对话").arg(staffName), 1);

        // 发送信号
        ChatSession session = getChatSession(sessionId);
        emit sessionUpdated(session);

        return true;
    }

    return false;
}

bool DatabaseManager::closeChatSession(int sessionId)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE chat_sessions
        SET status = 0, last_message_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )");

    query.addBindValue(sessionId);

    if (query.exec()) {
        // 发送系统消息
        sendMessage(sessionId, 0, "对话已结束，感谢您的咨询！", 1);

        // 发送信号
        ChatSession session = getChatSession(sessionId);
        emit sessionUpdated(session);

        return true;
    }

    return false;
}

QList<ChatSession> DatabaseManager::getActiveSessions()
{
    QList<ChatSession> sessions;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, visitor_id, staff_id, visitor_name, staff_name,
               created_at, last_message_at, status, last_message
        FROM chat_sessions
        WHERE status > 0
        ORDER BY last_message_at DESC
    )");

    if (query.exec()) {
        while (query.next()) {
            ChatSession session;
            session.id = query.value("id").toInt();
            session.visitorId = query.value("visitor_id").toInt();
            session.staffId = query.value("staff_id").toInt();
            session.visitorName = query.value("visitor_name").toString();
            session.staffName = query.value("staff_name").toString();
            session.createdAt = query.value("created_at").toDateTime();
            session.lastMessageAt = query.value("last_message_at").toDateTime();
            session.status = query.value("status").toInt();
            session.lastMessage = query.value("last_message").toString();

            sessions.append(session);
        }
    }

    return sessions;
}

QList<ChatSession> DatabaseManager::getvisitorSessions(int visitorId)
{
    QList<ChatSession> sessions;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, visitor_id, staff_id, visitor_name, staff_name,
               created_at, last_message_at, status, last_message
        FROM chat_sessions
        WHERE visitor_id = ?
        ORDER BY last_message_at DESC
    )");

    query.addBindValue(visitorId);

    if (query.exec()) {
        while (query.next()) {
            ChatSession session;
            session.id = query.value("id").toInt();
            session.visitorId = query.value("visitor_id").toInt();
            session.staffId = query.value("staff_id").toInt();
            session.visitorName = query.value("visitor_name").toString();
            session.staffName = query.value("staff_name").toString();
            session.createdAt = query.value("created_at").toDateTime();
            session.lastMessageAt = query.value("last_message_at").toDateTime();
            session.status = query.value("status").toInt();
            session.lastMessage = query.value("last_message").toString();

            sessions.append(session);
        }
    }

    return sessions;
}

QList<ChatSession> DatabaseManager::getStaffSessions(int staffId)
{
    QList<ChatSession> sessions;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, visitor_id, staff_id, visitor_name, staff_name,
               created_at, last_message_at, status, last_message
        FROM chat_sessions
        WHERE staff_id = ? AND status > 0
        ORDER BY last_message_at DESC
    )");

    query.addBindValue(staffId);

    if (query.exec()) {
        while (query.next()) {
            ChatSession session;
            session.id = query.value("id").toInt();
            session.visitorId = query.value("visitor_id").toInt();
            session.staffId = query.value("staff_id").toInt();
            session.visitorName = query.value("visitor_name").toString();
            session.staffName = query.value("staff_name").toString();
            session.createdAt = query.value("created_at").toDateTime();
            session.lastMessageAt = query.value("last_message_at").toDateTime();
            session.status = query.value("status").toInt();
            session.lastMessage = query.value("last_message").toString();

            sessions.append(session);
        }
    }

    return sessions;
}

ChatSession DatabaseManager::getChatSession(int sessionId)
{
    ChatSession session;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, visitor_id, staff_id, visitor_name, staff_name,
               created_at, last_message_at, status, last_message
        FROM chat_sessions
        WHERE id = ?
    )");

    query.addBindValue(sessionId);

    if (query.exec() && query.next()) {
        session.id = query.value("id").toInt();
        session.visitorId = query.value("visitor_id").toInt();
        session.staffId = query.value("staff_id").toInt();
        session.visitorName = query.value("visitor_name").toString();
        session.staffName = query.value("staff_name").toString();
        session.createdAt = query.value("created_at").toDateTime();
        session.lastMessageAt = query.value("last_message_at").toDateTime();
        session.status = query.value("status").toInt();
        session.lastMessage = query.value("last_message").toString();
    }

    return session;
}

// ========== 聊天消息管理 ==========

int DatabaseManager::sendMessage(int sessionId, int senderId, const QString& content, int messageType)
{
    QString senderName = "系统";
    QString senderRole = "system";

    if (senderId > 0) {
        UserInfo sender = getUserInfo(senderId);
        senderName = sender.realName.isEmpty() ? sender.username : sender.realName;
        senderRole = sender.role;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO chat_messages (session_id, sender_id, sender_name, sender_role, content, message_type)
        VALUES (?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(sessionId);
    query.addBindValue(senderId);
    query.addBindValue(senderName);
    query.addBindValue(senderRole);
    query.addBindValue(content);
    query.addBindValue(messageType);

    if (query.exec()) {
        int messageId = query.lastInsertId().toInt();

        // 更新会话的最后消息时间和内容
        QSqlQuery updateQuery(m_database);
        updateQuery.prepare(R"(
            UPDATE chat_sessions
            SET last_message_at = CURRENT_TIMESTAMP, last_message = ?
            WHERE id = ?
        )");
        updateQuery.addBindValue(content);
        updateQuery.addBindValue(sessionId);
        updateQuery.exec();

        // 获取完整消息信息并发送信号
        ChatMessage message;
        message.id = messageId;
        message.sessionId = sessionId;
        message.senderId = senderId;
        message.senderName = senderName;
        message.senderRole = senderRole;
        message.content = content;
        message.timestamp = QDateTime::currentDateTime();
        message.messageType = messageType;
        message.isRead = 0;

        emit newMessageReceived(message);

        return messageId;
    }

    return -1;
}

QList<ChatMessage> DatabaseManager::getChatMessages(int sessionId, int limit)
{
    QList<ChatMessage> messages;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, session_id, sender_id, sender_name, sender_role,
               content, timestamp, message_type, is_read
        FROM chat_messages
        WHERE session_id = ?
        ORDER BY timestamp ASC
        LIMIT ?
    )");

    query.addBindValue(sessionId);
    query.addBindValue(limit);

    if (query.exec()) {
        while (query.next()) {
            ChatMessage message;
            message.id = query.value("id").toInt();
            message.sessionId = query.value("session_id").toInt();
            message.senderId = query.value("sender_id").toInt();
            message.senderName = query.value("sender_name").toString();
            message.senderRole = query.value("sender_role").toString();
            message.content = query.value("content").toString();
            message.timestamp = query.value("timestamp").toDateTime();
            message.messageType = query.value("message_type").toInt();
            message.isRead = query.value("is_read").toInt();

            messages.append(message);
        }
    }

    return messages;
}

QList<ChatMessage> DatabaseManager::getUnreadMessages(int userId)
{
    QList<ChatMessage> messages;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT m.id, m.session_id, m.sender_id, m.sender_name, m.sender_role,
               m.content, m.timestamp, m.message_type, m.is_read
        FROM chat_messages m
        JOIN chat_sessions s ON m.session_id = s.id
        WHERE (s.visitor_id = ? OR s.staff_id = ?)
        AND m.sender_id != ?
        AND m.is_read = 0
        ORDER BY m.timestamp ASC
    )");

    query.addBindValue(userId);
    query.addBindValue(userId);
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            ChatMessage message;
            message.id = query.value("id").toInt();
            message.sessionId = query.value("session_id").toInt();
            message.senderId = query.value("sender_id").toInt();
            message.senderName = query.value("sender_name").toString();
            message.senderRole = query.value("sender_role").toString();
            message.content = query.value("content").toString();
            message.timestamp = query.value("timestamp").toDateTime();
            message.messageType = query.value("message_type").toInt();
            message.isRead = query.value("is_read").toInt();

            messages.append(message);
        }
    }

    return messages;
}

bool DatabaseManager::markMessageAsRead(int messageId)
{
    QSqlQuery query(m_database);
    query.prepare("UPDATE chat_messages SET is_read = 1 WHERE id = ?");
    query.addBindValue(messageId);

    return query.exec();
}

bool DatabaseManager::markSessionAsRead(int sessionId, int userId)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE chat_messages
        SET is_read = 1
        WHERE session_id = ? AND sender_id != ?
    )");

    query.addBindValue(sessionId);
    query.addBindValue(userId);

    return query.exec();
}

// ========== 在线状态管理 ==========

bool DatabaseManager::updateUserOnlineStatus(int userId, bool isOnline)
{
    QSqlQuery query(m_database);
    query.prepare("UPDATE users SET is_online = ? WHERE id = ?");
    query.addBindValue(isOnline ? 1 : 0);
    query.addBindValue(userId);

    if (query.exec()) {
        emit userOnlineStatusChanged(userId, isOnline);
        return true;
    }

    return false;
}

QList<UserInfo> DatabaseManager::getOnlineStaff()
{
    QList<UserInfo> staffList;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, username, email, phone, role, real_name,
               created_at, last_login, status, avatar_path
        FROM users
        WHERE role = '客服' AND is_online = 1 AND status = 1
        ORDER BY last_login DESC
    )");

    if (query.exec()) {
        while (query.next()) {
            UserInfo user;
            user.id = query.value("id").toInt();
            user.username = query.value("username").toString();
            user.email = query.value("email").toString();
            user.phone = query.value("phone").toString();
            user.role = query.value("role").toString();
            user.realName = query.value("real_name").toString();
            user.createdAt = query.value("created_at").toDateTime();
            user.lastLogin = query.value("last_login").toDateTime();
            user.status = query.value("status").toInt();
            user.avatarPath = query.value("avatar_path").toString();

            staffList.append(user);
        }
    }

    return staffList;
}

// ========== 用户列表获取 ==========

QList<UserInfo> DatabaseManager::getAllUsers()
{
    QList<UserInfo> users;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, username, email, phone, role, real_name,
               created_at, last_login, status, avatar_path
        FROM users
        WHERE status = 1
        ORDER BY created_at DESC
    )");

    if (query.exec()) {
        while (query.next()) {
            UserInfo user;
            user.id = query.value("id").toInt();
            user.username = query.value("username").toString();
            user.email = query.value("email").toString();
            user.phone = query.value("phone").toString();
            user.role = query.value("role").toString();
            user.realName = query.value("real_name").toString();
            user.createdAt = query.value("created_at").toDateTime();
            user.lastLogin = query.value("last_login").toDateTime();
            user.status = query.value("status").toInt();
            user.avatarPath = query.value("avatar_path").toString();

            users.append(user);
        }
    }

    return users;
}

QList<UserInfo> DatabaseManager::getUsersByRole(const QString& role)
{
    QList<UserInfo> users;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, username, email, phone, role, real_name,
               created_at, last_login, status, avatar_path
        FROM users
        WHERE role = ? AND status = 1
        ORDER BY created_at DESC
    )");

    query.addBindValue(role);

    if (query.exec()) {
        while (query.next()) {
            UserInfo user;
            user.id = query.value("id").toInt();
            user.username = query.value("username").toString();
            user.email = query.value("email").toString();
            user.phone = query.value("phone").toString();
            user.role = query.value("role").toString();
            user.realName = query.value("real_name").toString();
            user.createdAt = query.value("created_at").toDateTime();
            user.lastLogin = query.value("last_login").toDateTime();
            user.status = query.value("status").toInt();
            user.avatarPath = query.value("avatar_path").toString();

            users.append(user);
        }
    }

    return users;
}

bool DatabaseManager::isUsernameExists(const QString& username)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

bool DatabaseManager::isEmailExists(const QString& email)
{
    if (email.isEmpty()) return false;

    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM users WHERE email = ?");
    query.addBindValue(email);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

QString DatabaseManager::hashPassword(const QString& password)
{
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool DatabaseManager::verifyPassword(const QString& password, const QString& hash)
{
    return hashPassword(password) == hash;
}

UserInfo DatabaseManager::getUserInfo(int userId)
{
    UserInfo userInfo;

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, username, email, phone, role, real_name, created_at, last_login, status, avatar_path
        FROM users
        WHERE id = ?
    )");

    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        userInfo.id = query.value("id").toInt();
        userInfo.username = query.value("username").toString();
        userInfo.email = query.value("email").toString();
        userInfo.phone = query.value("phone").toString();
        userInfo.role = query.value("role").toString();
        userInfo.realName = query.value("real_name").toString();
        userInfo.createdAt = query.value("created_at").toDateTime();
        userInfo.lastLogin = query.value("last_login").toDateTime();
        userInfo.status = query.value("status").toInt();
        userInfo.avatarPath = query.value("avatar_path").toString();
    }

    return userInfo;
}

bool DatabaseManager::updateUserInfo(const UserInfo& userInfo)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE users
        SET email = ?, phone = ?, real_name = ?, avatar_path = ?
        WHERE id = ?
    )");

    query.addBindValue(userInfo.email);
    query.addBindValue(userInfo.phone);
    query.addBindValue(userInfo.realName);
    query.addBindValue(userInfo.avatarPath);
    query.addBindValue(userInfo.id);

    return query.exec();
}

bool DatabaseManager::changePassword(int userId, const QString& oldPassword, const QString& newPassword)
{
    // 先验证旧密码
    QSqlQuery query(m_database);
    query.prepare("SELECT password_hash FROM users WHERE id = ?");
    query.addBindValue(userId);

    if (!query.exec() || !query.next()) {
        return false;
    }

    QString storedHash = query.value(0).toString();
    if (!verifyPassword(oldPassword, storedHash)) {
        return false;
    }

    // 更新新密码
    query.prepare("UPDATE users SET password_hash = ? WHERE id = ?");
    query.addBindValue(hashPassword(newPassword));
    query.addBindValue(userId);

    return query.exec();
}
