#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include "interfaces/IDataBase.h"

class SQLiteDatabase : public IDataBase {
 public:
  explicit SQLiteDatabase(const QString& db_path = "chat_db.sqlite");

 private:
  void createUserTable(QSqlDatabase& db);
  void createMessageStatusTable(QSqlDatabase& db);
  void createMessageTable(QSqlDatabase& db);
  void createChatTable(QSqlDatabase& db);
  void createChatMemberTable(QSqlDatabase& database);
  void createUserCredentialsTable(QSqlDatabase& database);
  void deleteTable(QSqlDatabase& database, const QString& name);
  void initializeSchema();
};

#endif  // SQLITEDATABASE_H
