#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QThread>
#include <QDebug>
#include <optional>
#include <QDateTime>

#include "messagedatabase.h"

QSqlDatabase DataBase::getThreadDatabase() {
    const QString connName = QString("connection_%1").arg((quintptr)QThread::currentThreadId());

    QSqlDatabase db;
    if (QSqlDatabase::contains(connName)) {
        db = QSqlDatabase::database(connName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName("chat_db");
    }

    if (!db.isOpen()) {
        if (!db.open()) {
            qCritical() << "[ERROR] Cannot open DB in thread:"
                        << QThread::currentThread()
                        << db.lastError().text();

            throw std::runtime_error("Cannot open database");
        }
    }

    return db;
}

Message DataBase::getMessageFromQuery(const QSqlQuery& query){
    Message msg;
    msg.id = query.value("id").toInt();
    msg.chatId = query.value("chat_id").toInt();
    msg.senderId = query.value("sender_id").toInt();
    msg.text = query.value("text").toString().toStdString();
    QDateTime dt = query.value("timestamp").toDateTime();
    dt = dt.addSecs(3 * 3600);
    msg.timestamp = dt.toString(Qt::ISODate);
    return msg;
}

template<typename... Args>
bool DataBase::executeQuery(QSqlQuery& query, Args&&... args) {
    auto toVariant = [](auto&& value) -> QVariant {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return QVariant(QString::fromStdString(value));
        } else {
            return QVariant(std::forward<decltype(value)>(value));
        }
    };

    (query.addBindValue(toVariant(std::forward<Args>(args))), ...);
    bool success = query.exec();
    if (!success) {
        qDebug() << "[ERROR] Query failed:" << query.lastQuery();
        qDebug() << "[ERROR] Error:" << query.lastError().text();
    }else{
        qDebug() << "[INFO] Query succeeded:" << query.lastQuery();
    }
    return success;
}

void DataBase::clearDataBase(){
    auto db = getThreadDatabase();
    QSqlQuery query(db), query2(db);
    query.prepare("DROP TABLE IF EXISTS messages;");
    query2.prepare("DROP TABLE IF EXISTS message_status;");

    executeQuery(query);
    executeQuery(query2);
}

bool DataBase::initialDb() {
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query1(db), query2(db);

    query1.prepare(R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            chat_id INT,
            sender_id INT,
            text TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )");

    query2.prepare(R"(
        CREATE TABLE IF NOT EXISTS message_status (
            message_id INT,
            receiver_id INT,
            delivered BOOLEAN DEFAULT 0,
            read BOOLEAN DEFAULT 0,
            PRIMARY KEY (message_id, receiver_id)
        );
    )");

    return executeQuery(query1) && executeQuery(query2);
}

OptionalMessageId DataBase::addMsgToDatabase(const std::string& messageText, int fromUserId, int chatId) {
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare(R"(INSERT INTO messages (chat_id, sender_id, text, timestamp) VALUES (?, ?, ?, CURRENT_TIMESTAMP))");

    if (!executeQuery(query, chatId, fromUserId, QString::fromStdString(messageText))) {
        return std::nullopt;
    }

    return query.lastInsertId().toInt();
}

QList<Message> DataBase::getChatMessages(int chatId) {
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare(R"(SELECT id, chat_id, sender_id, text, timestamp
                 FROM messages
                 WHERE chat_id = ?
                 ORDER BY timestamp ASC)");

    if (!executeQuery(query, chatId)) {
        return {};
    }

    auto messages = QList<Message>{};
    while (query.next()) {
        auto msg = getMessageFromQuery(query);
        messages.append(msg);
    }

    qDebug() << "[INFO] getChatMessages:" << messages.size() << "messages found";
    return messages;
}

void DataBase::markDelivered(int msgId) {
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare(R"(UPDATE message_status SET delivered = 1
                                        WHERE message_id = ? AND delivered = 0)");

    executeQuery(query, msgId);
}

void DataBase::saveMessage(int msgId, int senderId, int receiverId, const std::string& text, bool delivered) {
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare(R"(INSERT INTO message_status (message_id, receiver_id, delivered, read) VALUES (?, ?, ?, 0))"); // u don't save sender_id

    executeQuery(query, msgId, senderId, (int)delivered);
}

QList<Message> DataBase::getUndeliveredMessages(int userId) {
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare(R"(SELECT m.id, m.chat_id, m.sender_id, ms.receiver_id, m.text, m.timestamp
                    FROM messages m
                    JOIN message_status ms ON m.id = ms.message_id
                    WHERE ms.receiver_id = ? AND ms.delivered = 0)");

    if (!executeQuery(query, userId)) {
        return {};
    }

    auto messages = QList<Message>{};
    while (query.next()) {
        auto msg = getMessageFromQuery(query);
        messages.append(msg);
    }

    qDebug() << "[INFO] getUndeliveredMessages found:" << messages.size();
    return messages;
}

OptinonalMessage DataBase::saveSendedMessage(int chatId, int sender_id, std::string text){
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare(R"(INSERT INTO message (chat_id, sender_id, text) VALUES (?, ?, ?))");

    if (!executeQuery(query, chatId, sender_id, QString::fromStdString(text))) {
        return std::nullopt;
    }

    int msgId = query.lastInsertId().toInt();

    return Message{
        .id = msgId,
        .senderId = sender_id,
        .timestamp = QDateTime::currentDateTime().toString(Qt::ISODate),
        .text = text
    };
}
