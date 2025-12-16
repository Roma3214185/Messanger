#ifndef ISQLEXECUTOR_H
#define ISQLEXECUTOR_H

#include <QString>
#include "interfaces/IQuery.h"

class ISqlExecutor {
 public:
  virtual ~ISqlExecutor() = default;
  virtual std::unique_ptr<IQuery> execute(const QString& sql, const QList<QVariant>& values = {}) = 0;
  //virtual std::optional<long long> executeReturningId(const QString&         sql,
  //                                                    QSqlQuery&             outQuery,
  //                                                     const QList<QVariant>& values = {}) = 0;
};

#endif  // ISQLEXECUTOR_H
