#ifndef ISQLEXECUTOR_H
#define ISQLEXECUTOR_H

#include <QString>
#include <QtSql/QSqlQuery>

class ISqlExecutor {
 public:
  virtual ~ISqlExecutor() = default;
  virtual bool execute(const QString& sql, QSqlQuery& outQuery, const QList<QVariant>& values = {}) = 0;
  virtual std::optional<long long> executeReturningId(const QString&         sql,
                                                      QSqlQuery&             outQuery,
                                                       const QList<QVariant>& values = {})                     = 0;
};

#endif  // ISQLEXECUTOR_H
