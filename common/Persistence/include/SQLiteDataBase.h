#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include "interfaces/IDataBase.h"
#include "SQLiteQuery.h"

#include <QSqlDatabase>

#include "Debug_profiling.h"

class SQLiteDatabase : public IDataBase {
    QSqlDatabase& db_;
 public:
  explicit SQLiteDatabase(QSqlDatabase& db_);

   bool exec(const QString& sql) override {
     QSqlQuery q(db_);
     if(!q.exec(sql)) {
        LOG_ERROR("For sql {} execute failed:", sql.toStdString(), q.lastError().text().toStdString());
       return false;
      }
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
     auto q = std::make_unique<SQLiteQuery>(db_); //TODO: factory??
     if (!q->prepare(sql)) {
       LOG_ERROR("For sql {} prepare failed:", sql.toStdString(), q->error());
       return nullptr;
     }
     return q;
   }

   bool commit() override {
     return db_.commit();
   }

  bool initializeSchema();

 protected:
  bool tableExists(QSqlDatabase& db, const QString& tableName);
  bool executeSql(QSqlDatabase& db, const QString& sql);
  bool deleteTable(QSqlDatabase& db, const QString& name);
};

#endif  // SQLITEDATABASE_H
