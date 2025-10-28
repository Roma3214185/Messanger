#include "SQLiteDataBase.h"

#include <QtSql/QSqlDatabase>

void SQLiteDatabase::initializeSchema() {
  QSqlDatabase database = getThreadDatabase();
  createUserTable(database);
  createMessageStatusTable(database);
  createMessageTable(database);
  createChatTable(database);
  LOG_INFO("Database schema initialized successfully");
}

void SQLiteDatabase::createChatTable(QSqlDatabase& database) {
  QSqlQuery query(database);
  if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS chats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                is_group BOOLEAN NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )")) {
    LOG_ERROR("Failed to create chats table: {}",
              query.lastError().text().toStdString());
  }
}

void SQLiteDatabase::createUserTable(QSqlDatabase& database) {
  QSqlQuery query(database);

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

void SQLiteDatabase::createMessageStatusTable(QSqlDatabase& database) {
  QSqlQuery query(database);

  if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS messages_status (
                id INT,
                receiver_id INTEGER,
                is_read BOOLEAN,
                read_at INTEGER,
                PRIMARY KEY(id, receiver_id)
            );
        )")) {
    LOG_ERROR("Failed to create messages_status table: {}",
              query.lastError().text().toStdString());
  }
}

void SQLiteDatabase::createMessageTable(QSqlDatabase& database) {
  QSqlQuery query(database);
  if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS messages_status (
                id INT,
                receiver_id INTEGER,
                is_read BOOLEAN,
                read_at INTEGER,
                PRIMARY KEY(id, receiver_id)
            );
        )")) {
    LOG_ERROR("Failed to create messages_status table: {}",
              query.lastError().text().toStdString());
  }
}

QSqlDatabase& SQLiteDatabase::getThreadDatabase() {
  PROFILE_SCOPE("QSqlDatabase::getThreadDatabase");
  thread_local QSqlDatabase db;
  thread_local bool initialized = false;

  if (!initialized) {
    static std::atomic<int> connCounter{0};

    QString connection_name =
        QString("conn_%1_%2")
            .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()))
            .arg(connCounter++);

    db = QSqlDatabase::addDatabase("QSQLITE", connection_name);
    db.setDatabaseName(db_path_);

    if (!db.open()) {
      throw std::runtime_error("Cannot open database for this thread");
    }

    QObject::connect(QThread::currentThread(), &QThread::finished,
                     [connection_name]() -> void {
                       QSqlDatabase::removeDatabase(connection_name);
                     });

    initialized = true;
  }

  return db;
}

SQLiteDatabase::SQLiteDatabase(const QString& db_path) : db_path_(db_path) {
  initializeSchema();
}
