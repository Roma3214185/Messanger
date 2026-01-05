#ifndef MOCKDATABASE_H
#define MOCKDATABASE_H

#include "MockQuery.h"
#include "SQLiteDataBase.h"

class MockDatabase : public IDataBase {
 public:
  std::string last_execute_sql;
  bool        exec_should_fail        = false;
  bool        should_fail_preare      = false;
  bool        commit_should_fail      = false;
  bool        transaction_should_fail = false;
  int         cnt_rollback            = 0;
  MockQuery   mock_query;
  std::string last_prepared_sql;

  bool exec(const QString& sql) override {
    last_execute_sql = sql.toStdString();
    return !exec_should_fail;
  }

  std::unique_ptr<IQuery> prepare(const QString& sql) override {
    return prepare(sql.toStdString());
  }

  std::unique_ptr<IQuery> prepare(const std::string& sql) override {
    last_prepared_sql = sql;
    if (should_fail_preare) return nullptr;
    return std::make_unique<MockQuery>(mock_query);
  }

  bool commit() override { return !commit_should_fail; }

  void rollback() override { ++cnt_rollback; }

  bool transaction() override { return !transaction_should_fail; }
};

#endif  // MOCKDATABASE_H
