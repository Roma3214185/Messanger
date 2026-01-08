#ifndef ISQLEXECUTOR_H
#define ISQLEXECUTOR_H

#include <QString>

#include "interfaces/IQuery.h"

struct SqlExecutorResult {
  std::unique_ptr<IQuery> query;
  std::string error;
};

class ISqlExecutor {
 public:
  virtual ~ISqlExecutor() = default;

  [[nodiscard]] virtual SqlExecutorResult execute(const QString &sql, const QList<QVariant> &values = {}) = 0;
};

#endif  // ISQLEXECUTOR_H
