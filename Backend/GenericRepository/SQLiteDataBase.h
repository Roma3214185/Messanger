#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include <QtSql/qsqlquery.h>
#include <qthread.h>
#include <QString>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

#include "Debug_profiling.h"

class IDataBase {
 public:
  virtual QSqlDatabase& getThreadDatabase() = 0;
  virtual ~IDataBase() = default;
};

class SQLiteDatabase : public IDataBase {
 public:
  explicit SQLiteDatabase(const QString& db_path = "chat_db.sqlite");
  QSqlDatabase& getThreadDatabase() override;

 private:
  void createUserTable(QSqlDatabase& db);
  void createMessageStatusTable(QSqlDatabase& db);
  void createMessageTable(QSqlDatabase& db);
  void createChatTable(QSqlDatabase& db);
  void createChatMemberTable(QSqlDatabase& database);
  void initializeSchema();

  QString db_path_;
};

#endif  // SQLITEDATABASE_H
