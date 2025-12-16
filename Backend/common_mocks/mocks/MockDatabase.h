#ifndef MOCKDATABASE_H
#define MOCKDATABASE_H

#include "SQLiteDataBase.h"
#include "MockQuery.h"

class MockDatabase : public IDataBase {
 public:
  std::string last_execute_sql;
  bool exec_should_fail = false;
  bool should_fail_preare = false;
  bool commit_should_fail = false;
  bool transaction_should_fail = false;
  int cnt_rollback = 0;
  MockQuery mock_query;

   bool exec(const QString& sql) override {
     last_execute_sql = sql.toStdString();
     return !exec_should_fail;
   }

   std::unique_ptr<IQuery> prepare(const QString& sql) override {
     if(should_fail_preare) return nullptr;
     return std::make_unique<MockQuery>(mock_query);
   }

   std::unique_ptr<IQuery> prepare(const std::string& sql) override {
     return prepare(QString::fromStdString(sql));
   }

   bool commit() override {
      return !commit_should_fail;
   }

   void rollback() override {
     ++cnt_rollback;
   }

   bool transaction() override {
     return !transaction_should_fail;
   }
};

#endif  // MOCKDATABASE_H
