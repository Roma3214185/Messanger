#include "SQLiteDataBase.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/qsqlquery.h>
#include <qthread.h>
#include <QtSql/QSqlError>

#include "Debug_profiling.h"

void SQLiteDatabase::initializeSchema() {
  QSqlDatabase database = getThreadDatabase();
  createUserTable(database);
  createMessageStatusTable(database);
  createMessageTable(database);
  createChatTable(database);
  createChatMemberTable(database);
  createUserCredentialsTable(database);
  LOG_INFO("Database schema initialized successfully");
}

void SQLiteDatabase::createUserTable(QSqlDatabase& database) {
  QSqlQuery query(database);
  //deleteTable(database, "users");

  if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                email TEXT UNIQUE NOT NULL,
                tag TEXT NOT NULL,
                username TEXT NOT NULL
            );
        )")) {
    LOG_ERROR("Failed to create users table: {}",
              query.lastError().text().toStdString());
  }
}

void SQLiteDatabase::deleteTable(QSqlDatabase& database, const QString& name) {
  QSqlQuery query(database);

  QString sql = QString("DROP TABLE IF EXISTS \"%1\"").arg(name);
  if (!query.exec(sql)) {
    qDebug() << "Error deleting table" << name << ":" << query.lastError().text();
  } else {
    qDebug() << "Deleted table" << name;
  }
}

void SQLiteDatabase::createMessageStatusTable(QSqlDatabase& database) {
  QSqlQuery query(database);
  //deleteTable(database, "messages_status");

  if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS messages_status (
                message_id INT,
                receiver_id INTEGER,
                is_read BOOLEAN,
                read_at INTEGER,
                PRIMARY KEY(message_id, receiver_id)
            );
        )")) {
    LOG_ERROR("Failed to create messages_status table: {}",
              query.lastError().text().toStdString());
  }
}

void SQLiteDatabase::createChatTable(QSqlDatabase& database){
  QSqlQuery query(database);
  //deleteTable(database, "chats");

  query.prepare(R"(
        CREATE TABLE IF NOT EXISTS chats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            is_group INTEGER NOT NULL DEFAULT 0,
            name TEXT,
            avatar TEXT,
            created_at INTEGER
        );
    )");

  if(!query.exec()){
    LOG_ERROR("Error creating chats {}", query.lastError().text().toStdString());
  }
}

void SQLiteDatabase::createChatMemberTable(QSqlDatabase& database){
  QSqlQuery query(database);
  //deleteTable(database, "chat_members");

  query.prepare(R"(
        CREATE TABLE IF NOT EXISTS chat_members (
            chat_id INTEGER,
            user_id INTEGER,
            status TEXT DEFAULT 'member',
            added_at INTEGER,
            PRIMARY KEY(chat_id, user_id)
        );
    )");

  if(!query.exec()) {
    LOG_ERROR("Error creating chat_members", query.lastError().text().toStdString());
  }
}

void SQLiteDatabase::createUserCredentialsTable(QSqlDatabase& database){
  QSqlQuery query(database);
  //deleteTable(database, "credentials");

  query.prepare(R"(
        CREATE TABLE IF NOT EXISTS credentials (
            user_id INTEGER PRIMARY KEY,
            hash_password TEXT NOT NULL
        );
    )");

  if(!query.exec()) {
    LOG_ERROR("Error creating credentials", query.lastError().text().toStdString());
  }
}

void SQLiteDatabase::createMessageTable(QSqlDatabase& database) {
  QSqlQuery query(database);
  //deleteTable(database, "messages");

  if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                chat_id INT,
                sender_id INT,
                text TEXT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                local_id TEXT
            );
        )")) {
    LOG_ERROR("Failed to create messages_status table: {}",
              query.lastError().text().toStdString());
  }
}

SQLiteDatabase::SQLiteDatabase(const QString& db_path) : IDataBase(db_path) {
  initializeSchema();
}
