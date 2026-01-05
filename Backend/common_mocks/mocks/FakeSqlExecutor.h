#ifndef FAKESQLEXECUTOR_H
#define FAKESQLEXECUTOR_H

#include <QSqlField>
#include <QSqlRecord>
#include <QString>

#include "interfaces/ISqlExecutor.h"
#include "mocks/MockQuery.h"

class FakeSqlExecutor : public ISqlExecutor {
 public:
  QString         lastSql = "";
  QList<QVariant> lastValues;
  bool            shouldFail    = false;
  int             execute_calls = 0;

  std::vector<std::string> last_sqls;
  MockQuery                mock_query;

  std::unique_ptr<IQuery> execute(const QString& sql, const QList<QVariant>& values) override {
    ++execute_calls;
    lastSql    = sql;
    lastValues = values;
    last_sqls.push_back(lastSql.toStdString());
    if (shouldFail) return nullptr;
    return std::make_unique<MockQuery>(mock_query);
  }

  // virtual std::optional<long long> executeReturningId(const QString&         sql,
  //                                                     QSqlQuery&             outQuery,
  //                                                     const QList<QVariant>& values) override {
  //   ++execute_returning_id_calls;
  //   if (!execute(sql, outQuery, values)) return std::nullopt;
  //   return mocked_id;
  // }
};

#endif  // FAKESQLEXECUTOR_H
