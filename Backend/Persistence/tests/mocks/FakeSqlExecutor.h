#ifndef FAKESQLEXECUTOR_H
#define FAKESQLEXECUTOR_H

#include <QString>
#include "interfaces/ISqlExecutor.h"

class FakeSqlExecutor : public ISqlExecutor {
  public:
    QString lastSql;
    QList<QVariant> lastValues;
    bool shouldFail = false;
    bool execute(const QString& sql, const QList<QVariant>& values, QSqlQuery& outQuery) override {
      lastSql = sql;
      lastValues = values;
      return !shouldFail;
    }
};

#endif // FAKESQLEXECUTOR_H
