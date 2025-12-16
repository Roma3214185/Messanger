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

#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>
#include <nlohmann/json.hpp>

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

    // Start the worker thread
    void startWorker() {
        if (!isRunning()) {
            start();
        }
    }

    // Signal the worker to stop
    void stop() {
        QMutexLocker locker(&m_mutex);
        m_stop = true;
        m_waitCondition.wakeAll();
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
                QMutexLocker locker(&m_mutex);
                if (m_stop) break;
            }

            processBatch(db_);

            QMutexLocker locker(&m_mutex);
            m_waitCondition.wait(&m_mutex, 10);
        }

        //db.close();
        //QSqlDatabase::removeDatabase("OutboxWorkerConnection");
    }

private:
    IDataBase& db_;
    volatile bool m_stop;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;

  void processBatch(IDataBase& db) {
    QString sql_command = "SELECT id, table_trigered, payload FROM outbox WHERE processed = 0 LIMIT 100;";
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

    QList<int> processedIds;

    while (query->next()) {
      //LOG_INFO("First candodate");
      QString tableTriggered = query->value("table_trigered").toString();
      QString payloadStr = query->value("payload").toString();
      int id = query->value("id").toInt();

      if (tableTriggered == "user_table") {
        LOG_INFO("Triggered user_table");
        //QSqlQuery query1(db);
        User user = nlohmann::json::parse(payloadStr.toStdString());
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

      processedIds.append(id);
    }

    if(!processedIds.empty()) LOG_INFO("Processed ids size: {}", processedIds.size());
    for (int id : processedIds) {
      const std::string command = "UPDATE outbox SET processed = 1 WHERE id = ?";
      auto markQuery = db.prepare(command);
      if(markQuery) {
        markQuery->bind(id);
        markQuery->exec();
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
