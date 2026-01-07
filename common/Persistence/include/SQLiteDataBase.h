#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include <QSqlDatabase>
#include <QThread>

#include "Debug_profiling.h"
#include "interfaces/IDataBase.h"
#include "query/SQLiteQuery.h"

class SQLiteDatabase : public IDataBase {
  QString db_name_;

public:
  explicit SQLiteDatabase(QString db_name) : db_name_(std::move(db_name)) {}

  [[nodiscard]] QSqlDatabase db() const {
    const QString conn = QString("%1_%2").arg(db_name_).arg(
        reinterpret_cast<quintptr>(QThread::currentThreadId()));

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

  [[nodiscard]] bool exec(const QString &sql) override {
    QSqlQuery q(db());
    LOG_INFO("To execute {}", sql.toStdString());
    if (!q.exec(sql)) {
      LOG_ERROR("For sql {} execute failed: {}", sql.toStdString(),
                q.lastError().text().toStdString());
      return false;
    }
    LOG_INFO("Execute succced, affected : {} rows", q.numRowsAffected());
    return true;
  }

  std::unique_ptr<IQuery> prepare(const std::string &sql) override {
    return prepare(QString::fromStdString(sql));
  }

  void rollback() override {}

  bool transaction() override { return db().transaction(); }

  std::unique_ptr<IQuery> prepare(const QString &sql) override {
    auto query = std::make_unique<SQLiteQuery>(db()); // TODO: factory??
    if (!query->prepare(sql)) {
      LOG_ERROR("For sql {} prepare failed: {}", sql.toStdString(),
                query->error());
      return nullptr;
    }
    return query;
  }

  bool commit() override { return db().commit(); }

  bool initializeSchema();

  bool tableExists(QSqlDatabase db, const QString &table_name);
  bool deleteTable(QSqlDatabase db, const QString &name);

protected:
  bool executeSql(QSqlDatabase db, const QString &sql);
};

#endif // SQLITEDATABASE_H
