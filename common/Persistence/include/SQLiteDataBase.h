#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include "interfaces/IDataBase.h"

class ISqlExecutor;

class SQLiteDatabase : public IDataBase {
 public:
  explicit SQLiteDatabase(const QString& db_path = "chat_db.sqlite");

 protected:
  bool tableExists(QSqlDatabase& db, const QString& tableName);
  bool executeSql(QSqlDatabase& db, const QString& sql);
  bool deleteTable(QSqlDatabase& db, const QString& name);
  void initializeSchema();
};

#endif  // SQLITEDATABASE_H
