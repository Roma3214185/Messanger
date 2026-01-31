#include "SQLiteDataBase.h"

#include <QThread>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include "Debug_profiling.h"

namespace {
const QString CREATE_USERS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            tag TEXT NOT NULL
        );
    )";

const QString CREATE_USERS_BY_EMAIL_TABLE = R"(
        CREATE TABLE IF NOT EXISTS users_by_email (
            id INTEGER PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            tag TEXT NOT NULL
        );
    )";

const QString CREATE_USERS_BY_EMAIL_INDEX = R"(
    CREATE INDEX IF NOT EXISTS idx_user_email ON users_by_email(email);
)";

const QString CREATE_USERS_BY_TAG_TABLE = R"(
        CREATE TABLE IF NOT EXISTS users_by_tag (
            id INTEGER PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            tag TEXT NOT NULL
        );
    )";

const QString CREATE_USERS_BY_TAG_INDEX = R"(
    CREATE INDEX IF NOT EXISTS idx_user_tag ON users_by_tag(tag);
)";

const QString CREATE_MESSAGES_TABLE = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY,
            chat_id INT,
            sender_id INT,
            text TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            local_id TEXT,
            answer_on INT NULL
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

const QString CREATE_MESSAGES_REACTION_TABLE = R"(
        CREATE TABLE IF NOT EXISTS messages_reaction (
            message_id INTEGER,
            receiver_id INTEGER,
            reaction_id INTEGER,
            PRIMARY KEY(message_id, receiver_id)
        );
    )";

const QString CREATE_MESSAGES_REACTION_INFO_TABLE = R"(
        CREATE TABLE IF NOT EXISTS messages_reaction_info (
            id INTEGER,
            image TEXT NOT NULL,
            PRIMARY KEY(id)
        );
    )";

const QString CREATE_CHATS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS chats (
            id INTEGER PRIMARY KEY,
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

// TODO: commit when insert private_chat and in table chat
const QString CREATE_PRIVATE_CHATS_TABLE =
    R"(CREATE TABLE IF NOT EXISTS private_chats (
    chat_id       INTEGER PRIMARY KEY,
    user1_id      INTEGER NOT NULL,
    user2_id      INTEGER NOT NULL,

    user_min      INTEGER GENERATED ALWAYS AS (CASE WHEN user1_id < user2_id THEN user1_id ELSE user2_id END) VIRTUAL,
    user_max      INTEGER GENERATED ALWAYS AS (CASE WHEN user1_id > user2_id THEN user1_id ELSE user2_id END) VIRTUAL,

    CONSTRAINT fk_user1 FOREIGN KEY (user1_id) REFERENCES users(id) ON DELETE CASCADE,
    CONSTRAINT fk_user2 FOREIGN KEY (user2_id) REFERENCES users(id) ON DELETE CASCADE,

    CONSTRAINT unique_private_chat UNIQUE (user_min, user_max)
    );
  )";

const QString CREATE_OUTBOX_TABLE = R"(CREATE TABLE IF NOT EXISTS outbox (
      id                INTEGER PRIMARY KEY AUTOINCREMENT,
      table_trigered    TEXT NOT NULL,
      payload           TEXT NOT NULL,           -- json string
      processed         INTEGER NOT NULL DEFAULT 0  -- 0 = not processed, 1 = processed
    );
  )";

}  // namespace

bool SQLiteDatabase::initializeSchema() {
  const std::vector<QString> tables = {CREATE_USERS_TABLE,
                                       CREATE_MESSAGES_TABLE,
                                       CREATE_MESSAGES_STATUS_TABLE,
                                       CREATE_CHATS_TABLE,
                                       CREATE_CHAT_MEMBERS_TABLE,
                                       CREATE_CREDENTIALS_TABLE,
                                       CREATE_PRIVATE_CHATS_TABLE,
                                       CREATE_OUTBOX_TABLE,
                                       CREATE_USERS_BY_EMAIL_TABLE,
                                       CREATE_USERS_BY_EMAIL_INDEX,
                                       CREATE_MESSAGES_REACTION_TABLE,
                                       CREATE_MESSAGES_REACTION_INFO_TABLE};

  for (const auto &sql : tables) {
    if (!executeSql(db(), sql)) {
      return false;
    }
  }

  LOG_INFO("Database schema initialized successfully");
  return true;
}

bool SQLiteDatabase::executeSql(const QSqlDatabase &db, const QString &sql) {
  QSqlQuery query(db);
  if (!query.exec(sql)) {
    LOG_ERROR("SQL error: {}", query.lastError().text().toStdString());
    return false;
  }
  return true;
}

bool SQLiteDatabase::deleteTable(const QString &name) {
  const QString sql = QString("DROP TABLE IF EXISTS \"%1\"").arg(name);
  bool res = executeSql(db(), sql);
  if (res) LOG_WARN("Deleted table: {}", name.toStdString());
  return res;
}

bool SQLiteDatabase::tableExists(const QString &table_name) {
  QSqlQuery query(db());
  query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=?;");
  query.addBindValue(table_name);
  if (!query.exec()) return false;
  return query.next();
}

QSqlDatabase SQLiteDatabase::db() const {
  const QString conn = QString("%1_%2").arg(db_name_).arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

  if (!QSqlDatabase::contains(conn)) {
    auto db = QSqlDatabase::addDatabase("QSQLITE", conn);
    db.setDatabaseName(db_name_);
    if (!db.open()) {
      qFatal("Failed to open DB");
    }
    return db;
  }

  return QSqlDatabase::database(conn);
}

bool SQLiteDatabase::exec(const QString &sql) {
  QSqlQuery q(db());
  LOG_INFO("To execute {}", sql.toStdString());
  if (!q.exec(sql)) {
    LOG_ERROR("For sql {} execute failed: {}", sql.toStdString(), q.lastError().text().toStdString());
    return false;
  }
  LOG_INFO("Execute succced, affected : {} rows", q.numRowsAffected());
  return true;
}

bool SQLiteDatabase::commit() { return db().commit(); }

std::unique_ptr<IQuery> SQLiteDatabase::prepare(const QString &sql) {
  auto query = std::make_unique<SQLiteQuery>(db());  // TODO: factory??
  if (!query->prepare(sql)) {
    LOG_ERROR("For sql {} prepare failed: {}", sql.toStdString(), query->error().toStdString());
    return nullptr;
  }
  return query;
}

std::unique_ptr<IQuery> SQLiteDatabase::prepare(const std::string &sql) { return prepare(QString::fromStdString(sql)); }

void SQLiteDatabase::rollback() { db().rollback(); }

bool SQLiteDatabase::transaction() { return db().transaction(); }
