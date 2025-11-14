#ifndef FAKESQLEXECUTOR_H
#define FAKESQLEXECUTOR_H

#include <QSqlField>
#include <QSqlRecord>
#include <QString>

#include "interfaces/ISqlExecutor.h"

class FakeSqlExecutor : public ISqlExecutor {
 public:
  QString         lastSql;
  QList<QVariant> lastValues;
  bool            shouldFail                 = false;
  int             execute_calls              = 0;
  int             execute_returning_id_calls = 0;
  int             mocked_id                  = 5;

  bool execute(const QString& sql, QSqlQuery& outQuery, const QList<QVariant>& values) override {
    ++execute_calls;
    lastSql    = sql;
    lastValues = values;
    return !shouldFail;
  }

  virtual std::optional<long long> executeReturningId(const QString&         sql,
                                                      QSqlQuery&             outQuery,
                                                      const QList<QVariant>& values) {
    ++execute_returning_id_calls;
    if (!execute(sql, outQuery, values)) return std::nullopt;
    return mocked_id;
  }
};

#endif  // FAKESQLEXECUTOR_H
