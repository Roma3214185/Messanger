#include "database.h"

#include <QDebug>
#include <QThread>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <optional>

QSqlDatabase DataBase::getThreadDatabase() {
  const QString connName =
      QString("connection_%1").arg((quintptr)QThread::currentThreadId());

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
                  << QThread::currentThread() << db.lastError().text();

      throw std::runtime_error("Cannot open database");
    }
  }

  return db;
}

template <typename... Args>
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
  } else {
    qDebug() << "[INFO] Query succeeded:" << query.lastQuery();
  }
  return success;
}

Chat DataBase::getChatFromQuery(QSqlQuery& query, int chat_id) {
  return Chat{.id = chat_id,
              .is_group = query.value("is_group").toInt() == 1,
              .name = query.value("name").toString().toStdString(),
              .avatar = query.value("avatar").toString().toStdString()};
}

void DataBase::clearDataBase() {
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

bool DataBase::addMembersToChat(int chat_id, const std::vector<int>& members_id) {
  QSqlDatabase db = getThreadDatabase();

  for (int user_id : members_id) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO chat_members (chat_id, user_id) VALUES (?, ?)");

    if (!executeQuery(query, chat_id, user_id)) {
      return false;
    }
  }

  return true;
}

bool DataBase::deleteChat(int chat_id) {
  QSqlDatabase db = getThreadDatabase();
  QSqlQuery query(db), query2(db);
  query.prepare("DELETE FROM chat_members WHERE chat_id = ?");

  if (!executeQuery(query, chat_id)) {
    return false;
  }

  query2.prepare("DELETE FROM chats WHERE id = ?");
  return executeQuery(query2, chat_id);
}

bool DataBase::deleteMembersFromChat(int chat_id,
                                     const std::vector<int>& members_id) {
  QSqlDatabase db = getThreadDatabase();

  for (int user_id : members_id) {
    QSqlQuery query(db);
    query.prepare(
        "DELETE FROM chat_members WHERE chat_id = :chat_id AND user_id = "
        ":user_id");
    if (!executeQuery(query, chat_id, user_id)) {
      return false;
    }
  }

  return true;
}

bool DataBase::initialDb() {
  QSqlDatabase db = getThreadDatabase();

  QSqlQuery query1(db), query2(db);
  query1.prepare(R"(
        CREATE TABLE IF NOT EXISTS chats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            is_group INTEGER NOT NULL,
            name TEXT,
            avatar TEXT,
            created_at INTEGER
        );
    )");

  query2.prepare(R"(
        CREATE TABLE IF NOT EXISTS chat_members (
            chat_id INTEGER,
            user_id INTEGER,
            status TEXT DEFAULT 'member',
            added_at INTEGER
        );
    )");

  if (!executeQuery(query1) || !executeQuery(query2)) {
    return false;
  }

  return true;
}

std::optional<QList<int>> DataBase::getMembersOfChat(int chatId) {
  QSqlDatabase db = getThreadDatabase();
  QSqlQuery query(db);
  query.prepare("SELECT user_id FROM chat_members WHERE chat_id=?");

  if (!executeQuery(query, chatId)) {
    return std::nullopt;
  }

  QList<int> membersId;
  while (query.next()) {
    int id = query.value("user_id").toInt();
    membersId.append(id);
  }

  return membersId;
}

QList<Chat> DataBase::getChatsOfUser(int user_id) {
  QSqlDatabase db = getThreadDatabase();
  QSqlQuery query(db);
  query.prepare("SELECT chat_id FROM chat_members WHERE user_id=?");
  if (!executeQuery(query, user_id)) {
    return {};
  }

  QList<int> chats_id;
  while (query.next()) {
    chats_id.append(query.value(0).toInt());
  }

  QList<Chat> chats;
  for (int chat_id : chats_id) {
    QSqlQuery query2(db);
    query2.prepare("SELECT is_group, name, avatar FROM chats WHERE id=?");

    if (!executeQuery(query2, chat_id)) {
      continue;
    }

    if (query2.next()) {
      Chat chat = getChatFromQuery(query2, chat_id);
      chats.append(chat);
      qDebug() << "[INFO] Loaded chat id=" << chat_id
               << " isGroup=" << chat.is_group;
    }
  }

  return chats;
}

OptionalChat DataBase::getChatById(int chat_id) {
  QSqlDatabase db = getThreadDatabase();
  QSqlQuery query2(db);
  query2.prepare("SELECT is_group, name, avatar FROM chats WHERE id=?");

  if (!executeQuery(query2, chat_id)) {
    return std::nullopt;
  }

  auto chat = getChatFromQuery(query2, chat_id);
  return chat;
}

int DataBase::getMembersCount(int chat_id) {
  auto list = getMembersOfChat(chat_id);

  if (!list) {
    return 0;
  }

  return list->size();
}

OptionalUserId DataBase::getOtherMemberId(int chat_id, int user_id) {
  auto list = getMembersOfChat(chat_id);
  if (!list) return std::nullopt;

  for (auto id : *list) {
    if (id != user_id) {
      return id;
    }
  }

  qDebug() << "[ERROR] Not found another member for chat: " << chat_id;
  return std::nullopt;
}
