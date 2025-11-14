#include "SQLiteDataBase.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QThread>

#include "Debug_profiling.h"

namespace {
const QString CREATE_USERS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT UNIQUE NOT NULL,
            tag TEXT NOT NULL,
            username TEXT NOT NULL
        );
    )";

const QString CREATE_MESSAGES_TABLE = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            chat_id INT,
            sender_id INT,
            text TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            local_id TEXT
        );
    )";

const QString CREATE_MESSAGES_STATUS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS messages_status (
            message_id INT,
            receiver_id INTEGER,
            is_read BOOLEAN,
            read_at INTEGER,
            PRIMARY KEY(message_id, receiver_id)
        );
    )";

const QString CREATE_CHATS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS chats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            is_group INTEGER NOT NULL DEFAULT 0,
            name TEXT,
            avatar TEXT,
            created_at INTEGER
        );
    )";

const QString CREATE_CHAT_MEMBERS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS chat_members (
            chat_id INTEGER,
            user_id INTEGER,
            status TEXT DEFAULT 'member',
            added_at INTEGER,
            PRIMARY KEY(chat_id, user_id)
        );
    )";

const QString CREATE_CREDENTIALS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS credentials (
            user_id INTEGER PRIMARY KEY,
            hash_password TEXT NOT NULL
        );
    )";
} // namespace

SQLiteDatabase::SQLiteDatabase(const QString& db_path)
    : IDataBase(db_path) {
  initializeSchema();
}

void SQLiteDatabase::initializeSchema() {
  QSqlDatabase db = getThreadDatabase();

  const std::vector<std::pair<QString, QString>> tables = {
                                                           {"users", CREATE_USERS_TABLE},
                                                           {"messages", CREATE_MESSAGES_TABLE},
                                                           {"messages_status", CREATE_MESSAGES_STATUS_TABLE},
                                                           {"chats", CREATE_CHATS_TABLE},
                                                           {"chat_members", CREATE_CHAT_MEMBERS_TABLE},
                                                           {"credentials", CREATE_CREDENTIALS_TABLE},
                                                           };

  for (const auto& [name, sql] : tables) {
    if (!executeSql(db, sql)) {
      LOG_ERROR("Failed to create table '{}'", name.toStdString());
    }
  }

  LOG_INFO("Database schema initialized successfully");
}

bool SQLiteDatabase::executeSql(QSqlDatabase& db, const QString& sql) {
  QSqlQuery query(db);
  if (!query.exec(sql)) {
    LOG_ERROR("SQL error: {}", query.lastError().text().toStdString());
    return false;
  }
  return true;
}

bool SQLiteDatabase::deleteTable(QSqlDatabase& db, const QString& name) {
  const QString sql = QString("DROP TABLE IF EXISTS \"%1\"").arg(name);
  return executeSql(db, sql);
}

bool SQLiteDatabase::tableExists(QSqlDatabase& db, const QString& tableName) {
  QSqlQuery query(db);
  query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=?;");
  query.addBindValue(tableName);
  if (!query.exec()) return false;
  return query.next();
}
