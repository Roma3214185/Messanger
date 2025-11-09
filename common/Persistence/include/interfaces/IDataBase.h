#ifndef IDATABASE_H
#define IDATABASE_H

#include <QString>
#include <QThread>
#include <QtSql/QSqlDatabase>

#include "Debug_profiling.h"

class IDataBase {
  QString db_path_;

 public:
  explicit IDataBase(const QString& db_path = "test_path.sqlite") : db_path_(db_path) {}

  QSqlDatabase& getThreadDatabase() {
    PROFILE_SCOPE("QSqlDatabase::getThreadDatabase");
    thread_local QSqlDatabase db;
    thread_local bool         initialized = false;

    if (!initialized) {
      static std::atomic<int> connCounter{0};

      QString connection_name = QString("conn_%1_%2")
                                    .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()))
                                    .arg(connCounter++);

      db = QSqlDatabase::addDatabase("QSQLITE", connection_name);
      db.setDatabaseName(db_path_);

      if (!db.open()) {
        throw std::runtime_error("Cannot open database for this thread");
      }

      QObject::connect(QThread::currentThread(), &QThread::finished, [connection_name]() -> void {
        QSqlDatabase::removeDatabase(connection_name);
      });

      initialized = true;
    }

    return db;
  }
  virtual ~IDataBase() = default;
};

#endif  // IDATABASE_H
