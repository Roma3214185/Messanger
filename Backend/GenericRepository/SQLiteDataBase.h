#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include <QString>
#include <QtSql/QSqlDatabase>
#include "../../DebugProfiling/Debug_profiling.h"
#include "QtSql/qsqlquery.h"
#include "qthread.h"
#include "QtSql/QSqlError"

class IDataBase {
public:
    virtual QSqlDatabase getThreadDatabase() = 0;
    virtual ~IDataBase() = default;
};

class SQLiteDatabase : public IDataBase {
public:
    explicit SQLiteDatabase(const QString& dbPath = "chat_db.sqlite")
        : dbPath(dbPath) {
        initializeSchema();
    }

    QSqlDatabase getThreadDatabase() override {
        const QString connName = QString("connection_%1").arg((quintptr)QThread::currentThreadId());
        QSqlDatabase db;
        if (QSqlDatabase::contains(connName))
            db = QSqlDatabase::database(connName);
        else {
            db = QSqlDatabase::addDatabase("QSQLITE", connName);
            db.setDatabaseName(dbPath);
        }
        if (!db.isOpen() && !db.open()){
            LOG_ERROR("Cann't open database");
            throw std::runtime_error("Cannot open database");
        }

        return db;
    }

    void intializeUserDb(){
        auto db = getThreadDatabase();
        QSqlQuery query(db);
        // if (!query.exec("DROP TABLE IF EXISTS users")) {
        //     qWarning() << "Failed to drop users table:" << query.lastError().text();
        // }

        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                email TEXT UNIQUE NOT NULL,
                tag TEXT NOT NULL,
                username TEXT NOT NULL
            );
        )")) {
            spdlog::warn("Failed to create users table:", query.lastError().text().toStdString());
            return;
        }
        LOG_INFO("Messages_status is initialized");
    }

    void intializeMessageDb(){
        auto db = getThreadDatabase();
        QSqlQuery query(db); //SELECT datetime(timestamp, 'unixepoch') FROM events;
        // if (!query.exec("DROP TABLE IF EXISTS messages")) {
        //     qWarning() << "Failed to drop users table:" << query.lastError().text();
        // }

        if(!query.exec(R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            chat_id INTEGER,
            sender_id INTEGER,
            text TEXT,
            timestamp INTEGER NOT NULL,
            FOREIGN KEY(chat_id) REFERENCES chats(id),
            FOREIGN KEY(sender_id) REFERENCES users(id)
            )
        )")){
            LOG_ERROR("Messages status isn't initialized: '{}'", query.lastError().text().toStdString());
            return;
        }

        query.exec(R"(CREATE INDEX IF NOT EXISTS idx_messages_chatid_time ON messages(chat_id, timestamp))");
        query.exec(R"(CREATE INDEX IF NOT EXISTS idx_messages_sender ON messages(sender_id))");
        LOG_INFO("Messages is initialized");
    }

    void intializeStatusDb(){
        auto db = getThreadDatabase();
        QSqlQuery query(db);
        // if (!query.exec("DROP TABLE IF EXISTS messages_status")) {
        //     qWarning() << "Failed to drop users table:" << query.lastError().text();
        // }

        if(!query.exec(R"(
            CREATE TABLE IF NOT EXISTS messages_status (
                id INT,
                receiver_id INTEGER,
                is_read BOOLEAN,
                read_at INTEGER,
                PRIMARY KEY(id, receiver_id)
            );
        )")){ //FOREIGN KEY(receiver_id) REFERENCES users(id),
            LOG_ERROR("Messages_status status isn't initialized: '{}'", query.lastError().text().toStdString());
        }
        LOG_INFO("Messages_status is initialized");
    }

    void initializeChatsDb(){
        auto db = getThreadDatabase();
        QSqlQuery query(db);

        if(!query.exec(R"(
            query.exec(R"(
             CREATE TABLE IF NOT EXISTS chats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                is_group BOOLEAN NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
             );
        )")){
            LOG_ERROR("Messages_status status isn't initialized: '{}'", query.lastError().text().toStdString());
        }
        LOG_INFO("Messages_status is initialized");

    }

    void initializeSchema() {
        intializeUserDb();
        intializeMessageDb();
        intializeStatusDb();
        //initializeChatsDb();
    }

private:
    QString dbPath;
};

#endif // SQLITEDATABASE_H
