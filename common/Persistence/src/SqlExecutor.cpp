#include "SqlExecutor.h"

SqlExecutor::SqlExecutor(IDataBase &database) : database_(database) {}

SqlExecutorResult SqlExecutor::execute(const QString &sql,
                                             const QList<QVariant> &values) {
  PROFILE_SCOPE("[SqlExecutor] Execute");

  auto outQuery = database_.prepare(sql);
  if (!outQuery) {
    LOG_ERROR("Error prepare outQuery");
    return SqlExecutorResult{.query = nullptr, .error = outQuery->error().toStdString() };
  }

  for (int i = 0; i < values.size(); ++i)
    outQuery->bind(values[i]);

  LOG_INFO("[SqlExecutor] Executing SQL: {}", sql.toStdString());

  if (!outQuery->exec()) {
    LOG_ERROR("[SqlExecutor] Exec failed: '{}'", sql.toStdString());
    return SqlExecutorResult{.query = nullptr, .error = outQuery->error().toStdString() };
  }

  LOG_INFO("[SqlExecutor] Exec succeed: '{}'", sql.toStdString());
  return SqlExecutorResult{.query = std::move(outQuery), .error = outQuery->error().toStdString() };
}
