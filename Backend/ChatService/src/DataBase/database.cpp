#include "database.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <optional>
#include <QThread>

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

Chat DataBase::getChatFromQuery(QSqlQuery& query, int chatId){
    return Chat{
        .id = chatId,
        .isGroup = query.value("is_group").toInt() == 1,
        .name = query.value("name").toString().toStdString(),
        .avatar = query.value("avatar").toString().toStdString()
    };

}

void DataBase::clearDataBase(){
    auto db = getThreadDatabase();

    QSqlQuery query(db), query2(db);
    query.prepare("DROP TABLE IF EXISTS chats;");
    query2.prepare("DROP TABLE IF EXISTS chat_members;");

    executeQuery(query);
    executeQuery(query2);
}

OptionalChatId DataBase::createPrivateChat() {
    QSqlDatabase db = getThreadDatabase();

    QSqlQuery query(db);
    query.prepare("INSERT INTO chats (is_group, name) VALUES (0, NULL);");

    if (!executeQuery(query)) {
        return std::nullopt;
    }

    int chatId = static_cast<int>(query.lastInsertId().toInt());
    return chatId;
}

bool DataBase::addMembersToChat(int chatId, const std::vector<int>& membersId) {
    QSqlDatabase db = getThreadDatabase();

    for (int userId : membersId) {
        QSqlQuery query(db);
        query.prepare("INSERT INTO chat_members (chat_id, user_id) VALUES (?, ?)");

        if (!executeQuery(query, chatId, userId)) {
            return false;
        }
    }

    return true;
}

bool DataBase::deleteChat(int chatId) {
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query(db), query2(db);
    query.prepare("DELETE FROM chat_members WHERE chat_id = ?");

    if (!executeQuery(query, chatId)) {
        return false;
    }

    query2.prepare("DELETE FROM chats WHERE id = ?");
    return executeQuery(query2, chatId);
}

bool DataBase::deleteMembersFromChat(int chatId, const std::vector<int>& membersId) {
    QSqlDatabase db = getThreadDatabase();

    for (int userId : membersId) {
        QSqlQuery query(db);
        query.prepare("DELETE FROM chat_members WHERE chat_id = :chat_id AND user_id = :user_id");
        if (!executeQuery(query, chatId, userId)) {
            return false;
        }
    }

    return true;
}

bool DataBase::initialDb(){
    QSqlDatabase db = getThreadDatabase();

    QSqlQuery query1(db), query2(db);
    query1.prepare(R"(
        CREATE TABLE IF NOT EXISTS chats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            is_group INTEGER NOT NULL,
            name TEXT,
            avatar TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )");

    query2.prepare(R"(
        CREATE TABLE IF NOT EXISTS chat_members (
            chat_id INTEGER,
            user_id INTEGER,
            status TEXT DEFAULT 'member',
            added_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )");

    if (!executeQuery(query1) || !executeQuery(query2)) {
        return false;
    }

    return true;
}

std::optional<QList<int>> DataBase::getMembersOfChat(int chatId){
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT user_id FROM chat_members WHERE chat_id=?");

    if(!executeQuery(query, chatId)){
        return std::nullopt;
    }

    QList<int> membersId;
    while(query.next()){
        int id = query.value("user_id").toInt();
        membersId.append(id);
    }

    return membersId;
}

QList<Chat> DataBase::getChatsOfUser(int userId) {
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT chat_id FROM chat_members WHERE user_id=?");
    if (!executeQuery(query, userId)) {
        return {};
    }

    QList<int> chatsId;
    while (query.next()) {
        chatsId.append(query.value(0).toInt());
    }

    QList<Chat> chats;
    for (int chatId : chatsId) {
        QSqlQuery query2(db);
        query2.prepare("SELECT is_group, name, avatar FROM chats WHERE id=?");

        if (!executeQuery(query2, chatId)) {
            continue;
        }

        if (query2.next()) {
            Chat chat = getChatFromQuery(query2, chatId);
            chats.append(chat);
            qDebug() << "[INFO] Loaded chat id=" << chatId << " isGroup=" << chat.isGroup;
        }
    }

    return chats;
}

OptionalChat DataBase::getChatById(int chatId) {
    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query2(db);
    query2.prepare("SELECT is_group, name, avatar FROM chats WHERE id=?");

    if (!executeQuery(query2, chatId)) {
        return std::nullopt;
    }

    auto chat = getChatFromQuery(query2, chatId);
    return chat;
}

int DataBase::getMembersCount(int chat_id){
    auto list = getMembersOfChat(chat_id);

    if(!list) {
        return 0;
    }

    return list->size();
}

OptionalUserId DataBase::getOtherMemberId(int chat_id, int userId){
    auto list = getMembersOfChat(chat_id);
    if(!list) return std::nullopt;

    for(auto id: *list){
        if(id != userId){
            return id;
        }
    }

    qDebug() << "[ERROR] Not found another member for chat: " << chat_id;
    return std::nullopt;
}
