#ifndef ISQLEXECUTOR_H
#define ISQLEXECUTOR_H

#include <QString>
#include <QtSql/QSqlQuery>

class ISqlExecutor {
 public:
  virtual ~ISqlExecutor() = default;
  virtual bool execute(const QString& sql, const QList<QVariant>& values, QSqlQuery& outQuery) = 0;
  virtual std::optional<long long> executeReturningId(const QString&         sql,
                                                      const QList<QVariant>& values,
                                                      QSqlQuery&             outQuery)                     = 0;
};

#endif  // ISQLEXECUTOR_H
