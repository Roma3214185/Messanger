#ifndef OUTBOXWORKER_H
#define OUTBOXWORKER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QDebug>
#include <nlohmann/json.hpp>

#include "entities/User.h"
#include "Debug_profiling.h"

#include <QMutex>
#include <QWaitCondition>

#include "interfaces/IDataBase.h"

class OutboxWorker : public QThread {
    //Q_OBJECT
public:
    // OutboxWorker(IDataBase& db, QObject* parent = nullptr)
    //     : QThread(parent), db_(db), m_stop(false) {}
    explicit OutboxWorker(IDataBase& db) : db_(db) {}

    ~OutboxWorker() {
        stop();
        wait(); // wait for thread to finish
    }

    OutboxWorker(const OutboxWorker&) = delete;
    OutboxWorker(OutboxWorker&&) = delete;
    OutboxWorker& operator=(const OutboxWorker&) = delete;
    OutboxWorker& operator=(OutboxWorker&&) = delete;

    // Start the worker thread
    void startWorker() {
        if (!isRunning()) {
            start();
        }
    }

    // Signal the worker to stop
    void stop() {
        QMutexLocker locker(&mutex_);
        stop_ = true;
        wait_condition_.wakeAll();
    }

protected:
    void run() override {
        // Each thread needs its own DB connection in SQLite
        // QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "OutboxWorkerConnection");
        // db.setDatabaseName(m_dbPath);
        // auto sql_bd = db_.getThreadDatabase();
        // if (!sql_bd.open()) {
        //     qDebug() << "Failed to open DB in OutboxWorker:" << db_.lastError().text();
        //     return;
        // }

        while (true) {
            {
                QMutexLocker locker(&mutex_);
                if (stop_) break;
            }

            processBatch(db_);

            QMutexLocker locker(&mutex_);
            wait_condition_.wait(&mutex_, 10);
        }

        //db.close();
        //QSqlDatabase::removeDatabase("OutboxWorkerConnection");
    }

private:
    IDataBase& db_;
    volatile bool stop_{ false };
    QMutex mutex_;
    QWaitCondition wait_condition_;

  [[noreturn]]
  void processBatch(IDataBase& db) {
    const QString sql_command = "SELECT id, table_trigered, payload FROM outbox WHERE processed = 0 LIMIT 100;";
    auto query = db.prepare(sql_command);
    if(!query) return;
    //QSqlQuery query(db);

    // Begin transaction
    // if (!query.exec("BEGIN TRANSACTION;")) {
    //   qDebug() << "Failed to begin transaction:" << query.lastError().text();
    //   //QThread::msleep(10);
    //   return;
    // }

    // Select unprocessed events (limit 100)
    if (!query->exec()) {
      //query.exec("ROLLBACK;");
      //QThread::msleep(10);
      return;
    }

    QList<int> processed_ids;

    while (query->next()) {
      //LOG_INFO("First candodate");
      const QString table_triggered = query->value("table_trigered").toString();
      const QString payload_str = query->value("payload").toString();
      const int id = query->value("id").toInt();
      //todo: make handler for each ecent trigered
      if (table_triggered == "user_table") {
        LOG_INFO("Triggered user_table");
        //QSqlQuery query1(db);

        const User user = nlohmann::json::parse(payload_str.toStdString()); //todo: try catch
        LOG_INFO("User: {}", nlohmann::json(user).dump());

        const std::string first_command = "INSERT OR REPLACE INTO users_by_email VALUES(?, ?, ?, ?";
        auto query1 = db.prepare(first_command);
        if(!query1) continue;
        query1->bind(user.id);
        query1->bind(QString::fromStdString(user.email));
        query1->bind(QString::fromStdString(user.username));
        query1->bind(QString::fromStdString(user.username));

        const std::string second_command = "INSERT OR REPLACE INTO users_by_tag VALUES(?, ?, ?, ?";
        auto query2 = db.prepare(second_command);
        if(!query2) continue;
        query2->bind(user.id);
        query2->bind(QString::fromStdString(user.email));
        query2->bind(QString::fromStdString(user.username));
        query2->bind(QString::fromStdString(user.username));

        if (!query1->exec() || !query2->exec()) continue;
      }

      processed_ids.append(id);
    }

    if(!processed_ids.empty()) LOG_INFO("Processed ids size: {}", processed_ids.size());
    for (const int id : processed_ids) {
      const std::string command = "UPDATE outbox SET processed = 1 WHERE id = ?";
      auto mark_query = db.prepare(command);
      if(mark_query) {
        mark_query->bind(id);
        mark_query->exec();
      }
    }

    // if (!query.exec("COMMIT;")) {
    //   qDebug() << "Failed to commit transaction:" << query.lastError().text();
    //   query.exec("ROLLBACK;");
    // }

    //QThread::msleep(10);
  }
};

#endif // OUTBOXWORKER_H
