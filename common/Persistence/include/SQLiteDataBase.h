#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include "interfaces/IDataBase.h"
#include "SQLiteQuery.h"

#include <QSqlDatabase>

#include "Debug_profiling.h"

class SQLiteDatabase : public IDataBase {
    QSqlDatabase& db_;
 public:
  explicit SQLiteDatabase(QSqlDatabase& db);

   bool exec(const QString& sql) override {
     QSqlQuery q(db_);
     LOG_INFO("To execute {}", sql.toStdString());
     if(!q.exec(sql)) {
        LOG_ERROR("For sql {} execute failed: {}", sql.toStdString(), q.lastError().text().toStdString());
       return false;
      }
     LOG_INFO("Execute succced, affected : {} rows", q.numRowsAffected());
     return true;
   }

   std::unique_ptr<IQuery> prepare(const std::string& sql) override {
     return prepare(QString::fromStdString(sql));
   }

   void rollback() override {

   }

   bool transaction() override {
     return db_.transaction();
   }

   std::unique_ptr<IQuery> prepare(const QString& sql) override {
     auto query = std::make_unique<SQLiteQuery>(db_); //TODO: factory??
     if (!query->prepare(sql)) {
       LOG_ERROR("For sql {} prepare failed: {}", sql.toStdString(), query->error());
       return nullptr;
     }
     return query;
   }

   bool commit() override {
     return db_.commit();
   }

  bool initializeSchema();

  bool tableExists(QSqlDatabase& db, const QString& table_name);
  bool deleteTable(QSqlDatabase& db, const QString& name);

 protected:
  bool executeSql(QSqlDatabase& db, const QString& sql);
};

#endif  // SQLITEDATABASE_H
