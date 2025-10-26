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
    virtual QSqlDatabase& getThreadDatabase() = 0;
    virtual ~IDataBase() = default;
};


class SQLiteDatabase : public IDataBase {
public:
    explicit SQLiteDatabase(const QString& dbPath = "chat_db.sqlite")
        : dbPath(dbPath)
    {
        QSqlDatabase db = getThreadDatabase();
        initializeSchema(db);
    }

    ~SQLiteDatabase() {
        // auto connectionNames = QSqlDatabase::connectionNames();
        // for (const auto& name : connectionNames) {
        //     QSqlDatabase::removeDatabase(name);
        // }
    }

    // QSqlDatabase getThreadDatabase() {
    //     QString connectionName = QString("conn_%1_%2")
    //     .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()))
    //         .arg(reinterpret_cast<quintptr>(this));

    //     if (QSqlDatabase::contains(connectionName)) {
    //         QSqlDatabase db = QSqlDatabase::database(connectionName);
    //         if (!db.isOpen() && !db.open()) {
    //             throw std::runtime_error("Cannot open database for this thread");
    //         }
    //         return db;
    //     }

    //     // Create connection in THIS thread
    //     QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    //     db.setDatabaseName(dbPath);
    //     if (!db.open()) {
    //         throw std::runtime_error("Cannot open database for this thread");
    //     }

    //     return db;
    // }

    QSqlDatabase& getThreadDatabase() override {
        PROFILE_SCOPE("QSqlDatabase::getThreadDatabase");
        thread_local QSqlDatabase db;
        thread_local bool initialized = false;

        if (!initialized) {
            static std::atomic<int> connCounter{0};

            QString connectionName = QString("conn_%1_%2")
                                         .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()))
                                         .arg(connCounter++);

            db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            db.setDatabaseName(dbPath);

            if (!db.open()) {
                throw std::runtime_error("Cannot open database for this thread");
            }

            QObject::connect(
                QThread::currentThread(),
                &QThread::finished,
                [connectionName]() {
                    QSqlDatabase::removeDatabase(connectionName);
                }
                );

            initialized = true;
        }

        return db;
    }

private:
    void initializeSchema(QSqlDatabase& db) {
        QSqlQuery query(db);

        // Users table
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                email TEXT UNIQUE NOT NULL,
                tag TEXT NOT NULL,
                username TEXT NOT NULL
            );
        )")) {
            LOG_ERROR("Failed to create users table: {}", query.lastError().text().toStdString());
        }

        // Messages table
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                chat_id INTEGER,
                sender_id INTEGER,
                text TEXT,
                timestamp INTEGER NOT NULL,
                FOREIGN KEY(chat_id) REFERENCES chats(id),
                FOREIGN KEY(sender_id) REFERENCES users(id)
            );
        )")) {
            LOG_ERROR("Failed to create messages table: {}", query.lastError().text().toStdString());
        }

        // Messages status table
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS messages_status (
                id INT,
                receiver_id INTEGER,
                is_read BOOLEAN,
                read_at INTEGER,
                PRIMARY KEY(id, receiver_id)
            );
        )")) {
            LOG_ERROR("Failed to create messages_status table: {}", query.lastError().text().toStdString());
        }

        // Optional: Chats table
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS chats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                is_group BOOLEAN NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )")) {
            LOG_ERROR("Failed to create chats table: {}", query.lastError().text().toStdString());
        }

        LOG_INFO("Database schema initialized successfully");
    }

private:
    QString dbPath;
};



#endif // SQLITEDATABASE_H
