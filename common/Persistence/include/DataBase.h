#ifndef PROSTESQLDATABASE_H
#define PROSTESQLDATABASE_H

#include "interfaces/IDataBase.h"

class ISqlExecutor;

class ProsteSQLDatabase : public IDataBase {
 public:
  explicit ProsteSQLDatabase(const QString& db_path = "messenger");

 protected:
  bool tableExists(QSqlDatabase& db, const QString& tableName);
  bool executeSql(QSqlDatabase& db, const QString& sql);
  bool deleteTable(QSqlDatabase& db, const QString& name);
  void initializeSchema();
};

#endif  // PROSTESQLDATABASE_H
