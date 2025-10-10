#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QThread>
#include <QDebug>
#include <optional>
#include "messagedatabase.h"

QSqlDatabase getThreadDatabase() {
    const QString connName = QString("connection_%1").arg((quintptr)QThread::currentThreadId());

    QSqlDatabase db;
    if (QSqlDatabase::contains(connName)) {
        db = QSqlDatabase::database(connName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName("chat_db");
        if (!db.open()) {
            qDebug() << "[ERROR] Cannot open DB in thread:" << QThread::currentThread()
            << db.lastError().text();
        }
    }
    return db;
}

void DataBase::clearDataBase(){
    auto db = getThreadDatabase();
    if (!db.isOpen()) return;
    qDebug() << "[INFO] I will delete messages and message_status";

    QSqlQuery query(db);
    query.prepare("DROP TABLE IF EXISTS messages;");

    if (!query.exec()) {
        qDebug() << "[ERROR] Failed to clearDataBase messages:" << query.lastError().text();
        return;
    }

    QSqlQuery query2(db);
    query2.prepare("DROP TABLE IF EXISTS message_status;");

    if (!query.exec()) {
        qDebug() << "[ERROR] Failed to clearDataBase message_status:" << query.lastError().text();
        return;
    }
}

bool DataBase::initialDb() {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return false;

    QSqlQuery query1(db);
    QSqlQuery query2(db);

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

    if (!query1.exec()) {
        qDebug() << "[ERROR] Creating messages table:" << query1.lastError().text();
        return false;
    }

    if (!query2.exec()) {
        qDebug() << "[ERROR] Creating message_status table:" << query2.lastError().text();
        return false;
    }

    return true;
}

std::optional<int> DataBase::addMsgToDatabase(const std::string& messageText, int fromUserId, int chatId) {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return std::nullopt;

    QSqlQuery query(db);
    query.prepare(R"(INSERT INTO messages (chat_id, sender_id, text, timestamp) VALUES (?, ?, ?, CURRENT_TIMESTAMP))");
    query.addBindValue(chatId);
    query.addBindValue(fromUserId);
    query.addBindValue(QString::fromStdString(messageText));

    if (!query.exec()) {
        qDebug() << "[ERROR] Insert message failed:" << query.lastError().text();
        return std::nullopt;
    }

    return query.lastInsertId().toInt();
}

QList<Message> DataBase::getChatMessages(int chatId) {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return {};

    QSqlQuery query(db);
    query.prepare(R"(SELECT id, chat_id, sender_id, text, timestamp FROM messages WHERE chat_id = :chatId ORDER BY timestamp ASC)");
    query.bindValue(":chatId", chatId);

    QList<Message> messages;
    if (!query.exec()) {
        qDebug() << "[ERROR] getChatMessages failed:" << query.lastError().text();
        return {};
    }

    while (query.next()) {
        Message msg;
        msg.id = query.value("id").toInt();
        msg.chatId = query.value("chat_id").toInt();
        msg.senderId = query.value("sender_id").toInt();
        msg.text = query.value("text").toString().toStdString();
        QDateTime dt = query.value("timestamp").toDateTime();
        dt = dt.addSecs(3 * 3600);
        msg.timestamp = dt.toString(Qt::ISODate);
        qDebug() << "msg.id = " << msg.id << " and time = " << msg.timestamp;
        messages.append(msg);
    }

    qDebug() << "[INFO] getChatMessages:" << messages.size() << "messages found";
    return messages;
}

void DataBase::markDelivered(int msgId) {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare(R"(UPDATE message_status SET delivered = 1 WHERE message_id = :msgId AND delivered = 0)");
    query.bindValue(":msgId", msgId);

    if (!query.exec()) {
        qDebug() << "[ERROR] markDelivered failed:" << query.lastError().text();
    } else {
        qDebug() << "[INFO] markDelivered updated rows:" << query.numRowsAffected();
    }
}

void DataBase::saveMessage(int msgId, int fromUser, int toUser, const std::string& text, bool delivered) {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare(R"(INSERT INTO message_status (message_id, receiver_id, delivered, read) VALUES (?, ?, ?, 0))"); // u don't save sender_id
    query.addBindValue(msgId);
    query.addBindValue(toUser);
    query.addBindValue(delivered ? 1 : 0);

    if (!query.exec()) {
        qDebug() << "[ERROR] saveMessage failed:" << query.lastError().text();
        qDebug() << "Message id = " << msgId << "    and  receiver_id    =     " << toUser;
    } else {
        qDebug() << "[INFO] saveMessage added message_id:" << msgId;
    }
}

QList<Message> DataBase::getUndeliveredMessages(int userId) {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return {};

    QSqlQuery query(db);
    query.prepare(R"(SELECT m.id, m.chat_id, m.sender_id, ms.receiver_id, m.text, m.timestamp
                    FROM messages m
                    JOIN message_status ms ON m.id = ms.message_id
                    WHERE ms.receiver_id = :userId AND ms.delivered = 0)");
    query.bindValue(":userId", userId);

    QList<Message> messages;
    if (!query.exec()) {
        qDebug() << "[ERROR] getUndeliveredMessages failed:" << query.lastError().text();
        return {};
    }

    while (query.next()) {
        Message msg;
        msg.id = query.value("id").toInt();
        msg.chatId = query.value("chat_id").toInt();
        msg.senderId = query.value("sender_id").toInt();
        msg.receiverId = query.value("receiver_id").toInt();
        msg.text = query.value("text").toString().toStdString();
        msg.timestamp = query.value("timestamp").toString();
        messages.append(msg);
    }

    qDebug() << "[INFO] getUndeliveredMessages found:" << messages.size();
    return messages;
}

std::optional<Message> saveSendedMessage(int chatId, int sender_id, std::string text){
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return std::nullopt;

    QSqlQuery query(db);
    query.prepare(R"(INSERT INTO message (chat_id, sender_id, text) VALUES (?, ?, ?))");
    query.addBindValue(chatId);
    query.addBindValue(sender_id);
    query.addBindValue(QString::fromStdString(text));

    if (!query.exec()) {
        qDebug() << "[ERROR] saveSendedMessage failed:" << query.lastError().text();
        return std::nullopt;
    }

    int msgId = query.lastInsertId().toInt();
    qDebug() << "[INFO] saveSendedMessage added message_id:" << msgId;

    std::optional<Message> msg = Message{
        .id = msgId,
        .senderId = sender_id,
        .timestamp = QDateTime::currentDateTime().toString(Qt::ISODate),
        .text = text
    };
    return msg;

}
