#include "database.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <optional>
#include <QThread>

// Функція для отримання поточного підключення бази в будь-якому потоці
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
    qDebug() << "[INFO] I will dalate chats and chat_members";

    QSqlQuery query(db);
    query.prepare("DROP TABLE IF EXISTS chats;");

    if (!query.exec()) {
        qDebug() << "[ERROR] Failed to clearDataBase chats:" << query.lastError().text();
        return;
    }

    QSqlQuery query2(db);
    query2.prepare("DROP TABLE IF EXISTS chat_members;");

    if (!query2.exec()) {
        qDebug() << "[ERROR] Failed to clearDataBase chat_members:" << query.lastError().text();
    }
}

// Створюємо приватний чат
std::optional<int> DataBase::createPrivateChat() {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return std::nullopt;

    QSqlQuery query(db);
    query.prepare("INSERT INTO chats (is_group, name) VALUES (0, NULL);");

    if (!query.exec()) {
        qDebug() << "[ERROR] Failed to create chat:" << query.lastError().text();
        return std::nullopt;
    }

    int chatId = static_cast<int>(query.lastInsertId().toInt());
    return chatId;
}

// Додаємо користувачів у чат
bool DataBase::addMembersToChat(int chatId, const std::vector<int>& membersId) {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return false;

    for (int userId : membersId) {
        QSqlQuery query(db);
        query.prepare("INSERT INTO chat_members (chat_id, user_id) VALUES (:chat_id, :user_id)");
        query.bindValue(":chat_id", chatId);
        query.bindValue(":user_id", userId);

        if (!query.exec()) {
            qDebug() << "[ERROR] Failed to add user" << userId
                     << "to chat" << chatId << ":" << query.lastError().text();
            return false;
        }
    }

    return true;
}

// Видалення чату
bool DataBase::deleteChat(int chatId) {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("DELETE FROM chat_members WHERE chat_id = :chat_id");
    query.bindValue(":chat_id", chatId);
    if (!query.exec()) {
        qDebug() << "[ERROR] Failed to delete chat members:" << query.lastError().text();
        return false;
    }

    query.prepare("DELETE FROM chats WHERE id = :chat_id");
    query.bindValue(":chat_id", chatId);
    if (!query.exec()) {
        qDebug() << "[ERROR] Failed to delete chat:" << query.lastError().text();
        return false;
    }

    return true;
}

// Видалення окремих учасників чату
bool DataBase::deleteMembersFromChat(int chatId, const std::vector<int>& membersId) {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return false;

    for (int userId : membersId) {
        QSqlQuery query(db);
        query.prepare("DELETE FROM chat_members WHERE chat_id = :chat_id AND user_id = :user_id");
        query.bindValue(":chat_id", chatId);
        query.bindValue(":user_id", userId);

        if (!query.exec()) {
            qDebug() << "[ERROR] Failed to delete user" << userId
                     << "from chat" << chatId << ":" << query.lastError().text();
            return false;
        }
    }

    return true;
}

bool DataBase::initialDb(){
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) return false;

    QSqlQuery query1(db);
    query1.prepare(R"(
        CREATE TABLE IF NOT EXISTS chats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            is_group INTEGER NOT NULL,
            name TEXT,
            avatar TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )");

    if (!query1.exec()) {
        qDebug() << "[ERROR] Creating chats table:" << query1.lastError().text();
        return false;
    }

    QSqlQuery query2(db);
    query2.prepare(R"(
        CREATE TABLE IF NOT EXISTS chat_members (
            chat_id INTEGER,
            user_id INTEGER,
            status TEXT DEFAULT 'member',
            added_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )");

    if (!query2.exec()) {
        qDebug() << "[ERROR] Creating chat_members table:" << query2.lastError().text();
        return false;
    }

    qDebug() << "[INFO] Database initialized successfully in thread" << QThread::currentThreadId();
    return true;
}

std::optional<QList<int>> DataBase::getMembersOfChat(int chatId){
    qDebug() << "[INFO] getMembersOfChat" << chatId;
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) {
        qDebug() << "[INFO] getMembersOfChat bd isn't opened";
        return std::nullopt;
    }


    QSqlQuery query(db);
    query.prepare("SELECT user_id FROM chat_members WHERE chat_id=?");
    query.addBindValue(chatId);


    QList<int> membersId;
    if(!query.exec()){
        qDebug() << "[ERROR] ERROR AFTER EXECUTE getMembersOfChat query:" << query.lastError().text();
        return std::nullopt;
    }

    while(query.next()){
        int id = query.value("user_id").toInt();
        membersId.append(id);
        qDebug() << "[INFO] return id" << id;
    }
    return membersId;
}


QList<Chat> DataBase::getChatsOfUser(int userId) {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) {
        qDebug() << "[ERROR] getChatsOfUser: database not opened";
        return {};
    }

    QList<int> chatsId;
    {
        QSqlQuery query(db);
        query.prepare("SELECT chat_id FROM chat_members WHERE user_id=?");
        query.addBindValue(userId);

        if (!query.exec()) {
            qDebug() << "[ERROR] getChatsOfUser: failed to select chat_ids -" << query.lastError().text();
            return {};
        }

        while (query.next()) {
            chatsId.append(query.value(0).toInt());
        }
    }

    QList<Chat> chats;
    for (int chatId : chatsId) {
        QSqlQuery query2(db);
        query2.prepare("SELECT is_group, name, avatar FROM chats WHERE id=?");
        query2.addBindValue(chatId);

        if (!query2.exec()) {
            qDebug() << "[ERROR] getChatsOfUser: failed for chatId" << chatId << "-" << query2.lastError().text();
            continue;
        }

        if (query2.next()) {
            Chat chat;
            chat.id = chatId;
            chat.isGroup = query2.value("is_group").toInt() == 1;
            chat.name = query2.value("name").toString().toStdString();
            chat.avatar = query2.value("avatar").toString().toStdString();
            chats.append(chat);

            qDebug() << "[INFO] Loaded chat id=" << chatId << " isGroup=" << chat.isGroup;
        }
    }

    return chats;
}

std::optional<Chat> DataBase::getChatById(int chatId) {
    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) {
        qDebug() << "[ERROR] getChatsOfUser: database not opened";
        return {};
    }
        QSqlQuery query2(db);
        query2.prepare("SELECT is_group, name, avatar FROM chats WHERE id=?");
        query2.addBindValue(chatId);

        if (!query2.exec()) {
            qDebug() << "[ERROR] getChatsOfUser: failed for chatId" << chatId << "-" << query2.lastError().text();
            return std::nullopt;
        }

    Chat chat;
    chat.id = chatId;
        chat.isGroup = query2.value("is_group").toInt() == 1;
        chat.name = query2.value("name").toString().toStdString();
         chat.avatar = query2.value("avatar").toString().toStdString();
        qDebug() << "[INFO] Loaded chat id=" << chatId << " isGroup=" << chat.isGroup;
            return chat;

}


int DataBase::getMembersCount(int chat_id){
    qDebug() << "[INFO] getMembersCount id" << chat_id;
    auto list = getMembersOfChat(chat_id);



    if(!list) {
        qDebug() << "[INFO] getMembersCount returned error";
        return 0;
    }
    qDebug() << "[INFO] getMembersCount returned " << list->size();
    return list->size();
}

std::optional<int> DataBase::getOtherMemberId(int chat_id, int userId){
    auto list = getMembersOfChat(chat_id);
    if(!list) return std::nullopt;

    for(auto id: *list) if(id != userId) return id;

    qDebug() << "[ERROR] Not found another member for chat: " << chat_id;
    return std::nullopt;

}

