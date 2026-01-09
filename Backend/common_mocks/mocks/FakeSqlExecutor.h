#ifndef FAKESQLEXECUTOR_H
#define FAKESQLEXECUTOR_H

#include <QSqlField>
#include <QSqlRecord>
#include <QString>

#include "interfaces/ISqlExecutor.h"
#include "mocks/MockQuery.h"

class FakeSqlExecutor : public ISqlExecutor {
 public:
  QString lastSql = "";
  QList<QVariant> lastValues;
  bool shouldFail = false;
  int execute_calls = 0;

  std::vector<std::string> last_sqls;
  MockQuery mock_query;

  SqlExecutorResult execute(const QString &sql, const QList<QVariant> &values) override {
    ++execute_calls;
    lastSql = sql;
    lastValues = values;
    last_sqls.push_back(lastSql.toStdString());
    if (shouldFail) return SqlExecutorResult{.query = nullptr};
    return SqlExecutorResult{.query = std::make_unique<MockQuery>(mock_query)};
  }
};

#endif  // FAKESQLEXECUTOR_H
