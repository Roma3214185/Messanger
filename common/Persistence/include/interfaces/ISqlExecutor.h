#ifndef ISQLEXECUTOR_H
#define ISQLEXECUTOR_H

#include <QString>

#include "interfaces/IQuery.h"

struct SqlExecutorResult {
  std::unique_ptr<IQuery> query;
  std::string error;

  SqlExecutorResult() = default;
  SqlExecutorResult(std::unique_ptr<IQuery> p_query, std::string p_error) : query(std::move(p_query)), error(std::move(p_error)) {}
  SqlExecutorResult(std::unique_ptr<IQuery> p_query) : query(std::move(p_query)) {}
  SqlExecutorResult( std::string p_error) : query(nullptr), error(std::move(p_error)) {}
  SqlExecutorResult (const SqlExecutorResult&) = delete;
  SqlExecutorResult& operator=(const SqlExecutorResult&) = delete;

  SqlExecutorResult (SqlExecutorResult&& other) noexcept
      : query(std::move(other.query)), error(std::move(other.error)) {}

  SqlExecutorResult& operator=(SqlExecutorResult&& other) noexcept {
     if (this != &other) {
      query = std::move(other.query);
      error = std::move(other.error);
     }
     return *this;
  }
};

class ISqlExecutor {
 public:
  virtual ~ISqlExecutor() = default;
  [[nodiscard]] virtual SqlExecutorResult execute(const QString &sql, const QList<QVariant> &values = {}) = 0;
};

#endif  // ISQLEXECUTOR_H
