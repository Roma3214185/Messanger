#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include <QSqlDatabase>
#include <QThread>

#include "Debug_profiling.h"
#include "interfaces/IDataBase.h"
#include "query/SQLiteQuery.h"

class SQLiteDatabase : public IDataBase {

 public:
  explicit SQLiteDatabase(QString db_name) : db_name_(std::move(db_name)) {}

  [[nodiscard]] QSqlDatabase db() const;

  [[nodiscard]] bool exec(const QString &sql) override;

  std::unique_ptr<IQuery> prepare(const std::string &sql) override;

  void rollback() override;

  bool transaction() override;

  std::unique_ptr<IQuery> prepare(const QString &sql) override;

  bool commit() override;

  bool initializeSchema();

  bool tableExists(const QString &table_name);
  bool deleteTable(const QString &name);

 protected:
  bool executeSql(const QSqlDatabase &db, const QString &sql);

 private:
  QString db_name_;
};

#endif  // SQLITEDATABASE_H
